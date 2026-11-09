#include "ImGuiManager.h"
#include <string>

ImGuiManager* ImGuiManager::GetInstance()
{
	static ImGuiManager instance;
	return &instance;
}

void ImGuiManager::Initialize([[maybe_unused]]WinApp* winApp, [[maybe_unused]] SrvManager* srvManager)
{
#ifdef USE_IMGUI

	this->winApp_ = winApp;

	IMGUI_CHECKVERSION();
	// ImGuiのコンテキストを生成
	ImGui::CreateContext();
	// MergeMode の重ね先になる標準フォントを先に登録する。
	ImGui::GetIO().Fonts->AddFontDefault();
	// Windows標準の日本語フォントを標準フォントへ重ね、HUDの日本語を読めるようにする。
	char windowsDirectory[MAX_PATH] = {};
	GetWindowsDirectoryA(windowsDirectory, MAX_PATH);
	std::string japaneseFontPath = std::string(windowsDirectory) + "\\Fonts\\meiryo.ttc";
	if (GetFileAttributesA(japaneseFontPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		japaneseFontPath = std::string(windowsDirectory) + "\\Fonts\\msgothic.ttc";
	}
	if (GetFileAttributesA(japaneseFontPath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		ImFontConfig japaneseFontConfig{};
		japaneseFontConfig.MergeMode = true;
		ImGui::GetIO().Fonts->AddFontFromFileTTF(
			japaneseFontPath.c_str(), 16.0f, &japaneseFontConfig,
			ImGui::GetIO().Fonts->GetGlyphRangesJapanese());
	}
	// ImGuiのスタイルを設定
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(winApp->GetHwnd());
	ImGui_ImplDX12_InitInfo initInfo = {};

	// DirectX12用初期化情報の構築
	initInfo.Device = DirectXCommon::GetInstance()->GetDevice();
	initInfo.CommandQueue = DirectXCommon::GetInstance()->GetCommandQueue();
	initInfo.NumFramesInFlight = 2;
	initInfo.RTVFormat = DirectXCommon::GetInstance()->GetBackBufferFormat();
	initInfo.DSVFormat = DirectXCommon::GetInstance()->GetDepthBufferFormat();

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

	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();
	// デスクリプタヒープの配列をセットするコマンド
	ID3D12DescriptorHeap* ppHeaps[] = { srvHeap };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	// 描画コマンドを発行
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

#endif // USE_IMGUI
}
