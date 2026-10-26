#define DIRECTINPUT_VERSION		0x0800
#include <dinput.h>
#include <cassert>
#include <Windows.h>
#include <wrl.h>

#include "WinApp.h"

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

using namespace Microsoft::WRL;

class Input
{
public:
	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
public:

	// インスタンス取得
	static Input* GetInstance();

	// ★ コピー・代入の禁止
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

	// 初期化
	void Initialize(WinApp* winApp);
	// 更新
	void Update();

	/// <summary>
	/// 現在キーが押されているかをチェック
	/// </summary>
	/// <param name="keyNumber">キー番号( DIK_0 等)</param>
	/// <returns>押されている間は true</returns>

	bool IsKeyDown(BYTE keyNumber) const;

	/// <summary>
	/// 前フレームでは離されていて、今フレームで押されたかをチェック
	/// </summary>
	/// <param name="keyNumber">キー番号( DIK_0 等)</param>
	/// <returns>押された最初のフレームだけ true</returns>
	bool WasKeyPressed(BYTE keyNumber) const;

	// マウス操作
	POINT GetMouseDelta() const;
	bool IsMouseButtonDown(int virtualKey) const;

private:

	// ★ コンストラクタを private へ移動
	Input() = default;
	~Input() = default;

	IDirectInput8* directInput = nullptr;
	// キーボードのデバイス
	ComPtr<IDirectInputDevice8> keyboard;

	BYTE key[256] = {};

	BYTE keyPre[256] = {};
	POINT mousePosition_{};
	POINT mousePositionPre_{};
	bool isMousePositionInitialized_ = false;

	// ポインタ
	// WindowsAPI
	WinApp* winApp = nullptr;

};
