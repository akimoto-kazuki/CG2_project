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
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

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
	// WindowsAPIの終了処理
	winApp->Finalize();

	// ウィンドウ解放
	delete winApp;
	// 入力解放
	delete input_;
	// DirectXの解放
	delete dxCommon;
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
