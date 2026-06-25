
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
// マネージャ
#include "TextureManager.h"
#include "ModelManager.h"
//カメラ
#include "Camera.h"
//
#include "SrvManager.h"
//
#include "MyMath.h"
// ImGuiManager
#include "ImGuiManager.h"
// スカイボックス
#include "SkyBox.h"
#include "SkyBoxCommon.h"

// ★ここに追加：パーティクル
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#ifdef USE_IMGUI	
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // DEBUG

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
#ifdef USE_IMGUI	
	if (ImGui_ImplWin32_WndProcHandler(hwnd,msg,wparam,lparam))
	{
		return true;
	}
#endif // DEBUE
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
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) 
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
	// スカイボックス
	SkyBoxCommon* skyBoxCommon = nullptr;
	SkyBox* skyBox = nullptr;
	// SRV
	SrvManager* srvManager = nullptr;
	// ImGui
	ImGuiManager* imGuiManeger = nullptr;
	// ウィンドウ
	winApp = new WinApp();
	winApp->Initialize();
	// キーの初期化
	input = new Input();
	input->Initialize(winApp);
	// DirectX
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);
	// SRV初期化
	srvManager = SrvManager::GetInstance();
	srvManager->Initialize(dxCommon);
	// ImGui
	imGuiManeger = new ImGuiManager;
	imGuiManeger->Initialize(winApp, dxCommon, srvManager);
	// Common
	// object3d
	object3dCommon = new Object3dCommon();
	object3dCommon->Initialize(dxCommon);
	// sprite
	spriteCommon = new SpriteCommon;
	spriteCommon->Initialize(dxCommon);
	// SkyBox
	skyBoxCommon = new SkyBoxCommon();
	skyBoxCommon->Initialize(dxCommon);
	
	// カメラ
	Camera* camera = new Camera();
	camera->SetRotate({ 0.0f,0.0f,0.0f });
	camera->SetTranslate({ 0.0f,0.0f,0.0f });
	object3dCommon->SetDefaultCamera(camera);
	skyBoxCommon->SetDefaultCamera(camera);
	// カメラの用
	Vector3 rotate = {0.0f,0.0f,0.0f};
	Vector3 translate = {0.0f,0.0f,0.0f};

	std::array<std::string, 2> spriteFile;

	spriteFile[0] = "resources/uvChecker.png";
	spriteFile[1] = "resources/monsterBall.png";

	TextureManager::GetInstance()->Initialize(dxCommon,srvManager);
	ModelManager::GetInstance()->Initialize(dxCommon);

	for (int i = 0; i < spriteFile.size(); i++)
	{
		TextureManager::GetInstance()->LoadTexture(spriteFile[i]);
	}
	ModelManager::GetInstance()->LoadModel("axis.obj");

	TextureManager::GetInstance()->LoadTexture("resources/skybox.dds");
	uint32_t skyboxTextureIndex = TextureManager::GetInstance()->GetTextureIndexByFilepath("resources/skybox.dds");

	// obj用
	Vector3 objPosition = { -2.0f,0.0f,10.0f };
	Vector3 objRotate = { 0.0f,3.0f,0.0f };
	// obj初期化
	object3d = new Object3d();
	object3d->Initialize(object3dCommon);
	object3d->SetModel("axis.obj");
	object3d->SetEnvironmentTextureIndex(skyboxTextureIndex); // ★ここで渡す！

	skyBox = new SkyBox();
	skyBox->Initialize(skyBoxCommon);
	// 3. 読み込んだテクスチャの番号を SkyBox に教える
	skyBox->SetTextureIndex(skyboxTextureIndex);

	// ★ここに追加：パーティクルマネージャの初期化
	ParticleManager::GetInstance()->Initialize(dxCommon, srvManager,camera);

	// 1. 画像の読み込みだけを行う（戻り値は受け取らない）
	TextureManager::GetInstance()->LoadTexture("Resources/circle2.png");
	TextureManager::GetInstance()->LoadTexture("Resources/gradationLine.png");

	// 2. これが「何枚目に読み込んだ画像か」で番号を直接決める
	// (例: 他に2枚読み込んでいて、これが3枚目の画像なら、0から数えて「2」になります)
	uint32_t particleTexIndex = TextureManager::GetInstance()->GetTextureIndexByFilepath("Resources/circle2.png"); // ★環境に合わせて 1 や 2 などに変えてみてください
	uint32_t particleRingTexIndex = TextureManager::GetInstance()->GetTextureIndexByFilepath("Resources/gradationLine.png");
	// ★ここに追加：パーティクルグループの作成とエミッターの生成
	// ※ 第2引数のSRVインデックスは、本来は TextureManager で読み込んだパーティクル用画像の番号を入れます。
	// 今回は仮に 0 を入れています。
	
	// パーティクル
	ParticleManager::GetInstance()->CreateGroup("magic", particleTexIndex);
	//ヒットエフェクト
	ParticleManager::GetInstance()->CreateGroup("Hit", particleTexIndex);
	ParticleManager::GetInstance()->CreateGroup("spark", particleTexIndex);
	ParticleManager::GetInstance()->CreateGroup("ring", particleRingTexIndex, true);


	// scale rotate translate
	Transform particleEffectTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,10.0f } };
	Transform particleHitEffectTransform = { {0.05f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,10.0f } };
	Transform particlesSparkEffectTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,10.0f } };
	Transform particleRingEffectTransform = { {1.0f,1.0f,1.0f},{0.0f,2.0f,0.0f},{0.0f,0.0f,10.0f } };

	// "magic" グループのパーティクルを、座標(0,0,0)から、1粒ずつ、0.1秒間隔で発生させるエミッターを作る
	ParticleEmitter* particleEmitterEffect = new ParticleEmitter("magic", particleEffectTransform, 1, 0.1f);
	ParticleEmitter* particleEmitterHitEffect = new ParticleEmitter("Hit", particleHitEffectTransform, 10, 2.0f);
	ParticleEmitter* particleEmitterSparkEffect = new ParticleEmitter("spark", particlesSparkEffectTransform, 20, 2.0f);
	ParticleEmitter* particleEmitterRingEffect = new ParticleEmitter("ring", particleRingEffectTransform, 4, 2.0f);

	// spr用
	Vector3 position = {0.0f,0.0f,0.0f};
	float rotation = 0.0f;
	Vector4 color = {1.0f,1.0f,1.0f,1.0f};
	Vector2 size = {1.0f,1.0f};

	// spr初期化
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
			// ImGuiの設定 始
			imGuiManeger->ImGuiBegin();

			// キー入力 始
			input->Update();
			if (input->PushKey(DIK_0))
			{
				OutputDebugStringA("Hit 0\n");
			}

			if (input->TriggerKey(DIK_1))
			{
				particleEmitterEffect->InputEffect();
			}
			if (input->TriggerKey(DIK_2))
			{
				particleEmitterHitEffect->InputHitEffect();
			}
			if (input->TriggerKey(DIK_3))
			{
				particleEmitterSparkEffect->InputSprakEffect();
			}
			if (input->TriggerKey(DIK_4))
			{
				particleEmitterRingEffect->InputHitEffect();
			}

			float pos = 0.0f;
			camera->Update();
			object3d->Update();
			object3d->SetRotate(objRotate);
			object3d->SetTranslate(objPosition);

			// 3. 更新
			skyBox->Update();

			// ★ここに追加：パーティクルの更新
			ParticleManager::GetInstance()->Update();	   // 発生した全パーティクルの移動と寿命チェック

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

			// ImGui
