#include "Object3d.h"
#include "Object3dCommon.h"

#include <cassert>
//ファイルやディレクトリに関する操作を行うライブラリ
#include <filesystem>
//ファイルに書いたり読んだりするライブラリ
#include<fstream>

using namespace MyMath;

void Object3d::Initialize(Object3dCommon* object3dCommon)
{
	this->object3dCommon_ = object3dCommon;
	modelData = LoadObjFile("resources", "plane.obj");
	// リソース制作
	vertexResource = object3dCommon_->GetDxCommon()->CreatBufferResource(sizeof(VertexData) * modelData.vertices.size());

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData)* modelData.vertices.size();
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 書き込むためのアドレスを取得
	vertexResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	// 左下
	vertexData[0].position = { -0.5f, -0.5f, 0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	// 上
	vertexData[1].position = { 0.0f,  0.5f, 0.0f, 1.0f };
	vertexData[1].texcoord = { 0.5f,0.0f };
	// 右下
	vertexData[2].position = { 0.5f, -0.5f, 0.0f, 1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };

	// 左下
	vertexData[3].position = { -0.5f, -0.5f,0.5f, 1.0f };
	vertexData[3].texcoord = { 0.0f,1.0f };
	// 上
	vertexData[4].position = { 0.0f,  0.0f, 0.0f, 1.0f };
	vertexData[4].texcoord = { 0.5f,0.0f };
	// 右下
	vertexData[5].position = { 0.5f, -0.5f,-0.5f, 1.0f };
	vertexData[5].texcoord = { 1.0f,1.0f };

	//マテリアル
	materialResource = object3dCommon_->GetDxCommon()->CreatBufferResource(sizeof(Vector4));
	// 書き込むためのアドレスを取得
	materialResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 今回は赤を書き込んでみる
	materialData->color = Vector4{ 1.0f,1.0f,1.0f,0.0f };
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();

	//トランスフォーム
	transformationMatrixResource = object3dCommon_->GetDxCommon()->CreatBufferResource(sizeof(TransformationMatrix));
	transformationMatrixResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();

	//ライティング
	directionalLightResource = object3dCommon_->GetDxCommon()->CreatBufferResource(sizeof(DirectionalLight));
	directionalLightResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,0.0f,1.0f };
	directionalLightData->intensity = 1.0f;

	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);

	modelData.material.textureIndex = 
		TextureManager::GetInstance()->GetTextureIndexByFilepath(modelData.material.textureFilePath);

	transform = { {1.0f,1.0f,1.0f},{0.0f,rotation,0.0f},{0.0f,0.0f,0.0f} };

	cameraTransform = { {1.0f,1.0f,1.0f},{0.3f,0.0f,0.0f},{0.0f,4.0f,-10.0f} };

}

Object3d::MaterialData Object3d::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
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

Object3d::ModelData Object3d::LoadObjFile(const std::string& directoryPath, const std::string& filename)
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
				//normal.x *= -1.0f;
				//VertexData vertex = { position,texcoord,normal };
				//VertexData vertex = { position,texcoord };
				//modelData.vertices.push_back(vertex);
				triangle[faceVertex] = { position,texcoord,normal };
				//triangle[faceVertex] = { position,texcoord };
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

void Object3d::Update()
{
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);

	transformationMatrixData->WVP = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->World = worldMatrix;
}

void Object3d::Draw()
{
	object3dCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);  // VBVを設定

	// マテリアルCBufferの場所を設定
	object3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// wvp用のCBufferの場所を設定]
	object3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = object3dCommon_->GetDxCommon()->GetSRVGPUDescriptorHandle(1);
	object3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex));
	object3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	// 描画！（DrawCall／ドローコール）。3頂点で1つのインスタンス。インスタンスについては今後
	object3dCommon_->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(UINT(modelData.vertices.size()), 1, 0, 0,0);
}