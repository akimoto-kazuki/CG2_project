#include "Model.h"
#include <cassert>
//ファイルやディレクトリに関する操作を行うライブラリ
#include <filesystem>
//ファイルに書いたり読んだりするライブラリ
#include<fstream>

using namespace MyMath;

void Model::Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename)
{
	modelCommon_ = modelCommon;
	modelData = LoadModelFile(directorypath, filename);
	// リソース制作
	vertexResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());

	indexResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * modelData.vertices.size());
	
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * UINT(modelData.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	indexBufferView.BufferLocation = indexResource.Get()->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * UINT(modelData.vertices.size());
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// 書き込むためのアドレスを取得
	vertexResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	indexResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	//マテリアル
	materialResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(Material));
	// 書き込むためのアドレスを取得
	materialResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 今回は赤を書き込んでみる
	materialData->color = { 1.0f,1.0f,1.0f,1.0f };
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();
	// テクスチャ読み込み
	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
	// 番号を取得
	modelData.material.textureIndex =
		TextureManager::GetInstance()->GetTextureIndexByFilepath(modelData.material.textureFilePath);

	// ======= ★ここを追加！ =======
	materialData->shininess = 70.0f;               // 光沢度（基本は 10.0f 〜 100.0f あたり）
	materialData->environmentCoefficient = 0.3f;   // 環境マップの映り込みの強さ（0.0f 〜 1.0f）
	// =============================
}

Model::MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	MaterialData materialData;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "map_Kd")
		{
			std::string textureFilename;
			s >> textureFilename;

			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	return materialData;
}

Model::ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	ModelData modelData;
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v")
		{
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt")
		{
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn")
		{
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (identifier == "f")
		{
			VertexData triangle[3];
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex)
			{
				std::string vertexDefinition;
				s >> vertexDefinition;

				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element)
				{
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}
				Vector4 position = positions[elementIndices[0] - 1];
				position.x *= -1.0f;
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				texcoord.y = 1.0f - texcoord.y;
				Vector3 normal = normals[elementIndices[2] - 1];
				triangle[faceVertex] = { position,texcoord,normal };
			}
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib")
		{
			std::string materialFilename;
			s >> materialFilename;

			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	return modelData;
}

Model::ModelData Model::LoadModelFile(const std::string& directoryPath, const std::string& filename)
{
	ModelData modelData;

	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMaterials());

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
	{
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
		{
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);
			for (uint32_t element = 0; element < face.mNumIndices; ++element)
			{
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				VertexData vertex;
				vertex.position = { position.x,position.y,position.z,1.0f };
				vertex.normal = { normal.x,normal.y,normal.z };
				vertex.texcoord = { texcoord.x,texcoord.y };

				vertex.position.x *= -1.0f;
				vertex.normal.x *= -1.0f;
				modelData.vertices.push_back(vertex);
			}
		}
	}

	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex)
	{
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
		{
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			modelData.material.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();
		}
	}

	modelData.rootNode = ReadNode(scene->mRootNode);

	return modelData;
}

Model::Animation Model::LoadAnimationFile(const std::string& directoryPath, const std::string& filename)
{
	Animation animation;
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(),0);
	assert(scene->mNumAnimations != 0);
	aiAnimation* animationAssimp = scene->mAnimations[0];
	animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);
	for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
		aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];

		// 位置（Translate）のキーフレーム読み込み
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
			nodeAnimation.translate.push_back(keyframe);
		}

		// ※同様に mRotationKeys (回転), mScalingKeys (スケール) も読み込む
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
			// ★ポイント1：型が aiVectorKey ではなく aiQuatKey になる！
			aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			KeyframeQuaternion keyframe;
			// 時間の計算（Translateと同様に秒単位へ変換）
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			// ★ポイント2：値が3つ(x,y,z)ではなく、wを加えた4つになる！
			// ※ご自身の MyMath::Quaternion の構造体定義（x,y,z,w の順番）に合わせて代入してください
			keyframe.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w };
			nodeAnimation.rotate.push_back(keyframe);
		}

		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
			nodeAnimation.scale.push_back(keyframe);
		}
	}
	return animation;
}

Model::Node Model::ReadNode(aiNode* node)
{
	Node result;
	aiVector3D scale, translate;
	aiQuaternion rotate;
	node->mTransformation.Decompose(scale, rotate, translate);
	result.transform.scale = { scale.x,scale.y,scale.z };
	result.transform.rotate = { rotate.x,-rotate.y,-rotate.z,rotate.w };
	result.transform.translate = { -translate.x,translate.y,translate.z };
	result.localMatrix = MakeAffineMatrixQuaternion(result.transform.scale, result.transform.rotate, result.transform.translate);

	result.name = node->mName.C_Str();
	result.children.resize(node->mNumChildren);
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; childIndex++)
	{
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}
	return result;
}

Vector3 Model::CalculateValueVector3(const std::vector<Model::KeyframeVector3>& keyframes, float time)
{
	assert(!keyframes.empty()); // キーフレームが空ならエラー

	// キーフレームが1つだけ、または再生時間が最初のキーより前なら最初の値を返す
	if (keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}
	// 再生時間が最後のキーより後ろなら最後の値を返す
	if (time >= keyframes.back().time) {
		return keyframes.back().value;
	}

	// 現在の時間に対応するキーフレームの区間を探す
	for (size_t i = 0; i < keyframes.size() - 1; ++i) {
		const auto& nextKey = keyframes[i + 1];
		if (time < nextKey.time) {
			const auto& prevKey = keyframes[i];
			// 2つのキーフレーム間の割合 t (0.0 ~ 1.0) を計算
			float t = (time - prevKey.time) / (nextKey.time - prevKey.time);
			// 線形補間（Lerp）
			return Lerp(prevKey.value, nextKey.value, t);
		}
	}
	return keyframes.back().value;
}

Quaternion Model::CalculateValueQuaternion(const std::vector<Model::KeyframeQuaternion>& keyframes, float time)
{
	assert(!keyframes.empty());

	if (keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}
	if (time >= keyframes.back().time) {
		return keyframes.back().value;
	}

	for (size_t i = 0; i < keyframes.size() - 1; ++i) {
		const auto& nextKey = keyframes[i + 1];
		if (time < nextKey.time) {
			const auto& prevKey = keyframes[i];
			float t = (time - prevKey.time) / (nextKey.time - prevKey.time);
			// 球面線形補間（Slerp）を使って回転を滑らかに補間する
			return Slerp(prevKey.value, nextKey.value, t);
		}
	}
	return keyframes.back().value;
}

void Model ::Draw()
{

	auto commandList = modelCommon_->GetDxCommon()->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);  // VBVを設定
	// インデックス
	commandList->IASetIndexBuffer(&indexBufferView);
	// マテリアルCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	
	assert(!modelData.material.textureFilePath.empty()); // パスが空じゃないか？
	assert(TextureManager::GetInstance()->textureDatas.contains(modelData.material.textureFilePath)); // 登録されているか？

	commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelData.material.textureFilePath));
	// 描画！（DrawCall／ドローコール）。3頂点で1つのインスタンス。インスタンスについては今後
	commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

}