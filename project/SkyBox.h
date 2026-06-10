#pragma once
#include "DirectXCommon.h"
#include "MyMath.h"
#include "Camera.h"

// 前方宣言（循環参照を防ぐため）
class SkyBoxCommon;
class Camera;

class SkyBox
{
public:

	// --- 既存の構造体に加えて、マテリアル構造体を追加 ---
	struct Material
	{
		MyMath::Vector4 color;
		int32_t enableLighting;
		float padding[3]; // 16バイトアライメント用パディング
		MyMath::Matrix4x4 uvTransform;
		float shininess;
		float padding2[3]; // 16バイトアライメント用パディング
	};

	// Skybox用のシンプルな頂点構造体（座標だけ）
	struct VertexData
	{
		MyMath::Vector4 position;
	};

	// 行列用の構造体（Object3dと同じ）
	struct TransformationMatrix
	{
		MyMath::Matrix4x4 WVP;
		MyMath::Matrix4x4 World;
	};

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(SkyBoxCommon* skyboxCommon);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	// テクスチャ（キューブマップ）のSRVインデックスを設定する関数
	void SetTextureIndex(uint32_t textureIndex) { textureIndex_ = textureIndex; }

private:
	SkyBoxCommon* skyboxCommon_ = nullptr;
	Camera* camera = nullptr;

	// 行列用のリソース（Object3dと同じ）
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = nullptr;
	TransformationMatrix* transformationMatrixData = nullptr;

	// --- 【Modelから移植】Skybox自身が持つ頂点バッファ一式 ---
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	// 描画に使用するキューブマップのSRVインデックス
	uint32_t textureIndex_ = 0;

	// --- 既存のメンバ変数の後ろに追加 ---
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr;
	Material* materialData = nullptr;
};