#include "SrvManager.h"
#include <cassert>

SrvManager* SrvManager::instance = nullptr;

const uint32_t SrvManager::kMaxSRVCount = 512;

SrvManager* SrvManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = new SrvManager;
	}
	return instance;
}

void SrvManager::Initialize(DirectXCommon* dxCommon)
{
	this->directXCommon = dxCommon;
	// デスクリプタヒープの生成
	descriptorHeap = directXCommon->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);
	// デスクリプタ1個分のサイズを取得して記録
	descriptorSize = directXCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

uint32_t SrvManager::Allocate()
{
	// 「現在のインデックスが上限より小さいこと」を保証する
	assert(useIndex < kMaxSRVCount);
	// return する番号を一旦記録しておく
	int index = useIndex;
	// 次回のために番号１進める
	useIndex++;
	// 上で記録した番号をreturn
	return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels)
{
	// 1. SRVの設定 (srvDescの各項目を埋める)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = Format; // 引数で受け取ったフォーマット
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // 基本はこのデフォルト値でOK
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャとして扱う
	srvDesc.Texture2D.MipLevels = MipLevels; // 引数で受け取ったミップレベル

	// 2. デバイスを使ってSRVを生成
	directXCommon->GetDevice()->CreateShaderResourceView(
		pResource,
		&srvDesc,
		GetCPUDescriptorHandle(srvIndex) // 管理しているヒープから指定インデックスのCPUハンドルを取得
	);
}

void SrvManager::CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride)
{
	// 1. SRVの設定 (Structured Buffer用)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN; // Structured Bufferの場合はUNKNOWNにする決まり
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER; // バッファとして扱う
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = numElements; // 配列の要素数
	srvDesc.Buffer.StructureByteStride = structureByteStride; // 1要素あたりのバイトサイズ
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	// 2. デバイスを使ってSRVを生成
	directXCommon->GetDevice()->CreateShaderResourceView(
		pResource,
		&srvDesc,
		GetCPUDescriptorHandle(srvIndex)
	);
}

void SrvManager::PreDraw()
{
	// 描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap.Get() };
	directXCommon->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
}

void SrvManager::SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex)
{
	directXCommon->GetCommandList()->SetGraphicsRootDescriptorTable(RootParameterIndex, GetGPUDescriptorHandle(srvIndex));
}
