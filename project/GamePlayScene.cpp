#include "GamePlayScene.h"
// シングルトン
#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "SkyBoxCommon.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ParticleManager.h"
#include "SrvManager.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "DirectXCommon.h"
#include "WinApp.h"
#include <algorithm>
#include <cmath>

namespace
{
	constexpr float kDeltaTime = 1.0f / 60.0f;
	constexpr float kDamageFlashDuration = 0.24f;
	constexpr float kEnemyAttackInterval = 1.2f;
	constexpr float kEnemyHitFlashDuration = 0.18f;
	constexpr float kEnemyHitFlashBlinkInterval = 0.045f;
	// 両モデルの幅(0.6)に余裕を少し足し、面同士が触れる手前で止める
	constexpr float kEnemyAttackRange = 0.65f;
	constexpr float kEnemySpawnDistance = 3.5f;
	constexpr float kDeathAnimationDuration = 0.9f;
	constexpr float kDeathFadeDuration = 0.7f;
	constexpr float kDeathFallAngle = 1.45f;

}

void GamePlayScene::Initialize()
{
	showSurvivalMessage_ = false;
	// 制作チャートの最初の段階として、次の戦闘用素材を集める作業パートから始める。
	phase_ = GamePhase::Work;
	equippedWeapon_ = WeaponType::None;
	isBossFight_ = false;
	gameClearTransitionRequested_ = false;
	enemyHealth_ = 0;
	enemyMaxHealth_ = 0;
	playerAttackCooldown_ = 0.0f;
	isCombatIntroActive_ = false;
	combatIntroStep_ = 0;
	playerHealth_ = kMaxPlayerHealth;
	enemyAttackCooldown_ = 0.0f;
	damageFlashTimer_ = 0.0f;
	deathAnimationTimer_ = 0.0f;
	deathStartRotationX_ = 0.0f;
	isPlayerDead_ = false;
	gameOverTransitionRequested_ = false;

	auto dxCommon = DirectXCommon::GetInstance();

	// --- カメラ ---
	camera = new Camera();
	// カメラの数値設定
	rotate = { 0.0f,0.0f,0.0f };
	translate = { 0.0f,0.0f,-10.0f };
	cameraOrbitTarget_ = { 0.0f, 0.0f, 10.0f };
	cameraOrbitYaw_ = 0.0f;
	cameraOrbitPitch_ = 0.0f;
	cameraOrbitDistance_ = 20.0f;
	camera->SetRotate(rotate);
	camera->SetTranslate(translate);
	Object3dCommon::GetInstance()->SetDefaultCamera(camera);
	SkyBoxCommon::GetInstance()->SetDefaultCamera(camera);
	
	// --- 描画オブジェクトパラメータ設定 ---
	// オブジェクト
	//	プレイヤー
	playerPosition = { 0.0f,TileMap::kTileSize,10.0f };
	playerIsInsideMap_ = tileMap_.WorldToTile(playerPosition, playerTileX_, playerTileY_);
	playerRotate = { 0.0f,0.0f,0.0f };
	velocityY = 0.0f;         // Y軸方向の現在の速度
	gravity = -0.025f;        // 重力（毎フレーム下に向かって引っ張る力）
	jumpPower = 0.3f;         // ジャンプ力（上に飛び上がる初速）
	// 初期装備は斧にする。
	currentTool_ = TileMap::ToolType::Axe;
	dirtCount_ = 0;
	stoneCount_ = 0;
	oreCount_ = 0;
	woodCount_ = 0;
	// 戦闘開始時の敵をプレイヤーの前方に配置する
	enemyPosition = {
		playerPosition.x + std::cos(playerRotate.y) * kEnemySpawnDistance,
		playerPosition.y,
		playerPosition.z - std::sin(playerRotate.y) * kEnemySpawnDistance
	};
	enemyRotate = { 0.0f,3.0f,0.0f };
	isEnemyAlive = true;
	enemyAttackCooldown_ = 0.8f;
	// isEnemyAlive = true; bool型だから宣言しなくていい わかりやすくするために置いてる
	//  スプライト
	spritePosition = { 0.0f,0.0f,0.0f };
	spriteRotation = 0.0f;
	spriteColor = { 1.0f,1.0f,1.0f,1.0f };
	spriteSize = { 1.0f,1.0f };

	// --- テクスチャ＆モデル読み込み ---
	spriteFile[0] = "resources/white.png";
	spriteFile[1] = "resources/white.png";
	for (int i = 0; i < spriteFile.size(); i++)
	{
		TextureManager::GetInstance()->LoadTexture(spriteFile[i]);
	}
	ModelManager::GetInstance()->LoadModel("GamePlayScene/character/fight/fight.obj");
	ModelManager::GetInstance()->LoadModel("GamePlayScene/character/survival/survival.obj");
	ModelManager::GetInstance()->LoadModel("GamePlayScene/character/spaceNext/spaceNext.obj");
	ModelManager::GetInstance()->LoadModel("GamePlayScene/character/spaceStart/spaceStart.obj");
	ModelManager::GetInstance()->LoadModel("GamePlayScene/part/workPart.obj");
	ModelManager::GetInstance()->LoadModel("GamePlayScene/part/fightPart.obj");
	ModelManager::GetInstance()->LoadModel("GamePlayScene/part/partchange.obj");
	ModelManager::GetInstance()->LoadModel("GamePlayScene/part/workNostart.obj");
	ModelManager::GetInstance()->LoadModel("block/block.obj");
	TextureManager::GetInstance()->LoadTexture("resources/skybox.dds");
	uint32_t skyboxTextureIndex = TextureManager::GetInstance()->GetTextureIndexByFilepath("resources/skybox.dds");

	// 作業・戦闘の両パートで共有するマップを初期化
	tileMap_.Initialize(skyboxTextureIndex);

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
	// アニメーション完成まで、当たり判定サイズが見えるブロックを仮モデルにする
	object3d->SetModel("block/block.obj");
	object3d->SetScale({ kPlayerCollisionRadius * 2.0f, kPlayerHeight, kPlayerCollisionRadius * 2.0f });
	object3d->SetTranslate({ playerPosition.x, playerPosition.y + kPlayerHeight * 0.5f, playerPosition.z });
	object3d->SetEnvironmentTextureIndex(skyboxTextureIndex); 
	//  敵
	enemy3d = new Object3d();
	enemy3d->Initialize();
	enemy3d->SetModel("block/block.obj");
	enemy3d->SetScale({ kPlayerCollisionRadius * 2.0f, kPlayerHeight, kPlayerCollisionRadius * 2.0f });
	enemy3d->SetColor({ 1.0f, 0.12f, 0.08f, 1.0f });
	enemy3d->SetEnvironmentTextureIndex(skyboxTextureIndex);

	// -----文字(案内)-----
	//   ---１ページ目---
	// Blenderで作成した戦闘開始メッセージ
	enemyApproachMessage_ = new Object3d();
	enemyApproachMessage_->Initialize();
	enemyApproachMessage_->SetModel("GamePlayScene/character/fight/fight.obj");
	enemyApproachMessage_->SetEnvironmentTextureIndex(skyboxTextureIndex);
	// Blenderモデルは原点を中心に作られているので、まず画面中央へ表示する
	enemyApproachMessage_->SetTranslate({ 0.0f, 1.0f, 0.0f });
	enemyApproachMessage_->SetScale({ 0.5f, 0.5f, 0.5f });
	// スペースの案内
	spaceNextMessage_ = new Object3d();
	spaceNextMessage_->Initialize();
	spaceNextMessage_->SetModel("GamePlayScene/character/spaceNext/spaceNext.obj");
	spaceNextMessage_->SetEnvironmentTextureIndex(skyboxTextureIndex);
	spaceNextMessage_->SetTranslate({ 0.0f, -1.0f, 0.0f });
	spaceNextMessage_->SetScale({ 0.5f, 0.5f, 0.5f });
	//   ---２ページ目---
	survivalGoalMessage_ = new Object3d();
	survivalGoalMessage_->Initialize();
	survivalGoalMessage_->SetModel("GamePlayScene/character/survival/survival.obj");
	survivalGoalMessage_->SetEnvironmentTextureIndex(skyboxTextureIndex);
	survivalGoalMessage_->SetTranslate({ 0.0f, 1.0f, 0.0f });
	survivalGoalMessage_->SetScale({ 0.5f, 0.5f, 0.5f });
	// スペースの案内
	spaceStartMessage_ = new Object3d();
	spaceStartMessage_->Initialize();
	spaceStartMessage_->SetModel("GamePlayScene/character/spaceStart/spaceStart.obj");
	spaceStartMessage_->SetEnvironmentTextureIndex(skyboxTextureIndex);
	spaceStartMessage_->SetTranslate({ 0.0f, -1.0f, 0.0f });
	spaceStartMessage_->SetScale({ 0.5f, 0.5f, 0.5f });

	// SkyBox
	skyBox = new SkyBox();
	skyBox->Initialize();
	// 読み込んだテクスチャの番号を SkyBox に教える
	skyBox->SetTextureIndex(skyboxTextureIndex);

	// --- パーティクル初期化 ---
	// パーティクル
	// 1. 画像の読み込みだけを行う（戻り値は受け取らない）
	TextureManager::GetInstance()->LoadTexture("Resources/circle2.png");
	// 2. これが「何枚目に読み込んだ画像か」で番号を直接決める
	// (例: 他に2枚読み込んでいて、これが3枚目の画像なら、0から数えて「2」になります)
	uint32_t particleTexIndex = TextureManager::GetInstance()->GetTextureIndexByFilepath("Resources/circle2.png"); // ★環境に合わせて 1 や 2 などに変えてみてください
	// 設定
	ParticleManager::GetInstance()->CreateGroup("magic", particleTexIndex);
	particleEffectTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,10.0f } };
	particleEmitterEffect = new ParticleEmitter("magic", particleEffectTransform, 1, 0.1f);

	// --- スプライト初期化 ---
	for (uint32_t i = 0; i < 5; ++i)
	{
		Sprite* sprite = new Sprite();
		sprite->Initialize(spriteFile[i % 2]);
		sprites_.push_back(sprite);
	}

	// 開始演出用の半透明な黒い幕
	startOverlay_ = new Sprite();
	startOverlay_->Initialize("resources/white.png");
	startOverlay_->SetPosition({ 0.0f, 0.0f, 0.0f });
	startOverlay_->SetSize({
		WinApp::kClientWidth / 100.0f,
		WinApp::kClientHeight / 100.0f
	});
	startOverlay_->SetColor({ 0.0f, 0.0f, 0.0f, 0.55f });
	startOverlay_->Update();

	// 被弾時の赤い画面フラッシュと、戦闘中のHPバー
	damageFlashOverlay_ = new Sprite();
	damageFlashOverlay_->Initialize("resources/white.png");
	damageFlashOverlay_->SetPosition({ 0.0f, 0.0f, 0.0f });
	damageFlashOverlay_->SetSize({ WinApp::kClientWidth / 100.0f, WinApp::kClientHeight / 100.0f });
	damageFlashOverlay_->SetColor({ 1.0f, 0.0f, 0.0f, 0.0f });
	damageFlashOverlay_->Update();

	// 倒れた後に画面全体を暗くしてからゲームオーバーへ遷移する
	deathFadeOverlay_ = new Sprite();
	deathFadeOverlay_->Initialize("resources/white.png");
	deathFadeOverlay_->SetPosition({ 0.0f, 0.0f, 0.0f });
	deathFadeOverlay_->SetSize({ WinApp::kClientWidth / 100.0f, WinApp::kClientHeight / 100.0f });
	deathFadeOverlay_->SetColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	deathFadeOverlay_->Update();

	healthBarBackground_ = new Sprite();
	healthBarBackground_->Initialize("resources/white.png");
	healthBarBackground_->SetPosition({ 32.0f, 28.0f, 0.0f });
	healthBarBackground_->SetSize({ 3.16f, 0.28f });
	healthBarBackground_->SetColor({ 0.08f, 0.08f, 0.1f, 0.9f });
	healthBarBackground_->Update();

	healthBarFill_ = new Sprite();
	healthBarFill_->Initialize("resources/white.png");
	healthBarFill_->SetPosition({ 36.0f, 32.0f, 0.0f });
	healthBarFill_->SetColor({ 0.85f, 0.06f, 0.08f, 1.0f });
	UpdateHealthBar();
}

