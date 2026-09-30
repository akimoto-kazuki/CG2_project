#include "Framework.h"

void Framework::Initialiaze()
{
	// ウィンドウ
	winApp = new WinApp();
	winApp->Initialize();
	// キーの初期化
	input_ = new Input();
	input_->Initialize(winApp);
	// DirectX
	DirectXCommon::GetInstance()->Initialize(winApp);
	// SRV初期化
	srvManager = SrvManager::GetInstance();
	srvManager->Initialize();
	// ImGui
	imGuiManeger = new ImGuiManager;
	imGuiManeger->Initialize(winApp, srvManager);

	// Common
	// object3d
	Object3dCommon::GetInstance()->Initialize();
	// sprite
	SpriteCommon::GetInstance()->Initialize();
	// SkyBox
	SkyBoxCommon::GetInstance()->Initialize();

	TextureManager::GetInstance()->Initialize(srvManager);
	ModelManager::GetInstance()->Initialize();
	//パーティクルマネージャの初期化
	ParticleManager::GetInstance()->Initialize(srvManager);

	//FenceのSignalを待つためのイベントを作成する
	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	assert(fenceEvent != nullptr);

	//ログのディレクトリを用意する
	std::filesystem::create_directory("logs");
	//現在時刻を取得する(UTC時刻)
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	//ログファイルの名前にコンマ何秒はいらないので、削って秒にする
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	//日本時間(PCの設定時間)に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
	//formatを使って年月日_時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	//時刻を使ってファイル名を決定
	std::string logFilePath = std::string("logs/") + dateString + ".log";
	//ファイルを作って書き込み準備
	std::ofstream logStream(logFilePath);

	//ウィンドウを表示する
	ShowWindow(winApp->GetHwnd(), SW_SHOW);
}

void Framework::Finalize()
{
	CloseHandle(fenceEvent);

	// ImGuiの終了処理。詳細はさして重要ではないので解説は省略する
	// こういうもんである。初期化を逆順に行う
	imGuiManeger->Finalize();
	delete imGuiManeger;

	TextureManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();

	// 入力解放
	delete input_;

	// WindowsAPIの終了処理
	winApp->Finalize();
	// ウィンドウ解放
	delete winApp;
	// 
	delete srvManager;

	//リソースリークチェック
	IDXGIDebug1* debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}
}

void Framework::Update()
{
	if (winApp->ProcessMessage())
	{
		endRequst_ = true;
	}


}

void Framework::Run()
{
	// 初期化
	Initialiaze();

	while (true)
	{
		// 毎フレーム更新
		Update();
		// 終了リクエストが来たら抜ける
		if (IsEndRequst())
		{
			break;
		}
		// 描画
		Draw();
	}
	// ゲーム終了
	Finalize();
}
