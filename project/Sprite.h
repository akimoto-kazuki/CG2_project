#pragma once
#include "DirectXCommon.h"
#include "Logger.h"
#include "StringUtility.h"
#include "MyMath.h"

#include <stdint.h>

class SpriteCommon;
class Sprite
{

	struct VertexData
	{
		MyMath::Vector4 position;
		MyMath::Vector2 texcoord;
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

public:

	// 初期化
	void Initialize(SpriteCommon* spriteCommon);

	void Update();

	void Draw();

private:

	SpriteCommon* spriteCommon = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	// 頂点インデックス
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr;

	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr;

	Material* materialData = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = nullptr;

	TransformationMatrix* transformationMatrixData = nullptr;

};