void GamePlayScene::Finalize()
{
	tileMap_.Finalize();
	for (Sprite* sprite : sprites_)
	{
		delete sprite;
	}
	sprites_.clear();
	delete startOverlay_;
	startOverlay_ = nullptr;
	delete damageFlashOverlay_;
	damageFlashOverlay_ = nullptr;
	delete deathFadeOverlay_;
	deathFadeOverlay_ = nullptr;
	delete healthBarBackground_;
	healthBarBackground_ = nullptr;
	delete healthBarFill_;
	healthBarFill_ = nullptr;
	delete object3d;
	delete enemy3d;
	delete enemyApproachMessage_;
	delete survivalGoalMessage_;
	delete spaceNextMessage_;
	delete spaceStartMessage_;
	delete skyBox;
	delete camera;
	delete lineRenderer;
	// エフェクト
	delete particleEmitterEffect;

	delete postEffect;
}

void GamePlayScene::Update()
{
	auto input = Input::GetInstance();
	auto imGuiManager = ImGuiManager::GetInstance();

	// ImGuiの設定 始
	imGuiManager->ImGuiBegin();

	// キー入力 始
	input->Update();
	if (isPlayerDead_)
	{
		UpdateDamageFlash();
		UpdateDeathAnimation();
		DrawObjectiveNavigation();
		DrawControlGuide();
		imGuiManager->ImGuiEnd();
		return;
	}
	UpdateMouseCamera();
	// 展示用に、Enterによる作業・戦闘パート切り替えは一時停止
	if (isCombatIntroActive_)
	{
		UpdateCombatIntro();
		UpdateDamageFlash();
		// 案内中はゲームプレイ側の更新を止め、表示に必要な行列だけ更新する
		camera->SetRotate(rotate);
		camera->SetTranslate(translate);
		camera->Update();
		tileMap_.Update();
		object3d->SetRotate(playerRotate);
		object3d->SetTranslate({ playerPosition.x, playerPosition.y + kPlayerHeight * 0.5f, playerPosition.z });
		object3d->Update();
		DrawObjectiveNavigation();
		DrawControlGuide();
		imGuiManager->ImGuiEnd();
		return;
	}

	// 移動や重力など、作業・戦闘の両方に共通する処理を先に進める
	UpdateSharedGameplay();
	// パート固有の入力とルールは、現在のフェーズに応じて分けて処理する
	if (phase_ == GamePhase::Work)
	{
		UpdateWork();
	}
	else
	{
		UpdateCombat();
	}
	UpdateDamageFlash();
	UpdateHealthBar();

#ifdef _DEBUG
	DrawGameplayHud();
#endif
#ifdef USE_IMGUI
	DrawObjectiveNavigation();
	DrawControlGuide();
	DrawHotbar();
#endif
	imGuiManager->ImGuiEnd();
}

