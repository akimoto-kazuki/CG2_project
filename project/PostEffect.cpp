#include "PostEffect.h"
#include "SrvManager.h"
#include "WinApp.h"
#include <cassert>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

void PostEffect::Initialize(DirectXCommon* dxCommon) {
    assert(dxCommon);
    dxCommon_ = dxCommon;
    auto device = dxCommon_->GetDevice();

    // =========================================================================
    // 1. RenderTexture（描画先テクスチャ）リソースの作成（資料3〜4枚目仕様）
    // =========================================================================
    D3D12_RESOURCE_DESC textureDesc{};
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    textureDesc.Width = WinApp::kClientWidth;   // 1280
    textureDesc.Height = WinApp::kClientHeight; // 720
    textureDesc.DepthOrArraySize = 1;
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 資料指定フォーマット
    textureDesc.SampleDesc.Count = 1;
    // レンダーターゲットとして書き込めるようにフラグを設定
    textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    // クリアカラー（背景色）の設定（資料の既定値）
    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = textureDesc.Format;
    clearValue.Color[0] = 1.0f;  // R
    clearValue.Color[1] = 0.0f;  // G
    clearValue.Color[2] = 0.0f;  // B
    clearValue.Color[3] = 1.0f;  // A

    // リソースの生成
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT); // 変数として実体を作る

    // リソースの生成
    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // 初期状態はテクスチャとして読める状態
        &clearValue,
        IID_PPV_ARGS(&textureResource_)
    );
    assert(SUCCEEDED(hr));

    // =========================================================================
    // 2. RTV（レンダーターゲットビュー）の作成（資料5枚目仕様）
    // =========================================================================
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap_));

    rtvHandle_ = rtvHeap_->GetCPUDescriptorHandleForHeapStart();

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = textureDesc.Format;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    device->CreateRenderTargetView(textureResource_.Get(), &rtvDesc, rtvHandle_);

    // =========================================================================
    // 3. SRV（シェーダーリソースビュー）の作成（資料6枚目仕様）
    // =========================================================================
    // 自作エンジンの SrvManager を使って賢くデスクリプタを自動割り当て
    srvIndex_ = SrvManager::GetInstance()->Allocate();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = textureDesc.Format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    device->CreateShaderResourceView(
        textureResource_.Get(),
        &srvDesc,
        SrvManager::GetInstance()->GetCPUDescriptorHandle(srvIndex_)
    );

    // =========================================================================
    // 4. ルートシグネチャの作成（資料11〜13枚目仕様）
    // =========================================================================
    D3D12_DESCRIPTOR_RANGE descriptorRange{};
    descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange.NumDescriptors = 1;      // レンダーテクスチャ1枚
    descriptorRange.BaseShaderRegister = 0;   // t0
    descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParameters[1]{};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // ピクセルシェーダーから見える

    // 静的サンプラーの設定（テクスチャ引き伸ばし時の補間方法）
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1]{};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    staticSamplers[0].ShaderRegister = 0; // s0
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.NumParameters = _countof(rootParameters);
    rootSigDesc.pParameters = rootParameters;
    rootSigDesc.NumStaticSamplers = _countof(staticSamplers);
    rootSigDesc.pStaticSamplers = staticSamplers;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    assert(SUCCEEDED(hr));
    hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));

    // =========================================================================
    // 5. パイプラインステート(PSO)の作成（資料14〜18枚目仕様）
    // =========================================================================
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlob = nullptr;

    // ユーザーが作成したポストエフェクト用シェーダーをコンパイルして読み込み
    hr = D3DCompileFromFile(L"GrayScale.VS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        assert(false);
    }
    hr = D3DCompileFromFile(L"GrayScale.PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        assert(false);
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = rootSignature_.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

    psoDesc.SampleMask = UINT_MAX;
    psoDesc.SampleDesc.Count = 1;

    // ★重要：画面全体を裏表関係なく覆うため、カリングは「無し」にする
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

    // ブレンド設定（レンダーテクスチャをそのまま100%上書き描画するためブレンドはOFF）
    D3D12_RENDER_TARGET_BLEND_DESC blendDesc{};
    blendDesc.BlendEnable = FALSE;
    blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.BlendState.RenderTarget[0] = blendDesc;

    // ★重要：全画面ポリゴンを最前面に描くため、深度テスト（Depth）は「無し」にする
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;

    // インプットレイアウト（POSITION と TEXCOORD）
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
    psoDesc.InputLayout.pInputElementDescs = inputLayout;
    psoDesc.InputLayout.NumElements = _countof(inputLayout);

    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // バックバッファと同じフォーマット

    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));
    assert(SUCCEEDED(hr));

    // =========================================================================
    // 6. 全画面頂点バッファの作成（資料19〜20枚目仕様）
    // =========================================================================
    // クライアント領域全体を覆うように、正規化デバイス座標系(-1.0 〜 1.0)で三角形を2つ並べる
    Vertex vertices[] = {
        { {-1.0f,  1.0f, 0.0f, 1.0f}, {0.0f, 0.0f} }, // 左上
        { { 1.0f,  1.0f, 0.0f, 1.0f}, {1.0f, 0.0f} }, // 右上
        { {-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f} }, // 左下

        { {-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f} }, // 左下
        { { 1.0f,  1.0f, 0.0f, 1.0f}, {1.0f, 0.0f} }, // 右上
        { { 1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 1.0f} }, // 右下
    };

    CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));

    hr = device->CreateCommittedResource(
        &uploadHeapProps,        // 変数のアドレスを渡す
        D3D12_HEAP_FLAG_NONE,
        &bufferResourceDesc,     // 変数のアドレスを渡す
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer_)
    );
    assert(SUCCEEDED(hr));

    // 頂点バッファにデータを転送
    Vertex* vertexMap = nullptr;
    vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexMap));
    std::copy(std::begin(vertices), std::end(vertices), vertexMap);
    vertexBuffer_->Unmap(0, nullptr);

    // ビューの作成
    vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(vertices);
    vertexBufferView_.StrideInBytes = sizeof(Vertex);
}

