#pragma once
#include "DirectXCommon.h"
#include "Logger.h"
#include "StringUtility.h"

class Camera;

class Object3dCommon
{
public:

    // インスタンス取得
    static Object3dCommon* GetInstance();

    // ★ コピー・代入の禁止
    Object3dCommon(const Object3dCommon&) = delete;
    Object3dCommon& operator=(const Object3dCommon&) = delete;
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
    Object3dCommon() = default;
    ~Object3dCommon() = default;
    // ルートシグネチャの作成
    void RootSignature();
    // グラフィックスパイプラインの生成
    void GraphicsPipelineState();
    // ルートシグネチャ
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

    Camera* defaultCamera = nullptr;

};

