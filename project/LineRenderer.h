#pragma once
#include "DirectXCommon.h"
#include "Camera.h"
#include "MyMath.h"
#include <vector>
#include <wrl.h>

using namespace MyMath;

class LineRenderer
{
public:
    // 頂点の構造体（位置と色だけ）
    struct LineVertex 
    {
        Vector3 position;
        Vector4 color;
    };

    // GPUに送る用の定数バッファ（ビュープロジェクション行列）
    struct ConstBufferData 
    {
        Matrix4x4 viewProjection;
    };

    static const uint32_t kMaxLineCount = 65536;
    static const uint32_t kMaxVertexCount = kMaxLineCount * 2;

public:
    // 初期化（PSOやバッファの作成）
    void Initialize(DirectXCommon* dxCommon);

    // 毎フレーム、線を登録する関数
    void AddLine(const Vector3& start, const Vector3& end, const Vector4& color);

    void AddSphere(const Vector3& center, float radius, const Vector4& color, uint32_t division = 12);

    // 描画（溜まった線を一気に描画して、リストをクリアする）
    void Draw(DirectXCommon* dxCommon, Camera* camera);

private:
    // 描画待ちの頂点データ
    std::vector<LineVertex> vertices_;

    // DirectX12 リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    LineVertex* mappedVertexData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> constBuffer_;
    ConstBufferData* mappedConstData_ = nullptr;

    // PSOとルートシグネチャ（必要に応じて実装）
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

    DirectXCommon* dxCommon_;
};
