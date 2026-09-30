#pragma once
#include "DirectXCommon.h"
#include "Logger.h"
#include "StringUtility.h"

class Camera;

class SkyBoxCommon 
{
public:

    // ★ Singleton インスタンス取得
    static SkyBoxCommon* GetInstance();

    // ★ コピー・代入の禁止
    SkyBoxCommon(const SkyBoxCommon&) = delete;
    SkyBoxCommon& operator=(const SkyBoxCommon&) = delete;

    // 初期化
    void Initialize();
    // 共通描画設定
    void DrawCommon();

    // set
    void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
    // get
    Camera* GetDefaultCamera()const { return defaultCamera; }

private:

    // ★ コンストラクタを private へ移動
    SkyBoxCommon() = default;
    ~SkyBoxCommon() = default;

    // ルートシグネチャの作成
    void RootSignature();
    // グラフィックスパイプラインの生成
    void GraphicsPipelineState();
    // ルートシグネチャ
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

    Camera* defaultCamera = nullptr;
};