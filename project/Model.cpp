#include "Model.h"
#include <cassert>
//ファイルやディレクトリに関する操作を行うライブラリ
#include <filesystem>
//ファイルに書いたり読んだりするライブラリ
#include<fstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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
	std::string fullPath = directoryPath + "/" + filename;

	// 1. Assimpのインポーターを作成
	Assimp::Importer importer;

	// 2. ファイルを読み込む
	// コントロールフラグでDX12（左手系）に合わせた変換を自動で行わせるのが便利です
	const aiScene* scene = importer.ReadFile(fullPath,
		aiProcess_Triangulate |           // 四角形ポリゴンなどをすべて三角形に変換
		aiProcess_FlipUVs |               // UV座標のY軸を反転 (今の実装の texcoord.y = 1.0f - texcoord.y に相当)
		aiProcess_ConvertToLeftHanded |   // 左手系 (DirectX標準) に変換 (X軸反転などに相当)
		aiProcess_CalcTangentSpace        // 法線や接ベクトルを計算
	);

	// 読み込み失敗時のチェック
	assert(scene && scene->mRootNode && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE));

	// 3. メッシュデータの解析 (今回はシンプルに最初のメッシュ [0] を読み込む例)
	// ※実際のゲームエンジンでは、scene->mNumMeshes をループして複数メッシュに対応させます
	if (scene->mNumMeshes > 0)
	{
		aiMesh* mesh = scene->mMeshes[0];

		// 頂点データの取得
		for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
		{
			VertexData vertex{};

			// 位置 (Position)
			vertex.position.x = mesh->mVertices[i].x;
			vertex.position.y = mesh->mVertices[i].y;
			vertex.position.z = mesh->mVertices[i].z;
			vertex.position.w = 1.0f;

			// 法線 (Normal)
			if (mesh->HasNormals())
			{
				vertex.normal.x = mesh->mNormals[i].x;
				vertex.normal.y = mesh->mNormals[i].y;
				vertex.normal.z = mesh->mNormals[i].z;
			}

			// テクスチャ座標 (Texcoord) - Assimpは最大8つのUVチャンネルを持てるが、通常は [0] を使う
			if (mesh->HasTextureCoords(0))
			{
				vertex.texcoord.x = mesh->mTextureCoords[0][i].x;
				vertex.texcoord.y = mesh->mTextureCoords[0][i].y;
			}
			else
			{
				vertex.texcoord = { 0.0f, 0.0f };
			}

			modelData.vertices.push_back(vertex);
		}

		// インデックスデータの取得 (今回は drawInstanced で直列に描画しているため、頂点配列に展開する例)
		// ※今後インデックスバッファ(IBV)を本格的に使う場合は、ここを index配列に push_back します
		std::vector<VertexData> triangulatedVertices;
		for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
		{
			aiFace face = mesh->mFaces[i];
			// aiProcess_Triangulate を指定しているので、必ず3頂点(三角形)になります
			for (uint32_t j = 0; j < face.mNumIndices; ++j)
			{
				uint32_t index = face.mIndices[j];
				triangulatedVertices.push_back(modelData.vertices[index]);
			}
		}
		modelData.vertices = std::move(triangulatedVertices);

		// 4. マテリアル（テクスチャパス）の取得
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			aiString texturePath;

			// ディフューズ（ベースカラー）テクスチャのパスを取得
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
			{
				// ファイル名だけを取り出してパスを結合するなどの処理
				std::filesystem::path path(texturePath.C_Str());
				modelData.material.textureFilePath = directoryPath + "/" + path.filename().string();
			}
		}
	}

	return modelData;
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