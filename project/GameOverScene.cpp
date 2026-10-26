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

#ifdef USE_IMGUI
#include "imgui.h"
#endif

void GameOverScene::Initialize()
{
	TextureManager::GetInstance()->LoadTexture("resources/white.png");

	background_ = new Sprite();
	background_->Initialize("resources/white.png");
	background_->SetPosition({ 0.0f, 0.0f, 0.0f });
	background_->SetSize({ static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight) });
	background_->SetColor({ 0.025f, 0.008f, 0.015f, 1.0f });

	accent_ = new Sprite();
	accent_->Initialize("resources/white.png");
	accent_->SetSize({ static_cast<float>(WinApp::kClientWidth), 5.0f });
	accent_->SetColor({ 0.75f, 0.06f, 0.08f, 1.0f });
	accent_->SetPosition({ 0.0f, 505.0f, 0.0f });

	timer_ = 0.0f;
}

void GameOverScene::Finalize()
{
	delete background_;
	background_ = nullptr;
	delete accent_;
	accent_ = nullptr;
}

void GameOverScene::Update()
{
	auto input = Input::GetInstance();
	input->Update();
	auto imGuiManager = ImGuiManager::GetInstance();
	imGuiManager->ImGuiBegin();
#ifdef USE_IMGUI
	if (sceneManager_->IsTransitionFinished())
	{
		ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), 60.0f, ImVec2(430.0f, 245.0f), IM_COL32(235, 225, 225, 255), "GAME OVER");
		ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), 24.0f, ImVec2(420.0f, 360.0f), IM_COL32(190, 175, 180, 255), "The expedition has ended.");
		ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), 22.0f, ImVec2(425.0f, 535.0f), IM_COL32(235, 225, 225, 255), "SPACE: Retry     ENTER: Title");
	}
#endif
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
	background_->Update();
	accent_->Update();
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
	ImGuiManager::GetInstance()->ImGuiDraw();

	dxCommon->PostDraw();
}
