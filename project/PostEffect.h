#pragma once
#include "DirectXCommon.h"
#include <d3d12.h>
#include <wrl.h>
#include <DirectXMath.h>

class PostEffect {
public:
    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize(DirectXCommon* dxCommon);

    /// <summary>
    /// 描画前処理（レンダーテクスチャへの描き込み開始）
    /// </summary>
    void PreDraw();

    /// <summary>
    /// 描画後処理（レンダーテクスチャへの描き込み終了）
    /// </summary>
    void PostDraw();

    /// <summary>
    /// 本番描画（レンダーテクスチャを全画面ポリゴンとしてバックバッファに描画）
    /// </summary>
    void Draw();

    /// <summary>
    /// 解放処理
    /// </summary>
    void Finalize();

private:
    // ポストエフェクト専用の頂点構造体
    struct Vertex {
        DirectX::XMFLOAT4 pos; // 画面座標 (x, y, z, w)
        DirectX::XMFLOAT2 uv;  // テクスチャ座標 (u, v)
    };

    DirectXCommon* dxCommon_ = nullptr;

    // レンダーテクスチャ（描画先）リソースとビュー関連
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_ = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};
    uint32_t srvIndex_ = 0; // SrvManager管理用インデックス

    // パイプライン・ルートシグネチャ関連
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_ = nullptr;

    // 全画面表示用の頂点バッファ関連
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
};
