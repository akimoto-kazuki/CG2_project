#pragma once
#include "DirectXCommon.h"
#include "Logger.h"
#include "StringUtility.h"
class SpriteCommon;
class Sprite
{
public:
	struct Vector4 {
		float x;
		float y;
		float z;
		float w;
	};

	struct Vector3
	{
		float x;
		float y;
		float z;
	};

	struct Vector2
	{
		float x;
		float y;
	};

	struct Matrix4x4
	{
		float m[4][4];
	};

	struct VertexData
	{
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};

	struct Material
	{
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};

	struct Transform
	{
		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};


	struct TransformationMatrix
	{
		Matrix4x4 WVP;
		Matrix4x4 World;
	};

	// 初期化
	void Initialize(SpriteCommon* spriteCommon);

	void Update();

	void Draw();

	Matrix4x4 MakeIdentity4x4();

	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

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

