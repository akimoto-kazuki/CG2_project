
#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
//ファイルやディレクトリに関する操作を行うライブラリ
#include <filesystem>
//ファイルに書いたり読んだりするライブラリ
#include<fstream>
//時間を扱うライブラリ
#include<chrono>
//DirectX12のinclude
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dxgidebug.h>
#include <dxcapi.h>
// imGuiを使うため
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
// DirectXを使うため
#include "externals/DirectXTex/DirectXTex.h"
// 入力デバイス
#include "Input.h"
//WindowsAPI
#include "WinApp.h"
// DirectX
#include "DirectXCommon.h"
// スプライト
#include "Sprite.h"
#include "SpriteCommon.h"
// オブジェクト
#include "Object3d.h"
#include "Object3dCommon.h"

#include "TextureManager.h"
#include "ModelManager.h"
//カメラ
#include "Camera.h"

#include "MyMath.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include<fstream>
#include<sstream>

//libのリンク
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

using namespace MyMath;

//ウィンドウプロシーシャ
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd,msg,wparam,lparam))
	{
		return true;
	}
	//メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		//ウィンドウが破棄された
	case WM_DESTROY:
		//OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	//標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

//Windouwsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) 
{
	CoInitializeEx(0, COINIT_MULTITHREADED);

	// ポインタ
	// 入力
	Input* input = nullptr;
	// ウィンドウ
	WinApp* winApp = nullptr;
	// DirectX
	DirectXCommon* dxCommon = nullptr;
	// オブジェクト
	Object3dCommon* object3dCommon = nullptr;
	Object3d* object3d = nullptr;
	// スプライト
	SpriteCommon* spriteCommon = nullptr;
	Sprite* sprite = nullptr;
	
	// ウィンドウ
	winApp = new WinApp();
	winApp->Initialize();
	// キーの初期化
	input = new Input();
	input->Initialize(winApp);
	// DirectX
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	object3dCommon = new Object3dCommon();
	object3dCommon->Initialize(dxCommon);

	spriteCommon = new SpriteCommon;
	spriteCommon->Initialize(dxCommon);

	// カメラ
	Camera* camera = new Camera();
	camera->SetRotate({ 0.0f,0.0f,0.0f });
	camera->SetTranslate({ 0.0f,0.0f,0.0f });
	object3dCommon->SetDefaultCamera(camera);

	Vector3 rotate = {0.0f,0.0f,0.0f};
	Vector3 translate = {0.0f,0.0f,0.0f};

	std::array<std::string, 2> spriteFile;

	spriteFile[0] = "resources/uvChecker.png";
	spriteFile[1] = "resources/monsterBall.png";

	TextureManager::GetInstance()->Initialize(dxCommon);
	ModelManager::GetInstance()->Initialize(dxCommon);

	for (int i = 0; i < spriteFile.size(); i++)
	{
		TextureManager::GetInstance()->LoadTexture(spriteFile[i]);
	}
	ModelManager::GetInstance()->LoadModel("plane.obj");

	Vector3 position = {0.0f,0.0f,0.0f};
	float rotation = 0.0f;
	Vector4 color = {1.0f,1.0f,1.0f,1.0f};
	Vector2 size = {1.0f,1.0f};

	object3d = new Object3d();
	object3d->Initialize(object3dCommon);
	object3d->SetModel("plane.obj");

	std::vector<Sprite*> sprites_;
	for (uint32_t i = 0; i < 5; ++i)
	{
		Sprite* sprite = new Sprite();
		sprite->Initialize(spriteCommon, spriteFile[i%2]);
		sprites_.push_back(sprite);
	}
	
	//FenceのSignalを待つためのイベントを作成する
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

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
	
	//ウィンドウの×ボタンが押されるまでループ
	while (true) 
	{
		if (winApp->ProcessMessage())
		{
			break;
		}
		else 
		{
			// キー入力 始
			input->Update();
			if (input->PushKey(DIK_0))
			{
				OutputDebugStringA("Hit 0\n");
			}

			float pos = 0.0f;
			camera->Update();
			object3d->Update();
			object3d->SetRotation(rotation);

			for (Sprite* sprite : sprites_)
			{
				Vector3 changePos = {pos,0.0f,0.0f};
				pos += 200.0f;
				sprite->SetRotation(rotation);
				sprite->SetSize(size);
				sprite->SetPosition(Add(position,changePos));
				sprite->SetColor(color);
				sprite->Update();
				
			}
			
			camera->SetRotate(rotate);
			camera->SetTranslate(translate);
			//ゲームの処理

			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			// 開発用UIの処理。実際に開発用UIを出す場合はここをゲーム固有の処理に置き換える
			//ImGui::ShowDemoWindow();
			
			ImGui::Begin("Settings");
			ImGui::DragFloat3("cameraRotato", &rotate.x ,0.1f);
			ImGui::DragFloat3("cameraTranslate", &translate.x, 0.1f);
			ImGui::DragFloat2("SpritePosition", &position.x, 0.1f);
			ImGui::DragFloat("SpriteRotation", &rotation, 0.1f);
			ImGui::DragFloat4("SpriteColor", &color.x, 0.1f);
			ImGui::DragFloat2("SpriteSize", &size.x, 0.1f);
			ImGui::End();

			dxCommon->PreDraw();

			// ImGuiの内部コマンドを生成する
			ImGui::Render();

			object3dCommon->DrawCommon();

			object3d->Draw();

			spriteCommon->DrawCommon();

			for (Sprite* sprite : sprites_)
			{	
				//sprite->Draw();
			}

			// 実際のcommandListのImGuiの描画コマンドを積む
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

			//GPUにコマンドリストの実行を行わせる
			ID3D12CommandList* commandLists[] = { dxCommon->GetCommandList() };
			
		}
		dxCommon->PostDraw();
	}

	//出力ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	CloseHandle(fenceEvent);
	// WindowsAPIの終了処理
	winApp->Finalize();

	TextureManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();
	delete sprite;
	delete object3d;
	delete camera;
	
	// ウィンドウ解放
	delete winApp;
	// 入力解放
	delete input;
	// DirectXの解放
	delete dxCommon;

	delete spriteCommon;
	delete object3dCommon;

	// ImGuiの終了処理。詳細はさして重要ではないので解説は省略する
	// こういうもんである。初期化を逆順に行う
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// 何かのReleaseが足りない

	//リソースリークチェック
	IDXGIDebug1* debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}

	

	return 0;
}

