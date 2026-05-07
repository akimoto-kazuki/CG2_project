#pragma once
#include "DirectXCommon.h"

class SrvManager
{
private:
	DirectXCommon* directXCommon = nullptr;
	
	// サイズ
	uint32_t descriptorSize;
	// SRVヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	// 次に使用するSRVインデックス
	uint32_t useIndex = 0;

public:

	// 最大SRV
	static const uint32_t kMaxSRVCount;

	// 初期化
	void Initialize(DirectXCommon*dxCommon);

	uint32_t Allocate();

	/// <summary>
	/// SRVの指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	/// <summary>
	/// SRVの指定番号のGPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);
	// SRV生成(テクスチャ用)
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels);
	// SRV生成(Structured Buffer用)
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);
	// ヒープセットコマンド
	void PreDraw();
	// SRVセットコマンド
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);
};

