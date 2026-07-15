#pragma once
#include "ModelCommon.h"
#include "Logger.h"
#include "StringUtility.h"
#include "MyMath.h"
#include "TextureManager.h"

#include <stdint.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace MyMath;

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

	struct Node
	{
		Matrix4x4 localMatrix;
		std::string name;
		std::vector<Node> children;
	};

	struct ModelData
	{
		std::vector<VertexData> vertices;
		MaterialData material;
		Node rootNode;
	};

	struct Material
	{
		MyMath::Vector4 color;
		int32_t enableLighting;
		float padding[3];
		MyMath::Matrix4x4 uvTransform;
		float shininess;                  // 4バイト
		float environmentCoefficient;
	};

public:

	void Initialize(ModelCommon* modelCommon,const std::string& directorypath,const std::string& filename);

	void Draw();

	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

	static ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename);

	Material* GetMaterialData() { return materialData; }

	ModelData GetModelData() { return modelData; }

	static Node ReadNode(aiNode* node);

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

	uint32_t* indexData = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr;
	//
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr;
	//
	Material* materialData = nullptr;
};

