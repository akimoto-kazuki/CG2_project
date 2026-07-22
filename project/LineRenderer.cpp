#include "LineRenderer.h"
#include <cmath>
#include <numbers>

void LineRenderer::Initialize(DirectXCommon* dxCommon)
{
    dxCommon_ = dxCommon;

    vertexBuffer_ = dxCommon->CreateBufferResource(sizeof(LineVertex) * kMaxVertexCount);

    vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(LineVertex) * kMaxVertexCount;
    vertexBufferView_.StrideInBytes = sizeof(LineVertex);

    // マッピング（書き込み用にアドレスを取得）
    vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertexData_));

    // 2. 定数バッファ（カメラ用）の作成
    constBuffer_ = dxCommon->CreateBufferResource(sizeof(ConstBufferData));
    constBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedConstData_));

    // ※ここにルートシグネチャとPSOの作成処理が入ります（後述）
    // ==========================================
    // 1. ルートシグネチャの作成
    // ==========================================
    D3D12_ROOT_PARAMETER rootParameters[1] = {};
    // 定数バッファ(b0)としてカメラのViewProjection行列を渡す設定
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].Descriptor.ShaderRegister = 0; // b0に紐づけ
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // 頂点シェーダーで使う

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureDesc.NumParameters = _countof(rootParameters);
    rootSignatureDesc.pParameters = rootParameters;

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));


    // ==========================================
    // 2. インプットレイアウトの設定 (頂点データの構造をGPUに教える)
    // ==========================================
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        // float3 position : POSITION
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        // float4 color : COLOR
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };


    // ==========================================
    // 3. PSO（グラフィックスパイプラインステート）の作成
    // ==========================================
    // ※エンジンの関数を使ってVSとPSをコンパイルしてください
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(L"Line.VS.hlsl", L"vs_6_0");
    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(L"Line.PS.hlsl", L"ps_6_0");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSignature_.Get();
    psoDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
    psoDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };

    // ★ここが一番重要！ トポロジータイプを「LINE（線）」にする
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };

    // ブレンドステート（通常通り）
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // ラスタライザステート（カリングなし。線なのでカリングは関係ありませんが基本設定として）
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

    // 深度テストを【無効】にすると、モデルの中に骨が埋もれず「X線」のように透けて見えます
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // メインのレンダーターゲットのフォーマットに合わせる
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    psoDesc.SampleDesc.Count = 1;

    dxCommon->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));
}

void LineRenderer::AddLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
    if (vertices_.size() + 2 <= kMaxVertexCount) {
        vertices_.push_back({ start, color });
        vertices_.push_back({ end, color });
    }
}

void LineRenderer::AddSphere(const Vector3& center, float radius, const Vector4& color, uint32_t division)
{
    const float kPI = 3.1415926535f;

    // 緯度方向（縦の輪切り）
    for (uint32_t i = 0; i <= division; ++i)
    {
        float lat0 = kPI * (-0.5f + (float)(i - 1) / division);
        float z0 = radius * std::sin(lat0);
        float r0 = radius * std::cos(lat0);

        float lat1 = kPI * (-0.5f + (float)i / division);
        float z1 = radius * std::sin(lat1);
        float r1 = radius * std::cos(lat1);

        for (uint32_t j = 0; j < division; ++j)
        {
            float lng0 = 2.0f * kPI * (float)j / division;
            float lng1 = 2.0f * kPI * (float)(j + 1) / division;

            // 横方向の輪っか（緯度線）
            Vector3 p1 = { center.x + r1 * std::cos(lng0), center.y + z1, center.z + r1 * std::sin(lng0) };
            Vector3 p2 = { center.x + r1 * std::cos(lng1), center.y + z1, center.z + r1 * std::sin(lng1) };
            AddLine(p1, p2, color);

            // 縦方向の線（経度線）
            Vector3 p3 = { center.x + r0 * std::cos(lng0), center.y + z0, center.z + r0 * std::sin(lng0) };
            AddLine(p1, p3, color);
        }
    }
}

void LineRenderer::Draw(DirectXCommon* dxCommon, Camera* camera)
{
    if (vertices_.empty() || !camera) return;

    // 1. カメラのビュープロジェクション行列を定数バッファに書き込む
    mappedConstData_->viewProjection = camera->GetViewProjectionMatrix();

    // 2. 登録された線の頂点データをGPUにコピー
    std::memcpy(mappedVertexData_, vertices_.data(), sizeof(LineVertex) * vertices_.size());

    // 3. 描画コマンドの積む
    auto commandList = dxCommon->GetCommandList();

    commandList->SetPipelineState(pipelineState_.Get());
    commandList->SetGraphicsRootSignature(rootSignature_.Get());

    // ★重要：トポロジーを「線のリスト」に設定する！
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

    // 定数バッファをセット（ルートパラメータの番号は自身のシグネチャに合わせてください）
    commandList->SetGraphicsRootConstantBufferView(0, constBuffer_->GetGPUVirtualAddress());

    // 描画実行！（DrawInstanced）
    commandList->DrawInstanced(static_cast<UINT>(vertices_.size()), 1, 0, 0);

    // 4. 次のフレームのためにリストを空にする
    vertices_.clear();
}
