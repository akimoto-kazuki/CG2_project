#include "Game.h"
#include "Framework.h"

#ifdef USE_IMGUI	
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // DEBUG

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

void Game::Initialiaze()
{
	Framework::Initialiaze();

	// シーンの生成と初期化
	scene_ = new TitleScene();
	//scene_ = new GamePlayScene();
	scene_->Initialiaze();
}

void Game::Finalize()
{	
	scene_->Finalize();
	delete scene_;
	scene_ = nullptr;
	Framework::Finalize();

	//出力ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");
}

void Game::Update()
{
	Framework::Update();
	// シーンの更新
	scene_->Update();
}

void Game::Draw()
{
	// シーンの描画
	scene_->Draw();
}
