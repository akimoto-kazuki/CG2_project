#pragma once
#include "ModelCommon.h"
#include "Logger.h"
#include "StringUtility.h"
#include "MyMath.h"
#include "TextureManager.h"

#include <stdint.h>

class Model
{
	struct VertexData
	{
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
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

public:

	void Initialize(ModelCommon* modelCommon);

	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);


private:

	// モデルポインタ
	ModelCommon* modelCommon_;
	// Objファイルのデータ
	ModelData modelData;
	// 
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	//
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	//
	VertexData* vertexData = nullptr;
	//
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr;
	//
	Material* materialData = nullptr;
};

