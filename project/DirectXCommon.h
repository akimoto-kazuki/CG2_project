#pragma once
#include <format>

#include "Logger.h"
#include "StringUtility.h"
#include "WinApp.h"
//DirectX12のinclude
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
// imGuiを使うため
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"


class DirectXCommon
{
public:

	// 初期化
	void Initialize(WinApp* winApp);
	// デバイスの初期化
	void DeviceInitialize();
	// コマンド関連の初期化
	void CommandInitialize();
	// スワップチェーンの生成
	void SwapChainInitialize();
	// 深度バッファの生成
	void BufferInitialize();
	// 各種デスクリプタヒープの生成
	void DescriptorInitialize();
	// レンダーターゲットビューの初期化
	void RenderTargetInitialize();
	// 深度ステンシルビューの初期化
	void StencilInitialize();
	// フェンスの生成
	void FenceInitialize();
	// ビューポート矩形の初期化
	void ViewPortInitialize();
	// シザリング矩形の初期化
	void ScissoringInitialize();
	// DXCコンパイラの生成
	void DxcCompilerInitialize();
	// ImGuiの初期化
	void ImGuiInitialize();

	/// <summary>
	/// デスクリプタヒープを生成する
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>
		CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	
	/// <summary>
	/// SRVの指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

	/// <summary>
	/// SRVの指定番号のGPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

private:

	// DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	HRESULT hr;
	//使用するアダプタ用の変数。最初にnullptrを入れておく
	IDXGIAdapter4* useAdapter = nullptr;

	ID3D12Device* device = nullptr;
	//スワップチェーンを生成する
	IDXGISwapChain4* swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	//コマンドアロケータを生成する
	ID3D12CommandAllocator* commandAllocator = nullptr;
	//コマンドリストを生成する
	ID3D12GraphicsCommandList* commandList = nullptr;
	//コマンドキューを生成する
	ID3D12CommandQueue* commandQueue = nullptr;
	// SRVヒープ
	ID3D12DescriptorHeap* srvDescriptorHeap = nullptr;
	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

	// ポインタ
	// WindowsAPI
	WinApp* winApp = nullptr;

	/// <summary>
	/// 指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	static D3D12_CPU_DESCRIPTOR_HANDLE
		GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	/// <summary>
	/// 指定番号のGPUデスクリプタハンドルを取得する
	/// </summary>
	static D3D12_GPU_DESCRIPTOR_HANDLE
		GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);


	
};

