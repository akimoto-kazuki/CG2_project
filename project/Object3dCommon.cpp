#include "Object3dCommon.h"

void Object3dCommon::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;

	GraphicsPipelineState();
}

void Object3dCommon::DrawCommon()
{
	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get()); // PSOを設定
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Object3dCommon::RootSignature()
{
	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// RootSignature作成。複数設定できるので配列。今回は結果1つだけなので長さ１の配列
	D3D12_ROOT_PARAMETER rootParametersObj[5] = {};

	// CBVを使う
	rootParametersObj[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	// b0のbと一致する
	rootParametersObj[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	// レジスタ番号０とバインド
	rootParametersObj[0].Descriptor.ShaderRegister = 0;					// b0のbと一致する。もしb11と紐づけたいなら11となる

	// CBVを使う
	rootParametersObj[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	// b0のbと一致する
	rootParametersObj[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // PixelShaderで使う

	// レジスタ番号０とバインド
	rootParametersObj[1].Descriptor.ShaderRegister = 0;					// b0のbと一致する。もしb11と紐づけたいなら11となる

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	rootParametersObj[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParametersObj[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParametersObj[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParametersObj[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	rootParametersObj[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParametersObj[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParametersObj[3].Descriptor.ShaderRegister = 1;

	rootParametersObj[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParametersObj[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParametersObj[4].Descriptor.ShaderRegister = 2;

	descriptionRootSignature.pParameters = rootParametersObj;
	descriptionRootSignature.NumParameters = _countof(rootParametersObj);

	D3D12_STATIC_SAMPLER_DESC staticSamplersObj[1] = {};
	staticSamplersObj[0].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	staticSamplersObj[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplersObj[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplersObj[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplersObj[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplersObj[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplersObj[0].ShaderRegister = 0;
	staticSamplersObj[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplersObj);
	descriptionRootSignature.pStaticSamplers = staticSamplersObj;

	// シリアライズしてバイナリにする

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlobObj = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlobObj = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlobObj, &errorBlobObj);
	if (FAILED(hr))
	{
		Logger::Log(reinterpret_cast<char*>(errorBlobObj->GetBufferPointer()));
		assert(false);
	}

	// バイナリを元に生成
	//Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignatureObj = nullptr;
	hr = dxCommon_->GetDevice()->CreateRootSignature(
		0,
		signatureBlobObj->GetBufferPointer(), signatureBlobObj->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
}

void Object3dCommon::GraphicsPipelineState()
{
	RootSignature();
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);
	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};

	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlobObj = dxCommon_->CompileShader(L"Object3D.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlobObj != nullptr);
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlobObj = dxCommon_->CompileShader(L"Object3D.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlobObj != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDescObj{};
	graphicsPipelineStateDescObj.pRootSignature = rootSignature.Get();      // RootSignature
	graphicsPipelineStateDescObj.InputLayout = inputLayoutDesc;             // InputLayout
	graphicsPipelineStateDescObj.VS = { vertexShaderBlobObj->GetBufferPointer(),
									 vertexShaderBlobObj->GetBufferSize() };// VertexShader
	graphicsPipelineStateDescObj.PS = { pixelShaderBlobObj->GetBufferPointer(),
									 pixelShaderBlobObj->GetBufferSize() }; // PixelShader
	graphicsPipelineStateDescObj.BlendState = blendDesc;                    // BlendState
	graphicsPipelineStateDescObj.RasterizerState = rasterizerDesc;          // RasterizerState
	// 書き込むRTVの情報
	graphicsPipelineStateDescObj.NumRenderTargets = 1;
	graphicsPipelineStateDescObj.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ（形状）のタイプ。三角形
	graphicsPipelineStateDescObj.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定（気にしなくて良い）
	graphicsPipelineStateDescObj.SampleDesc.Count = 1;
	graphicsPipelineStateDescObj.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// depthStencilDescの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDescObj{};
	// Depthの機能を有効化
	depthStencilDescObj.DepthEnable = true;
	// 書き込みをする
	depthStencilDescObj.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDescObj.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// depthStencilの設定
	graphicsPipelineStateDescObj.DepthStencilState = depthStencilDescObj;
	graphicsPipelineStateDescObj.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDescObj, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
}