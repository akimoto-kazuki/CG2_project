#pragma once
#include "DirectXCommon.h"
#include "Logger.h"
#include "StringUtility.h"

class SpriteCommon
{
public:

	// ★ Singleton インスタンス取得
	static SpriteCommon* GetInstance();

	// ★ コピー・代入の禁止
	SpriteCommon(const SpriteCommon&) = delete;
	SpriteCommon& operator=(const SpriteCommon&) = delete;

	void Initialize();
	// 共通描画設定
	void DrawCommon();
private:

	// ★ コンストラクタを private へ移動
	SpriteCommon() = default;
	~SpriteCommon() = default;
	// ルートシグネチャの作成
	void RootSignature();
	// グラフィックスパイプラインの生成
	void GraphicsPipelineState();
	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
};

