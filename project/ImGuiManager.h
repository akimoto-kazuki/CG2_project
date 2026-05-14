#pragma once
//WindowsAPI
#include "WinApp.h"
#include <d3d12.h>     // 先にDX12本体
#include <dxgi1_4.h>
#ifdef USE_IMGUI
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#endif
// DirectX
#include "DirectXCommon.h"
//
#include "SrvManager.h"

class ImGuiManager
{
public:

	// 初期化
	void Initialize(WinApp* winApp, DirectXCommon* dxCommon, SrvManager* srvManager);
	// 後始末
	void Finalize();

	/// <summary>
	/// ImGui受付開始
	/// </summary>
	void ImGuiBegin();

	/// <summary>
	/// Imgui受付終了
	/// </summary>
	void ImGuiEnd();

	/// <summary>
	/// 画面の描画
	/// </summary>
	void ImGuiDraw();

private:

	// ウィンドウ
	WinApp* winApp_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;

	ID3D12DescriptorHeap* srvHeap;
};

