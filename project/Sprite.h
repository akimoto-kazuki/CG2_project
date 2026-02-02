#pragma once
#include "DirectXCommon.h"
#include "Logger.h"
#include "StringUtility.h"
#include "MyMath.h"
#include "TextureManager.h"

#include <stdint.h>

class SpriteCommon;
class Sprite
{

	MyMath::Vector2 anchorPoint = { 0.0f,0.0f };

	MyMath::Vector2 textureLeftTop = { 0.0f,0.0f };

	MyMath::Vector2 textureSize = { 100.0f,100.0f };

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
	void Initialize(SpriteCommon* spriteCommon, std::string textureFilePath);

	void Update();

	void Draw();

	const MyMath::Vector3& GetPosition()const { return position_; }
	void SetPosition(const MyMath::Vector3& position) { this->position_ = position; }

	float GetRotation()const { return rotation; }
	void SetRotation(float rotation) { this->rotation = rotation; }

	const MyMath::Vector4& GetColor()const { return materialData->color; }
	void SetColor(const MyMath::Vector4& color) { materialData->color = color; }

	const MyMath::Vector2& GetSize()const { return size; }
	void SetSize(const MyMath::Vector2& size) { this->size = size; }

	const MyMath::Vector2& GetAnchorPoint()const { return anchorPoint; }
	void SetAnchorPoint(const MyMath::Vector2& anchorPoint) { this->anchorPoint = anchorPoint; }

	bool GetIsFlipX()const { return isFlipX_; }
	void SetIsFlipX(const bool isFlipX) { this->isFlipX_ = isFlipX; }

	bool GetIsFlipY()const { return isFlipY_; }
	void SetIsFlipY(const bool isFlipY) { this->isFlipY_ = isFlipY; }

	const MyMath::Vector2& GetTextureLeftTop()const { return textureLeftTop; }
	void SetTextureLeftTop(const MyMath::Vector2& textureLeftTop) { this->textureLeftTop = textureLeftTop; }
	const MyMath::Vector2& GetTextureSize()const { return textureSize; }
	void SetTextureSize(const MyMath::Vector2& textureSize) { this->textureSize = textureSize; }

private:

	/// <summary>
	/// ポインタ
	/// </summary>
	SpriteCommon* spriteCommon = nullptr;
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	TransformationMatrix* transformationMatrixData = nullptr;
	Material* materialData = nullptr;

	/// <summary>
	/// リソース
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	// 頂点インデックス
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = nullptr;

	// 頂点リソースにデータを書き込む
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	/// <summary>
	/// スプライトの位置
	/// </summary>
	MyMath::Vector3 position_;
	/// <summary>
	/// スプライトの回転
	/// </summary>
	float rotation = 0.0f;
	/// <summary>
	/// スプライトのサイズ
	/// </summary>
	MyMath::Vector2 size;

	uint32_t textureIndex = 0;

	bool isFlipX_ = false;

	bool isFlipY_ = false;

	// テクスチャサイズをイメージに合わせる
	void AdjustTextureSize();

};

