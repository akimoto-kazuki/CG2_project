#include "GameClearScene.h"
// シングルトン
#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "SkyBoxCommon.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ParticleManager.h"
#include "SrvManager.h"
#include "Input.h"
#include "DirectXCommon.h"
#include "WinApp.h"
#include <cmath>

void GameClearScene::Initialize()
{
	auto dxCommon = DirectXCommon::GetInstance();

	// GameOverSceneと同じ構成で、暗い背景とクリア文字のスプライトを用意する。
	TextureManager::GetInstance()->LoadTexture("resources/white.png");
	TextureManager::GetInstance()->LoadTexture("resources/gameclear_ui.png");
	background_ = new Sprite();
	background_->Initialize("resources/white.png");
	background_->SetSize({ WinApp::kClientWidth / 100.0f, WinApp::kClientHeight / 100.0f });
	background_->SetColor({ 0.008f, 0.025f, 0.02f, 1.0f });
	accent_ = new Sprite();
	accent_->Initialize("resources/white.png");
	accent_->SetSize({ WinApp::kClientWidth / 100.0f, 0.05f });
	accent_->SetColor({ 0.08f, 0.72f, 0.42f, 1.0f });
	accent_->SetPosition({ 0.0f, 505.0f, 0.0f });
	textOverlay_ = new Sprite();
	textOverlay_->Initialize("resources/gameclear_ui.png");
	textOverlay_->SetSize({ WinApp::kClientWidth / 100.0f, WinApp::kClientHeight / 100.0f });
	textOverlay_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
	timer_ = 0.0f;

	// --- カメラ ---
	camera = new Camera();
	// カメラの数値設定
	rotate = { 0.0f,0.0f,0.0f };
	translate = { 0.0f,0.0f,-10.0f };
	camera->SetRotate(rotate);
	camera->SetTranslate(translate);
	Object3dCommon::GetInstance()->SetDefaultCamera(camera);
	SkyBoxCommon::GetInstance()->SetDefaultCamera(camera);

	// --- 描画オブジェクトパラメータ設定 ---
	// オブジェクト
	//	プレイヤー
	playerPosition = { -7.0f,-4.0f,10.0f };
	playerRotate = { 0.0f,3.0f,0.0f };
	velocityY = 0.0f;         // Y軸方向の現在の速度
	gravity = -0.025f;        // 重力（毎フレーム下に向かって引っ張る力）
	jumpPower = 0.3f;         // ジャンプ力（上に飛び上がる初速）
	//	敵
	enemyPosition = { -10.0f,-4.0f,10.0f };
	enemyRotate = { 0.0f,3.0f,0.0f };
	// スペース
	spacePosition = { -0.3f,-0.5f,0.0f };
	spaceRotate = { 0.0f,0.0f,0.0f };
	// タイトル名
	titlePosition = { 0.0f,1.0f,0.0f };
	titleRotate = { 0.0f,0.0f,0.0f };
	// タイトル背景
	titleBackPosition = { 0.0f,0.0f,10.0f };
	titleBackRotate = { 0.0f,0.0f,0.0f };
	titleBackSize = { 5.0f,5.0f,1.0f };

	// isEnemyAlive = true; bool型だから宣言しなくていい わかりやすくするために置いてる
	//  スプライト
	spritePosition = { 0.0f,0.0f,0.0f };
	spriteRotation = 0.0f;
	spriteColor = { 1.0f,1.0f,1.0f,1.0f };
	spriteSize = { 1.0f,1.0f };

	// --- テクスチャ＆モデル読み込み ---
	spriteFile[0] = "resources/white.png";
	spriteFile[1] = "resources/monsterBall.png";
	spriteFile[2] = "resources/sky_sphere.png";
	for (int i = 0; i < spriteFile.size(); i++)
	{
		TextureManager::GetInstance()->LoadTexture(spriteFile[i]);
	}
	// アニメーション
	ModelManager::GetInstance()->LoadModel("walk.gltf");
	// オブジェクト
	ModelManager::GetInstance()->LoadModel("space.obj");
	ModelManager::GetInstance()->LoadModel("title.obj");
	ModelManager::GetInstance()->LoadModel("titleBack.obj");
	// スカイドーム
	TextureManager::GetInstance()->LoadTexture("resources/skybox.dds");
	uint32_t skyboxTextureIndex = TextureManager::GetInstance()->GetTextureIndexByFilepath("resources/skybox.dds");

	// --- エフェクト・レンダー関連初期化 ---
	// エフェクト
	postEffect = new PostEffect();
	postEffect->Initialize();
	// レンダー
	lineRenderer = new LineRenderer();
	lineRenderer->Initialize();

	// --- 3Dオブジェクト & SkyBox 初期化 ---
	// 3Dオブジェクト
	//  プレイヤー
	object3d = new Object3d();
	object3d->Initialize();
	object3d->SetModel("walk.gltf");
	object3d->SetAnimation("resources", "walk.gltf");
	object3d->SetEnvironmentTextureIndex(skyboxTextureIndex);
	//  敵
	enemy3d = new Object3d();
	enemy3d->Initialize();
	enemy3d->SetModel("walk.gltf");
	enemy3d->SetAnimation("resources", "walk.gltf");
	enemy3d->SetEnvironmentTextureIndex(skyboxTextureIndex);
	// スペース
	space3d = new Object3d();
	space3d->Initialize();
	space3d->SetModel("space.obj");
	space3d->SetEnvironmentTextureIndex(skyboxTextureIndex);
	// タイトル
	title3d = new Object3d();
	title3d->Initialize();
	title3d->SetModel("title.obj");
	title3d->SetEnvironmentTextureIndex(skyboxTextureIndex);
	// タイトル背景
	titleBack3d = new Object3d();
	titleBack3d->Initialize();
	titleBack3d->SetModel("titleBack.obj");
	titleBack3d->SetEnvironmentTextureIndex(skyboxTextureIndex);

	// SkyBox
	skyBox = new SkyBox();
	skyBox->Initialize();
	// 読み込んだテクスチャの番号を SkyBox に教える
	skyBox->SetTextureIndex(skyboxTextureIndex);

	// --- スプライト初期化 ---
	for (uint32_t i = 0; i < 5; ++i)
	{
		Sprite* sprite = new Sprite();
		sprite->Initialize(spriteFile[i % 2]);
		sprites_.push_back(sprite);
	}
}