#ifdef USE_IMGUI			

			ImGui::ShowDemoWindow();

			ImGui::Text("Camera");
			ImGui::DragFloat3("cameraRotato", &rotate.x ,0.1f);
			ImGui::DragFloat3("cameraTranslate", &translate.x, 0.1f);
			ImGui::Text("Sprite");
			ImGui::DragFloat2("SpritePosition", &position.x, 0.1f);
			ImGui::DragFloat("SpriteRotation", &rotation, 0.1f);
			ImGui::DragFloat4("SpriteColor", &color.x, 0.1f);
			ImGui::DragFloat2("SpriteSize", &size.x, 0.1f);
			ImGui::Text("object3D");
			ImGui::DragFloat3("ObjectPosition", &objPosition.x, 0.1f);
			ImGui::DragFloat3("ObjectRotation", &objRotate.x, 0.1f);
			// 1. 現在の数値を Object3d から取得してローカル変数に入れる
			float envCoef = object3d->GetEnvironmentCoefficient();

			// 2. ImGuiのスライダーでローカル変数の値をいじる
			if (ImGui::SliderFloat("Environment Rate", &envCoef, 0.0f, 1.0f))
			{
				// 3. スライダーが動いて値が変わったら、新しい数値を Object3d にセットする
				object3d->SetEnvironmentCoefficient(envCoef);
			}
			
#endif // USE_IMGUI
			// ImGuiの設定 終
			imGuiManeger->ImGuiEnd();

			dxCommon->PreDraw();

			srvManager->PreDraw();

			object3dCommon->DrawCommon();

			//object3d->Draw();

			// 4. 描画
			skyBoxCommon->DrawCommon(); // Skybox用のルートシグネチャ・PSOに切り替え
			//skyBox->Draw();             // 引数なしでスッキリ呼び出せます！

			// ★ここに追加：パーティクルの描画
			ParticleManager::GetInstance()->Draw();

			/*spriteCommon->DrawCommon();

			for (Sprite* sprite : sprites_)
			{	
				sprite->Draw();
			}
			*/
			// ImGuiの描画
			imGuiManeger->ImGuiDraw();
		}
		dxCommon->PostDraw();
	}

	//出力ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	CloseHandle(fenceEvent);

	
	// WindowsAPIの終了処理
	winApp->Finalize();

	delete sprite;
	delete object3d;
	delete skyBox;
	delete camera;
	// ★ここに追加：エミッターの削除
	// (ParticleManagerはシングルトンなのでdelete不要です)
	delete particleEmitterEffect;
	delete particleEmitterHitEffect;
	delete particleEmitterSparkEffect;
	delete particleEmitterRingEffect;

	delete spriteCommon;
	delete object3dCommon;
	delete skyBoxCommon;
	delete imGuiManeger;

	TextureManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();

	delete srvManager;

	// ウィンドウ解放
	delete winApp;
	// 入力解放
	delete input;
	// DirectXの解放
	delete dxCommon;


	// ImGuiの終了処理。詳細はさして重要ではないので解説は省略する
	// こういうもんである。初期化を逆順に行う
	imGuiManeger->Finalize();

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