void PostEffect::PreDraw() {
    auto commandList = dxCommon_->GetCommandList();

    // 1. RenderTextureの状態を「レンダーターゲット（書き込み先）」に遷移（資料7枚目）
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = textureResource_.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList->ResourceBarrier(1, &barrier);

    // 2. 描画先を RenderTexture に、深度バッファを通常のものに設定
    // ※DSVは既存のゲッター、もしくはdsvのハンドルを渡してください
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxCommon_->GetDSVCPUDescriptorHandle(0);
    commandList->OMSetRenderTargets(1, &rtvHandle_, FALSE, &dsvHandle);

    // 3. レンダーターゲットと深度バッファのクリア
    FLOAT clearColor[] = { 1.0f, 0.0f, 0.0f, 1.0f }; // 初期化時と同じクリアカラー
    commandList->ClearRenderTargetView(rtvHandle_, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0,nullptr);

    // 4. ビューポートとシザー矩形の設定（テクスチャのサイズに合わせて設定）
    D3D12_VIEWPORT viewport{ 0.0f, 0.0f, (FLOAT)WinApp::kClientWidth, (FLOAT)WinApp::kClientHeight, 0.0f, 1.0f };
    D3D12_RECT scissorRect{ 0, 0, WinApp::kClientWidth, WinApp::kClientHeight };
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);
}

void PostEffect::PostDraw() {
    auto commandList = dxCommon_->GetCommandList();

    // 5. RenderTextureの状態を「ピクセルシェーダーリソース（読み込み元）」に安全に遷移（資料8枚目）
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = textureResource_.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    commandList->ResourceBarrier(1, &barrier);
}

void PostEffect::Draw() {
    auto commandList = dxCommon_->GetCommandList();

    // 1. パイプラインとルートシグネチャをポストエフェクト用に切り替え
    commandList->SetPipelineState(pipelineState_.Get());
    commandList->SetGraphicsRootSignature(rootSignature_.Get());

    // 2. 頂点バッファをセット
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

    // 3. SRV（描き込みが終わったテクスチャ）をシェーダーにバインド
    D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle = SrvManager::GetInstance()->GetGPUDescriptorHandle(srvIndex_);
    commandList->SetGraphicsRootDescriptorTable(0, srvGpuHandle);

    // 4. 描画コマンド（6頂点使って画面全体にテクスチャを貼り付ける）
    commandList->DrawInstanced(3, 1, 0, 0);
}

void PostEffect::Finalize() {
    // 特になし（ComPtrが自動で解放してくれます）
}