#pragma once
#include "DirectXCommon.h"
#include "Logger.h"
#include "StringUtility.h"
#include "MyMath.h"
#include "TextureManager.h"

#include <stdint.h>

class Object3dCommon;

class Object3d
{

	struct VertexData
	{
		MyMath::Vector4 position;
		MyMath::Vector2 texcoord;
		MyMath::Vector3 normal;
	};

	struct MaterialData
	{
		std::string textureFilePath;
		uint32_t textureIndex = 0;
	};

	struct ModelData
	{
		std::vector<VertexData> vertices;
		MaterialData material;
	};

	struct Material
	{
		MyMath::Vector4 color;
		int32_t enableLighting;
		float padding[3];
		MyMath::Matrix4x4 uvTransform;
	};

	struct TransformationMatrix
	{
		MyMath::Matrix4x4 WVP;
		MyMath::Matrix4x4 World;
	};

	struct DirectionalLight
	{
		MyMath::Vector4 color;
		MyMath::Vector3 direction;
		float intensity;
	};

public:
	
	// 初期化
	void Initialize(Object3dCommon* object3dCommon);

	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

	void Update();

	void Draw();

	float GetRotation()const { return rotation; }
	void SetRotation(float rotation) { this->rotation = rotation; }

private:

	Object3dCommon* object3dCommon_ = nullptr;

	ModelData modelData;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	VertexData* vertexData = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr;
	Material* materialData = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = nullptr;
	TransformationMatrix* transformationMatrixData = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = nullptr;
	DirectionalLight* directionalLightData = nullptr;

	MyMath::Transform transform;

	MyMath::Transform cameraTransform;

	uint32_t textureIndex = 0;

	float rotation = 0.0f;

};