// Debugビルドでだけ、追加のゲーム状態とデバッグ値を表示する。
void GamePlayScene::DrawGameplayHud()
{
#ifdef USE_IMGUI
	const char* toolName = "シャベル";
	if (currentTool_ == TileMap::ToolType::Pickaxe) toolName = "つるはし";
	if (currentTool_ == TileMap::ToolType::Axe) toolName = "斧";

	// 操作方法と目標は、フェーズに応じて必要なものだけを常時表示する。
	ImGui::SetNextWindowPos(ImVec2(18.0f, 188.0f), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.82f);
	const ImGuiWindowFlags guideWindowFlags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
	if (ImGui::Begin("ゲーム状況と操作方法", nullptr, guideWindowFlags))
	{
		ImGui::TextUnformatted(phase_ == GamePhase::Work ? "作業パート" : "戦闘パート");
		if (phase_ == GamePhase::Work)
		{
			if (!isBossFight_)
			{
				ImGui::Text("目標: 木材を%d個集めて木の剣を作る", kWorkGoalWoodCount);
				ImGui::Text("木材: %d / %d%s", woodCount_, kWorkGoalWoodCount,
					IsWorkGoalComplete() ? " 目標達成" : "");
			}
			else
			{
				ImGui::Text("目標: 鉱石2個で鉱石の剣を作り、ボスを倒す");
			}
			ImGui::Text("道具: %s", toolName);
		}
		else
		{
			ImGui::Text("敵HP: %d / %d%s", enemyHealth_, enemyMaxHealth_, isBossFight_ ? "  ボス" : "");
			ImGui::Text("プレイヤーHP: %d / %d", playerHealth_, kMaxPlayerHealth);
		}

		if (ImGui::CollapsingHeader("素材の所持数"))
		{
			ImGui::Text("木材 %d / 鉱石 %d / 石 %d / 土 %d", woodCount_, oreCount_, stoneCount_, dirtCount_);
		}
		if (ImGui::CollapsingHeader("デバッグ情報"))
		{
			ImGui::Text("Player Tile: (%d, %d) %s", playerTileX_, playerTileY_, playerIsInsideMap_ ? "Inside" : "Outside");
			ImGui::DragFloat3("cameraRotato", &rotate.x, 0.1f);
			ImGui::DragFloat3("cameraTranslate", &translate.x, 0.1f);
			ImGui::DragFloat3("ObjectPosition", &playerPosition.x, 0.1f);
			ImGui::DragFloat3("ObjectRotation", &playerRotate.x, 0.1f);
			float environmentRate = object3d->GetEnvironmentCoefficient();
			if (ImGui::SliderFloat("Environment Rate", &environmentRate, 0.0f, 1.0f))
			{
				object3d->SetEnvironmentCoefficient(environmentRate);
			}
		}
	}
	ImGui::End();
#endif
}

