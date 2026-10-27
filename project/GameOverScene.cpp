#include "GameOverScene.h"

#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "SceneManager.h"
#include "SpriteCommon.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "WinApp.h"
#include <cmath>

void GameOverScene::Initialize()
{
	TextureManager::GetInstance()->LoadTexture("resources/white.png");
	TextureManager::GetInstance()->LoadTexture("resources/gameover_ui.png");

	background_ = new Sprite();
	background_->Initialize("resources/white.png");
	background_->SetPosition({ 0.0f, 0.0f, 0.0f });
	// Spriteの基準クワッドは100x100なので、ピクセル寸法を100で割って倍率にする
	background_->SetSize({ WinApp::kClientWidth / 100.0f, WinApp::kClientHeight / 100.0f });
	background_->SetColor({ 0.025f, 0.008f, 0.015f, 1.0f });

	accent_ = new Sprite();
	accent_->Initialize("resources/white.png");
	accent_->SetSize({ WinApp::kClientWidth / 100.0f, 0.05f });
	accent_->SetColor({ 0.75f, 0.06f, 0.08f, 1.0f });
	accent_->SetPosition({ 0.0f, 505.0f, 0.0f });

	textOverlay_ = new Sprite();
	textOverlay_->Initialize("resources/gameover_ui.png");
	textOverlay_->SetPosition({ 0.0f, 0.0f, 0.0f });
	textOverlay_->SetSize({ WinApp::kClientWidth / 100.0f, WinApp::kClientHeight / 100.0f });
	textOverlay_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });

	timer_ = 0.0f;
}

void GameOverScene::Finalize()
{
	delete background_;
	background_ = nullptr;
	delete accent_;
	accent_ = nullptr;
	delete textOverlay_;
	textOverlay_ = nullptr;
}

void GameOverScene::Update()
{
	auto input = Input::GetInstance();
	input->Update();
	auto imGuiManager = ImGuiManager::GetInstance();
	imGuiManager->ImGuiBegin();
	imGuiManager->ImGuiEnd();

	// SPACEで再挑戦、ENTERでタイトルへ戻る
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

	timer_ += 1.0f / 60.0f;
	const float pulse = 0.5f + 0.5f * std::sin(timer_ * 3.0f);
	accent_->SetColor({ 0.5f + 0.35f * pulse, 0.015f, 0.025f, 1.0f });
	accent_->SetPosition({ 0.0f, 505.0f + std::sin(timer_ * 1.5f) * 2.0f, 0.0f });
	// 遷移ブロックが消えきるまでは文字スプライトを透明にしておく
	textOverlay_->SetColor({ 1.0f, 1.0f, 1.0f, sceneManager_->IsTransitionFinished() ? 1.0f : 0.0f });
	background_->Update();
	accent_->Update();
	textOverlay_->Update();
}

void GameOverScene::Draw()
{
	auto dxCommon = DirectXCommon::GetInstance();
	SrvManager::GetInstance()->PreDraw();
	dxCommon->PreDraw();

	SpriteCommon::GetInstance()->DrawCommon();
	background_->Draw();
	accent_->Draw();
	sceneManager_->DrawTransitionEffect();
	textOverlay_->Draw();
	ImGuiManager::GetInstance()->ImGuiDraw();

	dxCommon->PostDraw();
}
