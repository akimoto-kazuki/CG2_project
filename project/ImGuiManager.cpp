#include "ImGuiManager.h"


void ImGuiManager::Initialize([[maybe_unused]]WinApp* winApp, [[maybe_unused]] DirectXCommon* dxCommon, [[maybe_unused]] SrvManager* srvManager)
{
#ifdef USE_IMGUI

	this->winApp_ = winApp;
	this->dxCommon_ = dxCommon;

	IMGUI_CHECKVERSION();
	// ImGuiのコンテキストを生成
	ImGui::CreateContext();
	// ImGuiのスタイルを設定
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(winApp->GetHwnd());
	ImGui_ImplDX12_InitInfo initInfo = {};

	// DirectX12用初期化情報の構築
	initInfo.Device = dxCommon_->GetDevice();
	initInfo.CommandQueue = dxCommon_->GetCommandQueue();
	initInfo.NumFramesInFlight = 2;
	initInfo.RTVFormat = dxCommon_->GetBackBufferFormat();
	initInfo.DSVFormat = dxCommon_->GetDepthBufferFormat();

	// SRV用デスクリプタヒープの設定
	// SrvManagerが管理しているヒープを取得して渡す
	// ※SrvManagerにGetDescriptorHeap()などのGetterが必要になります
	initInfo.SrvDescriptorHeap = srvManager->GetDescriptorHeap();

	srvHeap = srvManager->GetDescriptorHeap();

	// SRV確保用関数の設定 (ラムダ式)
	initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
	{
		// user_data経由でSrvManagerにアクセスするか、シングルトン等で確保
		// ここではスライドの例に倣い、SrvManagerからインデックスを確保してハンドルをセットする
		// ※実際の実装ではSrvManagerへのポインタを適切に渡す必要があります
		//SrvManager* srvManager_ = SrvManager::GetInstance();
		SrvManager* srvManager_ = static_cast<SrvManager*>(info->UserData);
		uint32_t index = srvManager_->Allocate();
		*out_cpu_handle = srvManager_->GetCPUDescriptorHandle(index);
		*out_gpu_handle = srvManager_->GetGPUDescriptorHandle(index);
	};

	initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {
		// 解放処理（スライド通り、特に行わない）
		};

	initInfo.UserData = srvManager;

	ImGui_ImplDX12_Init(&initInfo);

#endif // USE_IMGUI
}

void ImGuiManager::Finalize()
{
#ifdef USE_IMGUI

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

#endif // USE_IMGUI
}

void ImGuiManager::ImGuiBegin()
{
#ifdef USE_IMGUI

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

#endif // USE_IMGUI
}

void ImGuiManager::ImGuiEnd()
{
#ifdef USE_IMGUI

	ImGui::Render();

#endif // USE_IMGUI
}

void ImGuiManager::ImGuiDraw()
{
#ifdef USE_IMGUI

	ID3D12GraphicsCommandList* comamandList = dxCommon_->GetCommandList();
	// デスクリプタヒープの配列をセットするコマンド
	ID3D12DescriptorHeap* ppHeaps[] = { srvHeap };
	comamandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	// 描画コマンドを発行
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), comamandList);

#endif // USE_IMGUI
}