// 進行状況に合わせて、次の目標と操作を左上に表示する。
void GamePlayScene::DrawObjectiveNavigation()
{
#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(18.0f, 18.0f), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.86f);
	const ImGuiWindowFlags navigationFlags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
	if (ImGui::Begin("##目標ナビ", nullptr, navigationFlags))
	{
		ImGui::TextUnformatted("ナビゲーション");
		ImGui::Separator();

		if (isPlayerDead_)
		{
			ImGui::TextUnformatted("体力がなくなりました");
			ImGui::TextUnformatted("ゲームオーバーへ移動中...");
		}
		else if (isCombatIntroActive_)
		{
			ImGui::TextUnformatted("戦闘の案内を確認しよう");
			ImGui::TextUnformatted("Spaceキーで次へ進む");
		}
		else if (phase_ == GamePhase::Work && !isBossFight_)
		{
			ImGui::TextUnformatted("通常戦の準備");
			if (equippedWeapon_ == WeaponType::None && !IsWorkGoalComplete())
			{
				ImGui::Text("木材を%d個集めよう", kWorkGoalWoodCount);
				ImGui::ProgressBar(
					static_cast<float>(woodCount_) / kWorkGoalWoodCount,
					ImVec2(250.0f, 0.0f));
				ImGui::Text("木材: %d / %d", woodCount_, kWorkGoalWoodCount);
			}
			else if (equippedWeapon_ == WeaponType::None)
			{
				ImGui::TextUnformatted("木の剣を作ろう");
				ImGui::TextUnformatted("Cキーで作成");
			}
			else
			{
				ImGui::TextUnformatted("通常戦を始めよう");
				ImGui::TextUnformatted("Enterキーで開始");
			}
		}
		else if (phase_ == GamePhase::Work)
		{
			ImGui::TextUnformatted("ボス戦の準備");
			if (equippedWeapon_ == WeaponType::None && oreCount_ < 2)
			{
				ImGui::TextUnformatted("鉱石を2個集めよう");
				ImGui::ProgressBar(static_cast<float>(oreCount_) / 2.0f, ImVec2(250.0f, 0.0f));
				ImGui::Text("鉱石: %d / 2", oreCount_);
			}
			else if (equippedWeapon_ == WeaponType::None)
			{
				ImGui::TextUnformatted("鉱石の剣を作ろう");
				ImGui::TextUnformatted("Cキーで作成");
			}
			else
			{
				ImGui::TextUnformatted("ボス戦を始めよう");
				ImGui::TextUnformatted("Enterキーで開始");
			}
		}
		else
		{
			ImGui::TextUnformatted(isBossFight_ ? "ボスを倒そう" : "通常敵を倒そう");
			ImGui::Text("敵HP: %d / %d", enemyHealth_, enemyMaxHealth_);
			ImGui::TextUnformatted("近づいてJキーまたは左クリックで攻撃");
		}
	}
	ImGui::End();
#endif
}

// 現在のフェーズで使える操作を、製品版でも常に見えるガイドにまとめる。
void GamePlayScene::DrawControlGuide()
{
#ifdef USE_IMGUI
	const float guideWidth = 360.0f;
	ImGui::SetNextWindowPos(ImVec2(
		WinApp::kClientWidth - guideWidth - 18.0f, 18.0f), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.82f);
	const ImGuiWindowFlags guideFlags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
	if (ImGui::Begin("##操作ガイド", nullptr, guideFlags))
	{
		ImGui::TextUnformatted(phase_ == GamePhase::Work ? "操作方法 / 作業パート" : "操作方法 / 戦闘パート");
		ImGui::Separator();
		ImGui::TextUnformatted("WASD: カメラ方向へ移動 / Space: ジャンプ");
		ImGui::TextUnformatted("右クリック+マウス: 視点操作");
		if (phase_ == GamePhase::Work)
		{
			ImGui::TextUnformatted("左クリック: 正面を採掘");
			ImGui::TextUnformatted("E: 上を採掘 / Q: 足元を採掘");
			if (equippedWeapon_ == WeaponType::None)
			{
				ImGui::TextUnformatted("C: 素材から武器を作成");
			}
			else
			{
				ImGui::TextUnformatted("Enter: 戦闘開始");
			}
		}
		else
		{
			ImGui::TextUnformatted("J または左クリック: 武器で攻撃");
		}
	}
	ImGui::End();
#endif
}

