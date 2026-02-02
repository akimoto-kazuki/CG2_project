#pragma once
#include "DirectXCommon.h"
#include "Logger.h"
#include "StringUtility.h"

class SpriteCommon
{
public:
	void Initialize(DirectXCommon* dxCommon);
	// 共通描画設定
	void DrawCommon();
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
private:
	// ルートシグネチャの作成
	void RootSignature();
	// グラフィックスパイプラインの生成
	void GraphicsPipelineState();
	DirectXCommon* dxCommon_;
	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
};

