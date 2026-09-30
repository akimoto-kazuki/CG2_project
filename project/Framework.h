#pragma once
// 入力デバイス
#include "Input.h"
//WindowsAPI
#include "WinApp.h"
// DirectX
#include "DirectXCommon.h"
// SrvManager
#include "SrvManager.h"
// ImGuiManager
#include "ImGuiManager.h"
// 
#include "SpriteCommon.h"
//
#include "Object3dCommon.h"
//
#include "SkyBoxCommon.h"
// マネージャ
// テキスト
#include "TextureManager.h"
// モデル
#include "ModelManager.h"
// パーティクル
#include "ParticleManager.h"

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

class Framework
{
public:

	// 初期化
	virtual void Initialiaze();
	// 終了
	virtual void Finalize();
	// 毎フレーム更新
	virtual void Update();
	// 描画
	virtual void Draw() = 0;

	// 終了フラグのチェック
	virtual bool IsEndRequst() { return endRequst_; }

	virtual ~Framework() = default;

	void Run();

	// ポインタ
	// 入力
	Input* input_ = nullptr;
	// ウィンドウ
	WinApp* winApp = nullptr;
	// DirectX
	DirectXCommon* dxCommon = nullptr;
	// SRV
	SrvManager* srvManager = nullptr;
	// ImGui
	ImGuiManager* imGuiManeger = nullptr;

	// オブジェクト
	Object3dCommon* object3dCommon = nullptr;
	// スプライト
	SpriteCommon* spriteCommon = nullptr;
	// スカイボックス
	SkyBoxCommon* skyBoxCommon = nullptr;

	HANDLE fenceEvent;

	bool endRequst_ = false;
};