void GameClearScene::Finalize()
{
	delete background_;
	background_ = nullptr;
	delete accent_;
	accent_ = nullptr;
	delete textOverlay_;
	textOverlay_ = nullptr;
	for (Sprite* sprite : sprites_)
	{
		delete sprite;
	}
	sprites_.clear();
	delete object3d;
	delete enemy3d;
	delete space3d;
	delete title3d;
	delete titleBack3d;
	delete skyBox;
	delete camera;
	delete lineRenderer;

	delete postEffect;
}

void GameClearScene::Update()
{
	auto input = Input::GetInstance();
	input->Update();

	// Spaceでゲームプレイへ、Enterでタイトルへ戻る。
	if (input->WasKeyPressed(DIK_SPACE))
	{
		sceneManager_->StartTransition(SceneType::GamePlay);
		return;
	}
	if (input->WasKeyPressed(DIK_RETURN))
	{
		sceneManager_->StartTransition(SceneType::Title);
		return;
	}

	// ゲームオーバー画面と同じく、アクセント線を揺らし、遷移完了後に文字を表示する。
	timer_ += 1.0f / 60.0f;
	const float pulse = 0.5f + 0.5f * std::sin(timer_ * 3.0f);
	accent_->SetColor({ 0.04f + 0.08f * pulse, 0.55f + 0.2f * pulse, 0.3f + 0.12f * pulse, 1.0f });
	accent_->SetPosition({ 0.0f, 505.0f + std::sin(timer_ * 1.5f) * 2.0f, 0.0f });
	textOverlay_->SetColor({ 1.0f, 1.0f, 1.0f, sceneManager_->IsTransitionFinished() ? 1.0f : 0.0f });
	background_->Update();
	accent_->Update();
	textOverlay_->Update();
}
void GameClearScene::Draw()
{
	auto dxCommon = DirectXCommon::GetInstance();
	SrvManager::GetInstance()->PreDraw();

	dxCommon->PreDraw();
	SpriteCommon::GetInstance()->DrawCommon();
	background_->Draw();
	accent_->Draw();
	sceneManager_->DrawTransitionEffect();
	textOverlay_->Draw();
	dxCommon->PostDraw();
}
