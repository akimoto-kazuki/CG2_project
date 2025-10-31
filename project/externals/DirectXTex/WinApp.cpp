#include "DirectXTexP.h"
#include "WinApp.h"

void WinApp::Initialize(LPCWSTR lpszClassName, HINSTANCE hInstance)
{
	//クライアント領域のサイズ
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	//ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0,0,kClientWidth,kClientHeight };

	//クライアント領域を元に実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウの生成
	HWND hwnd = CreateWindow(
		lpszClassName,				//利用するクラス名
		L"CG2_LE2D_03_アキモト_カズキ",	//タイトルバーの文字
		WS_OVERLAPPEDWINDOW,			//よく見るウィンドウスタイル
		CW_USEDEFAULT,					//表示X座標(Windowsに任せる)
		CW_USEDEFAULT,					//表示Y座標(WindowsOSに任せる)
		wrc.right - wrc.left,			//ウィンドウの横幅
		wrc.bottom - wrc.top,			//ウィンドウの縦幅
		nullptr,						//親ウィンドウハンドル
		nullptr,						//メニューハンドル
		hInstance,					//インスタンスハンドル
		nullptr							//オプション
	);
}

void WinApp::Update()
{
}