// Debug/Release共通のImGui描画で、道具・武器・素材数を9枠に表示する。
void GamePlayScene::DrawHotbar()
{
#ifdef USE_IMGUI
	const char* toolName = "シャベル";
	if (currentTool_ == TileMap::ToolType::Pickaxe) toolName = "つるはし";
	if (currentTool_ == TileMap::ToolType::Axe) toolName = "斧";

	// スロット順: 道具、木の剣、鉱石の剣、木材、鉱石、土、石、空き、空き。
	const char* slotNames[] = { toolName, "木の剣", "鉱石剣", "木材", "鉱石", "土", "石", "空き", "空き" };
	const int slotCounts[] = { 1,
		equippedWeapon_ == WeaponType::WoodSword ? 1 : 0,
		equippedWeapon_ == WeaponType::OreSword ? 1 : 0,
		woodCount_, oreCount_, dirtCount_, stoneCount_, 0, 0 };
	const int selectedSlot = equippedWeapon_ == WeaponType::WoodSword ? 1
		: equippedWeapon_ == WeaponType::OreSword ? 2 : 0;
	constexpr float kHotbarSlotWidth = 78.0f;
	constexpr float kHotbarSlotHeight = 64.0f;
	constexpr float kHotbarWindowWidth = 790.0f;
	constexpr float kHotbarWindowHeight = 96.0f;

	// 画面下中央にMinecraft/Terraria風の9枠ホットバーを固定表示する。
	ImGui::SetNextWindowPos(ImVec2(WinApp::kClientWidth * 0.5f,
		WinApp::kClientHeight - kHotbarWindowHeight - 16.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(kHotbarWindowWidth, kHotbarWindowHeight), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.86f);
	const ImGuiWindowFlags hotbarWindowFlags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNavFocus;
	if (ImGui::Begin("##所持品スロット", nullptr, hotbarWindowFlags))
	{
		for (int slotIndex = 0; slotIndex < 9; ++slotIndex)
		{
			// 選択中のスロットだけ背景色を明るくし、装備中の道具・武器を示す。
			const bool isSelected = slotIndex == selectedSlot;
			const ImVec4 slotColor = isSelected
				? ImVec4(0.55f, 0.40f, 0.16f, 1.0f)
				: ImVec4(0.12f, 0.14f, 0.18f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_ChildBg, slotColor);
			const char* slotId = "##HotbarSlot";
			ImGui::PushID(slotIndex);
			if (ImGui::BeginChild(slotId, ImVec2(kHotbarSlotWidth, kHotbarSlotHeight),
				ImGuiChildFlags_Borders | ImGuiChildFlags_FrameStyle,
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				ImGui::Text("%d", slotIndex + 1);
				ImGui::Text("%s", slotNames[slotIndex]);
				if (slotCounts[slotIndex] > 0)
				{
					ImGui::Text("x%d", slotCounts[slotIndex]);
				}
			}
			ImGui::EndChild();
			ImGui::PopID();
			ImGui::PopStyleColor();
			if (slotIndex < 8)
			{
				ImGui::SameLine();
			}
		}
	}
	ImGui::End();
#endif
}

void GamePlayScene::UpdateCombatIntro()
{
	UpdateCombatIntroLayout();

	if (Input::GetInstance()->WasKeyPressed(DIK_SPACE))
	{
		if (showSurvivalMessage_ && combatIntroStep_ == 0)
		{
			combatIntroStep_ = 1;
		}
		else
		{
			isCombatIntroActive_ = false;
		}
	}
}

void GamePlayScene::UpdateCombatIntroLayout()
{
	// 案内文字をカメラの正面に置き、マウスで視点を回しても画面内に保つ
	const float sinYaw = std::sin(cameraOrbitYaw_);
	const float cosYaw = std::cos(cameraOrbitYaw_);
	const float sinPitch = std::sin(cameraOrbitPitch_);
	const float cosPitch = std::cos(cameraOrbitPitch_);
	const Vector3 cameraForward = {
		-sinYaw * cosPitch,
		-sinPitch,
		 cosYaw * cosPitch
	};
	const Vector3 cameraUp = {
		-sinYaw * sinPitch,
		 cosPitch,
		 cosYaw * sinPitch
	};
	constexpr float kMessageDistance = 12.0f;
	const Vector3 messageCenter = {
		translate.x + cameraForward.x * kMessageDistance,
		translate.y + cameraForward.y * kMessageDistance,
		translate.z + cameraForward.z * kMessageDistance
	};
	const Vector3 cameraFacingRotation = { cameraOrbitPitch_, -cameraOrbitYaw_, 0.0f };
	constexpr float kMessageVerticalOffset = 1.0f;

	const Vector3 headingPosition = {
		messageCenter.x + cameraUp.x * kMessageVerticalOffset,
		messageCenter.y + cameraUp.y * kMessageVerticalOffset,
		messageCenter.z + cameraUp.z * kMessageVerticalOffset
	};
	const Vector3 promptPosition = {
		messageCenter.x - cameraUp.x * kMessageVerticalOffset,
		messageCenter.y - cameraUp.y * kMessageVerticalOffset,
		messageCenter.z - cameraUp.z * kMessageVerticalOffset
	};

	enemyApproachMessage_->SetTranslate(headingPosition);
	enemyApproachMessage_->SetRotate(cameraFacingRotation);
	survivalGoalMessage_->SetTranslate(headingPosition);
	survivalGoalMessage_->SetRotate(cameraFacingRotation);
	spaceNextMessage_->SetTranslate(promptPosition);
	spaceNextMessage_->SetRotate(cameraFacingRotation);
	spaceStartMessage_->SetTranslate(promptPosition);
	spaceStartMessage_->SetRotate(cameraFacingRotation);
}

void GamePlayScene::UpdateMouseCamera()
{
	auto input = Input::GetInstance();
	if (input->IsMouseButtonDown(VK_RBUTTON))
	{
		const POINT mouseDelta = input->GetMouseDelta();
		constexpr float kMouseSensitivity = 0.005f;
		cameraOrbitYaw_ += static_cast<float>(mouseDelta.x) * kMouseSensitivity;
		cameraOrbitPitch_ += static_cast<float>(mouseDelta.y) * kMouseSensitivity;

		// カメラが真上・真下を向いて操作が反転しないように制限する
		constexpr float kPitchLimit = 1.35f;
		if (cameraOrbitPitch_ > kPitchLimit) cameraOrbitPitch_ = kPitchLimit;
		if (cameraOrbitPitch_ < -kPitchLimit) cameraOrbitPitch_ = -kPitchLimit;
	}

	// カメラの注視高さをプレイヤーの中心に合わせ、掘り進んだ高さにも追従させる
	cameraOrbitTarget_.y = playerPosition.y + kPlayerHeight * 0.5f;

	const float cosPitch = std::cos(cameraOrbitPitch_);
	translate = {
		cameraOrbitTarget_.x + std::sin(cameraOrbitYaw_) * cosPitch * cameraOrbitDistance_,
		cameraOrbitTarget_.y + std::sin(cameraOrbitPitch_) * cameraOrbitDistance_,
		cameraOrbitTarget_.z - std::cos(cameraOrbitYaw_) * cosPitch * cameraOrbitDistance_
	};
	rotate = { cameraOrbitPitch_, -cameraOrbitYaw_, 0.0f };
}

void GamePlayScene::UpdateWork()
{
	auto input = Input::GetInstance();
	// 必要素材を消費して武器を作り、Enter で次の戦闘へ進む。
	if (input->WasKeyPressed(DIK_C))
	{
		CraftWeapon();
	}
	if (input->WasKeyPressed(DIK_RETURN) && equippedWeapon_ != WeaponType::None)
	{
		StartCombat();
	}
	// 左クリックで正面、Eで正面の一段上、Qで足元を採掘する。
	if (input->WasMouseButtonPressed(VK_LBUTTON))
	{
		TryMineFront();
	}
	if (input->WasKeyPressed(DIK_E))
	{
		TryMineFront(1);
	}
	if (input->WasKeyPressed(DIK_Q))
	{
		// 足元のブロックを採掘する
		TryMineBelow();
	}
}

// 作業目標の達成状況を判定する。後続の武器作成・パート進行からも再利用できる。
bool GamePlayScene::IsWorkGoalComplete() const
{
	return woodCount_ >= kWorkGoalWoodCount;
}

void GamePlayScene::TryMineFront(int verticalOffset)
{
	if (!tileMap_.WorldToTile(playerPosition, playerTileX_, playerTileY_))
	{
		return;
	}

	const int directionX = static_cast<int>(std::round(std::sin(playerRotate.y)));
	const int directionY = static_cast<int>(std::round(std::cos(playerRotate.y)));
	const int targetLayer = static_cast<int>(std::floor(playerPosition.y / TileMap::kTileSize)) + verticalOffset;
	TryMineTile(playerTileX_ + directionX, playerTileY_ + directionY, targetLayer);
}

void GamePlayScene::TryMineBelow()
{
	if (!tileMap_.WorldToTile(playerPosition, playerTileX_, playerTileY_))
	{
		return;
	}

	const int targetLayer = static_cast<int>(std::ceil(playerPosition.y / TileMap::kTileSize)) - 1;
	TryMineTile(playerTileX_, playerTileY_, targetLayer);
}

void GamePlayScene::TryMineTile(int tileX, int tileY, int layer)
{
	TileMap::TileType minedType = TileMap::TileType::Air;
	if (!tileMap_.MineBlock(tileX, tileY, layer, currentTool_, minedType))
	{
		return;
	}

	// 採掘に成功した素材だけ、対応する所持数へ加算する
	switch (minedType)
	{
	case TileMap::TileType::Dirt:
		++dirtCount_;
		break;
	case TileMap::TileType::Stone:
		++stoneCount_;
		break;
	case TileMap::TileType::Ore:
		++oreCount_;
		break;
	case TileMap::TileType::Wood:
		++woodCount_;
		break;
	default:
		break;
	}
}

void GamePlayScene::UpdateCombat()
{
	if (!isEnemyAlive || isPlayerDead_)
	{
		return;
	}

	enemyAttackCooldown_ = (std::max)(0.0f, enemyAttackCooldown_ - kDeltaTime);
	playerAttackCooldown_ = (std::max)(0.0f, playerAttackCooldown_ - kDeltaTime);
	// J または左クリックで近くの敵を攻撃する。
	const auto input = Input::GetInstance();
	if (input->WasKeyPressed(DIK_J) || input->WasMouseButtonPressed(VK_LBUTTON))
	{
		TryPlayerAttack();
	}
	if (phase_ != GamePhase::Combat || !isEnemyAlive)
	{
		return;
	}
	const float dx = playerPosition.x - enemyPosition.x;
	const float dz = playerPosition.z - enemyPosition.z;
	const float horizontalDistance = std::sqrt(dx * dx + dz * dz);

	if (horizontalDistance <= kEnemyAttackRange && enemyAttackCooldown_ <= 0.0f)
	{
		TakePlayerDamage();
		enemyAttackCooldown_ = kEnemyAttackInterval;
	}
}

// 素材を消費して武器を作成する。初戦は木材、ボス戦前は報酬の鉱石を使う。
void GamePlayScene::CraftWeapon()
{
	if (isBossFight_)
	{
		if (equippedWeapon_ == WeaponType::None && oreCount_ >= 2)
		{
			oreCount_ -= 2;
			equippedWeapon_ = WeaponType::OreSword;
		}
		return;
	}

	if (equippedWeapon_ == WeaponType::None && IsWorkGoalComplete())
	{
		woodCount_ -= kWorkGoalWoodCount;
		equippedWeapon_ = WeaponType::WoodSword;
	}
}

// 戦闘開始時に敵の強さと配置を現在の進行段階に合わせる。
void GamePlayScene::StartCombat()
{
	phase_ = GamePhase::Combat;
	isEnemyAlive = true;
	enemyMaxHealth_ = isBossFight_ ? 36 : 21;
	enemyHealth_ = enemyMaxHealth_;
	enemyAttackCooldown_ = isBossFight_ ? 1.8f : 1.0f;
	playerAttackCooldown_ = 0.0f;
	playerHealth_ = kMaxPlayerHealth;
	enemyPosition = {
		playerPosition.x + std::cos(playerRotate.y) * kEnemySpawnDistance,
		playerPosition.y,
		playerPosition.z - std::sin(playerRotate.y) * kEnemySpawnDistance
	};
	enemy3d->SetColor(isBossFight_
		? Vector4{ 0.55f, 0.08f, 0.75f, 1.0f }
		: Vector4{ 1.0f, 0.12f, 0.08f, 1.0f });
}

// 攻撃距離内の敵に武器の威力を適用し、HP が0になったら勝利処理へ進む。
void GamePlayScene::TryPlayerAttack()
{
	if (!isEnemyAlive || playerAttackCooldown_ > 0.0f || equippedWeapon_ == WeaponType::None)
	{
		return;
	}

	const float dx = playerPosition.x - enemyPosition.x;
	const float dz = playerPosition.z - enemyPosition.z;
	const float attackRange = 1.6f;
	if (dx * dx + dz * dz > attackRange * attackRange)
	{
		return;
	}

	const int damage = equippedWeapon_ == WeaponType::WoodSword ? 7 : 12;
	enemyHealth_ = (std::max)(0, enemyHealth_ - damage);
	playerAttackCooldown_ = 0.45f;

	// 命中時はエフェクトを出さず、敵の白点滅だけで攻撃成功を知らせる。
	enemyHitFlashTimer_ = kEnemyHitFlashDuration;

	if (enemyHealth_ == 0)
	{
		isEnemyAlive = false;
		HandleEnemyDefeated();
	}
}

// 通常敵の勝利で鉱石と回復を得てボス準備へ、ボス勝利でクリア画面へ進む。
void GamePlayScene::HandleEnemyDefeated()
{
	if (isBossFight_)
	{
		if (!gameClearTransitionRequested_)
		{
			gameClearTransitionRequested_ = true;
			sceneManager_->StartTransition(SceneType::GameClear);
		}
		return;
	}

	isBossFight_ = true;
	phase_ = GamePhase::Work;
	equippedWeapon_ = WeaponType::None;
	oreCount_ += 2;
	playerHealth_ = kMaxPlayerHealth;
	playerPosition = { 0.0f, TileMap::kTileSize, 10.0f };
	playerTileX_ = 0;
	playerTileY_ = 0;
	playerIsInsideMap_ = tileMap_.WorldToTile(playerPosition, playerTileX_, playerTileY_);
}

void GamePlayScene::TakePlayerDamage()
{
	if (isPlayerDead_)
	{
		return;
	}

	playerHealth_ = (std::max)(0, playerHealth_ - 1);
	damageFlashTimer_ = kDamageFlashDuration;
	damageFlashOverlay_->SetColor({ 1.0f, 0.02f, 0.02f, 0.22f });

	if (playerHealth_ == 0)
	{
		isPlayerDead_ = true;
		deathAnimationTimer_ = 0.0f;
		deathStartRotationX_ = playerRotate.x;
		velocityY = 0.0f;
		isJumping = false;
	}
}

void GamePlayScene::UpdateDeathAnimation()
{
	deathAnimationTimer_ += kDeltaTime;
	const float progress = (std::min)(deathAnimationTimer_ / kDeathAnimationDuration, 1.0f);
	const float easedProgress = progress * progress * (3.0f - 2.0f * progress);

	// プレイヤーの上半身を後ろへ倒し、体の中心も地面の高さまで下げる
	playerRotate.x = deathStartRotationX_ - kDeathFallAngle * easedProgress;
	const float centerHeight = kPlayerHeight * 0.5f +
		(kPlayerCollisionRadius - kPlayerHeight * 0.5f) * easedProgress;
	object3d->SetRotate(playerRotate);
	object3d->SetTranslate({ playerPosition.x, playerPosition.y + centerHeight, playerPosition.z });
	object3d->Update();
	camera->Update();
	tileMap_.Update();

	if (progress >= 1.0f)
	{
		const float fadeProgress = (std::min)(
			(deathAnimationTimer_ - kDeathAnimationDuration) / kDeathFadeDuration, 1.0f);
		deathFadeOverlay_->SetColor({ 0.0f, 0.0f, 0.0f, fadeProgress });
		deathFadeOverlay_->Update();

		if (fadeProgress >= 1.0f && !gameOverTransitionRequested_)
		{
			gameOverTransitionRequested_ = true;
			sceneManager_->StartTransition(SceneType::GameOver);
		}
	}
}

void GamePlayScene::UpdateDamageFlash()
{
	damageFlashTimer_ = (std::max)(0.0f, damageFlashTimer_ - kDeltaTime);
	enemyHitFlashTimer_ = (std::max)(0.0f, enemyHitFlashTimer_ - kDeltaTime);
	const float flashProgress = damageFlashTimer_ / kDamageFlashDuration;
	const float overlayAlpha = 0.22f * flashProgress;
	damageFlashOverlay_->SetColor({ 1.0f, 0.02f, 0.02f, overlayAlpha });
	object3d->SetColor(flashProgress > 0.0f
		? Vector4{ 1.0f, 0.08f, 0.08f, 1.0f }
		: Vector4{ 1.0f, 1.0f, 1.0f, 1.0f });
	damageFlashOverlay_->Update();
}

void GamePlayScene::UpdateHealthBar()
{
	const float healthRatio = static_cast<float>(playerHealth_) / kMaxPlayerHealth;
	healthBarFill_->SetPosition({ 36.0f, 32.0f, 0.0f });
	healthBarFill_->SetSize({ 2.96f * healthRatio, 0.20f });
	healthBarFill_->Update();
}

void GamePlayScene::UpdateSharedGameplay()
{
	auto input = Input::GetInstance();

	// WASD入力をまずカメラから見た左右・前後方向で受け取り、後でワールド方向へ変換する。
	float localMoveRight = 0.0f;
	float localMoveForward = 0.0f;
	Vector3 playerMove = { 0.0f, 0.0f, 0.0f };

	// 開発用入力だけ作業パートに限定する。
	if (phase_ == GamePhase::Work)
	{
		if (input->IsKeyDown(DIK_0))
		{
			OutputDebugStringA("Hit 0\n");
		}
		if (input->WasKeyPressed(DIK_1))
		{
			particleEmitterEffect->EmitEffectNow();
		}
	}

	// 戦闘中もWASDで移動し、Spaceでジャンプして敵との距離を調整できる。
	if (input->IsKeyDown(DIK_D)) localMoveRight += 1.0f;
	if (input->IsKeyDown(DIK_A)) localMoveRight -= 1.0f;
	if (input->IsKeyDown(DIK_W)) localMoveForward += 1.0f;
	if (input->IsKeyDown(DIK_S)) localMoveForward -= 1.0f;
	if (input->WasKeyPressed(DIK_SPACE) && !isJumping)
	{
		velocityY = jumpPower; // 上方向への初速を与える
		isJumping = true;      // ジャンプ状態をオンにする
	}

	// 水平方向のカメラ回転に合わせてWASDを回し、カメラ正面を前進方向にする。
	const float sinCameraYaw = std::sin(cameraOrbitYaw_);
	const float cosCameraYaw = std::cos(cameraOrbitYaw_);
	playerMove.x = localMoveRight * cosCameraYaw - localMoveForward * sinCameraYaw;
	playerMove.z = localMoveRight * sinCameraYaw + localMoveForward * cosCameraYaw;

	if (playerMove.x != 0.0f || playerMove.z != 0.0f)
	{
		float playerSpeed = 0.1f;

		// 斜め移動時に移動速度が速くならないよう正規化
		float length = std::sqrt(playerMove.x * playerMove.x + playerMove.z * playerMove.z);
		playerMove.x /= length;
		playerMove.z /= length;

		// 軸ごとに衝突判定することで、壁に当たっても壁沿いの移動を続けられる
		Vector3 nextPosition = playerPosition;
		nextPosition.x += playerMove.x * playerSpeed;
		if (tileMap_.CanOccupyWorldPosition(nextPosition, kPlayerCollisionRadius, kPlayerHeight))
		{
			playerPosition.x = nextPosition.x;
		}

		nextPosition = playerPosition;
		nextPosition.z += playerMove.z * playerSpeed;
		if (tileMap_.CanOccupyWorldPosition(nextPosition, kPlayerCollisionRadius, kPlayerHeight))
		{
			playerPosition.z = nextPosition.z;
		}

		// 2. 移動方向を向くように回転（敵の処理と同じatan2を使用）
		playerRotate.y = std::atan2(playerMove.x, playerMove.z);
	}
	playerIsInsideMap_ = tileMap_.WorldToTile(playerPosition, playerTileX_, playerTileY_);

	// 接地中も毎フレーム重力を適用する。
	// 足元のブロックを採掘して消えた場合、次の層まで落下できる。
	const float previousFeetY = playerPosition.y;
	velocityY += gravity;
	const float nextFeetY = playerPosition.y + velocityY;
	float landingY = 0.0f;

	if (velocityY <= 0.0f && tileMap_.FindLandingSurface(
		playerPosition, kPlayerCollisionRadius, previousFeetY, nextFeetY, landingY))
	{
		playerPosition.y = landingY;
		velocityY = 0.0f;
		isJumping = false;
	}
	else
	{
		Vector3 nextPosition = playerPosition;
		nextPosition.y = nextFeetY;
		if (tileMap_.CanOccupyWorldPosition(nextPosition, kPlayerCollisionRadius, kPlayerHeight))
		{
			playerPosition.y = nextFeetY;
		}
		else
		{
			// 上にブロックがある場合は頭が当たったところで上昇を止める
			velocityY = 0.0f;
		}
		isJumping = true;
	}

	float pos = 0.0f;
	camera->Update();
	object3d->SetRotate(playerRotate);
	object3d->SetTranslate({ playerPosition.x, playerPosition.y + kPlayerHeight * 0.5f, playerPosition.z });
	object3d->Update();
	Vector3 diff =
	{
		playerPosition.x - enemyPosition.x,
		playerPosition.y - enemyPosition.y,
		playerPosition.z - enemyPosition.z
	};

	// 敵は攻撃距離に入ったら水平方向の移動を止め、プレイヤーと重ならないようにする
	constexpr float enemySpeed = 0.05f;
	const float horizontalDistance = std::sqrt(diff.x * diff.x + diff.z * diff.z);
	if (phase_ == GamePhase::Combat && isEnemyAlive && !isPlayerDead_ &&
		horizontalDistance > kEnemyAttackRange)
	{
		const float directionX = diff.x / horizontalDistance;
		const float directionZ = diff.z / horizontalDistance;
		enemyPosition.x += directionX * enemySpeed;
		enemyPosition.z += directionZ * enemySpeed;
		enemyRotate.y = std::atan2(directionX, directionZ);
	}
	// プレイヤーがジャンプ・落下した場合は、敵の高さだけ追従させる
	if (phase_ == GamePhase::Combat && isEnemyAlive && !isPlayerDead_ && std::abs(diff.y) > 0.05f)
	{
		const float verticalStep = std::min(std::abs(diff.y), enemySpeed);
		enemyPosition.y += diff.y > 0.0f ? verticalStep : -verticalStep;
	}
	if (isEnemyAlive)
	{
		// 命中直後は短い間隔で白く点滅させ、通常敵／ボスの色は点滅後に戻す。
		const Vector4 enemyBaseColor = isBossFight_
			? Vector4{ 0.55f, 0.08f, 0.75f, 1.0f }
			: Vector4{ 1.0f, 0.12f, 0.08f, 1.0f };
		const bool showEnemyHitFlash = enemyHitFlashTimer_ > 0.0f &&
			static_cast<int>(enemyHitFlashTimer_ / kEnemyHitFlashBlinkInterval) % 2 == 0;
		enemy3d->SetColor(showEnemyHitFlash
			? Vector4{ 2.0f, 2.0f, 2.0f, 1.0f }
			: enemyBaseColor);
		enemy3d->SetRotate(enemyRotate);
		enemy3d->SetTranslate({ enemyPosition.x, enemyPosition.y + kPlayerHeight * 0.5f, enemyPosition.z });
		enemy3d->Update();
	}

	object3d->DrawSkeleton(lineRenderer);
	if (isEnemyAlive)
	{
		enemy3d->DrawSkeleton(lineRenderer);
	}

	// 3. 更新
	skyBox->Update();

	// ★ここに追加：パーティクルの更新
	ParticleManager::GetInstance()->Update();	   // 発生した全パーティクルの移動と寿命チェック

	for (Sprite* sprite : sprites_)
	{
		Vector3 changePos = { pos,0.0f,0.0f };
		pos += 200.0f;
		sprite->SetRotation(spriteRotation);
		sprite->SetSize(spriteSize);
		sprite->SetPosition(Add(spritePosition, changePos));
		sprite->SetColor(spriteColor);
		sprite->Update();
	}

	camera->SetRotate(rotate);
	camera->SetTranslate(translate);
	camera->Update();
	tileMap_.Update();
}

void GamePlayScene::Draw()
{
	auto dxCommon = DirectXCommon::GetInstance();
	auto srvManager = SrvManager::GetInstance();
	auto imGuiManager = ImGuiManager::GetInstance();

	postEffect->PreDraw();

	srvManager->PreDraw();

	Object3dCommon::GetInstance()->DrawCommon();
	tileMap_.Draw();
	object3d->Draw();
	if (phase_ == GamePhase::Work)
	{
		DrawWork();
	}
	else
	{
		DrawCombat();
	}

	// 4. 描画
	SkyBoxCommon::GetInstance()->DrawCommon(); // Skybox用のルートシグネチャ・PSOに切り替え
	//skyBox->Draw();             // 引数なしでスッキリ呼び出せます！
	//lineRenderer->Draw(camera);
	// 戦闘のヒット表現は敵の点滅で行い、パーティクル描画はここでは行わない。
	// ParticleManager::GetInstance()->Draw(camera);

	SpriteCommon::GetInstance()->DrawCommon();

	for (Sprite* sprite : sprites_)
	{
		//sprite->Draw();
	}

	postEffect->PostDraw();

	DirectXCommon::GetInstance()->PreDraw();

	postEffect->Draw();
	// スプライト描画の設定をしてから、遷移ブロックを重ねる
	SpriteCommon::GetInstance()->DrawCommon();
	if (isCombatIntroActive_)
	{
		startOverlay_->Draw();
		Object3dCommon::GetInstance()->DrawCommon();
		if (combatIntroStep_ == 0)
		{
			enemyApproachMessage_->Update();
			enemyApproachMessage_->Draw();
			Object3d* spaceMessage = showSurvivalMessage_ ? spaceNextMessage_ : spaceStartMessage_;
			spaceMessage->Update();
			spaceMessage->Draw();
		}
		else
		{
			survivalGoalMessage_->Update();
			survivalGoalMessage_->Draw();
			spaceStartMessage_->Update();
			spaceStartMessage_->Draw();
		}
	}
	// メッセージ描画で3D用PSOに切り替わるため、遷移Spriteの前に戻す
	SpriteCommon::GetInstance()->DrawCommon();
	damageFlashOverlay_->Draw();
	if (phase_ == GamePhase::Combat && !isCombatIntroActive_)
	{
		healthBarBackground_->Draw();
		healthBarFill_->Draw();
	}
	if (isPlayerDead_)
	{
		deathFadeOverlay_->Draw();
	}
	sceneManager_->DrawTransitionEffect();
	// ImGuiの描画
	imGuiManager->ImGuiDraw();

	DirectXCommon::GetInstance()->PostDraw();
}

void GamePlayScene::DrawWork()
{
	// 作業パート専用のオブジェクトやUIはここに追加する
}

void GamePlayScene::DrawCombat()
{
	// 戦闘パート専用のオブジェクトやUIはここに追加する
	if (isEnemyAlive)
	{
		enemy3d->Draw();
	}
}
