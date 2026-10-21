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

namespace
{
	// ゲーム起動中、最初の戦闘だけ2ページの案内を表示する
	bool hasShownFirstCombatIntro = false;
}

void GamePlayScene::Initialize()
{
	showSurvivalMessage_ = false;
	phase_ = GamePhase::Work;
	isCombatIntroActive_ = false;
	combatIntroStep_ = 0;

	auto dxCommon = DirectXCommon::GetInstance();

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
	playerPosition = { 0.0f,0.0f,10.0f };
	playerRotate = { 0.0f,3.0f,0.0f };
	velocityY = 0.0f;         // Y軸方向の現在の速度
	gravity = -0.025f;        // 重力（毎フレーム下に向かって引っ張る力）
	jumpPower = 0.3f;         // ジャンプ力（上に飛び上がる初速）
	//	敵
	enemyPosition = { 0.0f,0.0f,10.0f };
	enemyRotate = { 0.0f,3.0f,0.0f };
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
	ModelManager::GetInstance()->LoadModel("walk.gltf");
	ModelManager::GetInstance()->LoadModel("GamePlayScene/character/fight/fight.obj");
	ModelManager::GetInstance()->LoadModel("GamePlayScene/character/survival/survival.obj");
	ModelManager::GetInstance()->LoadModel("GamePlayScene/character/spaceNext/spaceNext.obj");
	ModelManager::GetInstance()->LoadModel("GamePlayScene/character/spaceStart/spaceStart.obj");
	ModelManager::GetInstance()->LoadModel("GamePlayScene/part/workPart.obj");
	ModelManager::GetInstance()->LoadModel("GamePlayScene/part/fightPart.obj");
	ModelManager::GetInstance()->LoadModel("GamePlayScene/part/partchange.obj");
	ModelManager::GetInstance()->LoadModel("GamePlayScene/part/workNostart.obj");
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
	// walk.gltfのメッシュは約1.65単位の高さなので、等倍で表示する
	object3d->SetScale({ 1.0f, 1.0f, 1.0f });
	object3d->SetEnvironmentTextureIndex(skyboxTextureIndex); 
	//  敵
	enemy3d = new Object3d();
	enemy3d->Initialize();
	enemy3d->SetModel("walk.gltf");
	enemy3d->SetAnimation("resources", "walk.gltf");
	enemy3d->SetScale({ 1.0f, 1.0f, 1.0f });
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

	// 後々消すやつ==========
	workPartMessage_ = new Object3d();
	workPartMessage_->Initialize();
	workPartMessage_->SetModel("GamePlayScene/part/workPart.obj");
	workPartMessage_->SetEnvironmentTextureIndex(skyboxTextureIndex);
	workPartMessage_->SetTranslate({ -2.5f, 2.0f, 0.0f });
	workPartMessage_->SetScale({ 0.5f, 0.5f, 0.5f });

	fightPartMessage_ = new Object3d();
	fightPartMessage_->Initialize();
	fightPartMessage_->SetModel("GamePlayScene/part/fightPart.obj");
	fightPartMessage_->SetEnvironmentTextureIndex(skyboxTextureIndex);
	fightPartMessage_->SetTranslate({ -2.5f, 2.0f, 0.0f });
	fightPartMessage_->SetScale({ 0.5f, 0.5f, 0.5f });

	partChangeMessage_ = new Object3d();
	partChangeMessage_->Initialize();
	partChangeMessage_->SetModel("GamePlayScene/part/partchange.obj");
	partChangeMessage_->SetEnvironmentTextureIndex(skyboxTextureIndex);
	partChangeMessage_->SetTranslate({ -1.0f, 1.5f, 0.0f });
	partChangeMessage_->SetScale({ 0.5f, 0.5f, 0.5f });

	workNoPartMessage_ = new Object3d();
	workNoPartMessage_->Initialize();
	workNoPartMessage_->SetModel("GamePlayScene/part/workNostart.obj");
	workNoPartMessage_->SetEnvironmentTextureIndex(skyboxTextureIndex);
	workNoPartMessage_->SetTranslate({ -0.0f, 1.0f, 0.0f });
	workNoPartMessage_->SetScale({ 0.3f, 0.3f, 0.3f });
	// =====================

	// SkyBox
	skyBox = new SkyBox();
	skyBox->Initialize();
	// 読み込んだテクスチャの番号を SkyBox に教える
	skyBox->SetTextureIndex(skyboxTextureIndex);

	// --- パーティクル初期化 ---
	// パーティクル
	// 1. 画像の読み込みだけを行う（戻り値は受け取らない）
	TextureManager::GetInstance()->LoadTexture("Resources/circle2.png");
	TextureManager::GetInstance()->LoadTexture("Resources/gradationLine.png");
	// 2. これが「何枚目に読み込んだ画像か」で番号を直接決める
	// (例: 他に2枚読み込んでいて、これが3枚目の画像なら、0から数えて「2」になります)
	uint32_t particleTexIndex = TextureManager::GetInstance()->GetTextureIndexByFilepath("Resources/circle2.png"); // ★環境に合わせて 1 や 2 などに変えてみてください
	uint32_t particleRingTexIndex = TextureManager::GetInstance()->GetTextureIndexByFilepath("Resources/gradationLine.png");
	// 設定
	ParticleManager::GetInstance()->CreateGroup("magic", particleTexIndex);
	particleEffectTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,10.0f } };
	particleEmitterEffect = new ParticleEmitter("magic", particleEffectTransform, 1, 0.1f);
	//ヒットエフェクト
	ParticleManager::GetInstance()->CreateGroup("Hit", particleTexIndex);
	particleHitEffectTransform = { {0.05f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,10.0f } };
	particleEmitterHitEffect = new ParticleEmitter("Hit", particleHitEffectTransform, 10, 2.0f);

	ParticleManager::GetInstance()->CreateGroup("spark", particleTexIndex);
	particlesSparkEffectTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,10.0f } };
	particleEmitterSparkEffect = new ParticleEmitter("spark", particlesSparkEffectTransform, 20, 2.0f);

	ParticleManager::GetInstance()->CreateGroup("ring", particleRingTexIndex, true);
	particleRingEffectTransform = { {1.0f,1.0f,1.0f},{0.0f,2.0f,0.0f},{0.0f,0.0f,10.0f } };
	particleEmitterRingEffect = new ParticleEmitter("ring", particleRingEffectTransform, 4, 2.0f);
	ParticleManager::GetInstance()->CreateGroup("cylinder", particleRingTexIndex, false, true);		
	particleCylinderTransform = { {1.0f,0.5f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,10.0f } };	
	particleEmitterCylinderEffect = new ParticleEmitter("cylinder", particleCylinderTransform, 1, 0.0f);

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
		static_cast<float>(WinApp::kClientWidth),
		static_cast<float>(WinApp::kClientHeight)
	});
	startOverlay_->SetColor({ 0.0f, 0.0f, 0.0f, 0.55f });
	startOverlay_->Update();
}

void GamePlayScene::Finalize()
{
	for (Sprite* sprite : sprites_)
	{
		delete sprite;
	}
	sprites_.clear();
	delete startOverlay_;
	startOverlay_ = nullptr;
	delete object3d;
	delete enemy3d;
	delete enemyApproachMessage_;
	delete survivalGoalMessage_;
	delete spaceNextMessage_;
	delete spaceStartMessage_;
	// 後々消すやつ
	delete workPartMessage_;
	delete fightPartMessage_;
	delete partChangeMessage_;
	delete workNoPartMessage_;
	// ===========
	delete skyBox;
	delete camera;
	delete lineRenderer;
	// エフェクト
	delete particleEmitterEffect;
	delete particleEmitterHitEffect;
	delete particleEmitterSparkEffect;
	delete particleEmitterRingEffect;
	delete particleEmitterCylinderEffect;

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
	if (input->TriggerKey(DIK_RETURN) && !isCombatIntroActive_)
	{
		if (phase_ == GamePhase::Work)
		{
			phase_ = GamePhase::Combat;
			isEnemyAlive = true;
			isCombatIntroActive_ = true;
			combatIntroStep_ = 0;
			showSurvivalMessage_ = !hasShownFirstCombatIntro;
			hasShownFirstCombatIntro = true;
		}
		else
		{
			phase_ = GamePhase::Work;
		}
	}
	if (isCombatIntroActive_)
	{
		UpdateCombatIntro();
		// 案内中はゲームプレイ側の更新を止め、表示に必要な行列だけ更新する
		camera->SetRotate(rotate);
		camera->SetTranslate(translate);
		camera->Update();
		object3d->SetRotate(playerRotate);
		object3d->SetTranslate(playerPosition);
		object3d->Update();
		fightPartMessage_->Update();
		partChangeMessage_->Update();
		imGuiManager->ImGuiEnd();
		return;
	}

	UpdateCommon();
	if (phase_ == GamePhase::Work)
	{
		UpdateWork();
	}
	else
	{
		UpdateCombat();
	}

	// ImGui
#ifdef USE_IMGUI
	ImGui::ShowDemoWindow();
	ImGui::Text("Phase: %s (Enterで切り替え)", phase_ == GamePhase::Work ? "Work" : "Combat");
	ImGui::Text("Camera");
	ImGui::DragFloat3("cameraRotato", &rotate.x, 0.1f);
	ImGui::DragFloat3("cameraTranslate", &translate.x, 0.1f);
	ImGui::Text("Sprite");
	ImGui::DragFloat2("SpritePosition", &spritePosition.x, 0.1f);
	ImGui::DragFloat("SpriteRotation", &spriteRotation, 0.1f);
	ImGui::DragFloat4("SpriteColor", &spriteColor.x, 0.1f);
	ImGui::DragFloat2("SpriteSize", &spriteSize.x, 0.1f);
	ImGui::Text("object3D");
	ImGui::DragFloat3("ObjectPosition", &playerPosition.x, 0.1f);
	ImGui::DragFloat3("ObjectRotation", &playerRotate.x, 0.1f);
	float envCoef = object3d->GetEnvironmentCoefficient();
	if (ImGui::SliderFloat("Environment Rate", &envCoef, 0.0f, 1.0f))
	{
		object3d->SetEnvironmentCoefficient(envCoef);
	}
#endif
	imGuiManager->ImGuiEnd();
}

void GamePlayScene::UpdateCombatIntro()
{
	if (Input::GetInstance()->TriggerKey(DIK_SPACE))
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

void GamePlayScene::UpdateWork()
{
	// 作業パート固有の処理をここに追加
	workPartMessage_->Update();
	workNoPartMessage_->Update();
}

void GamePlayScene::UpdateCombat()
{
	fightPartMessage_->Update();
	// 戦闘パート固有の処理をここに追加
	// Enterで作業パートへ戻れるため、敵全滅などの条件待ちは不要
}

void GamePlayScene::UpdateCommon()
{
	auto input = Input::GetInstance();
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

	Vector3 playerMove = { 0.0f, 0.0f, 0.0f };

	if (input->PushKey(DIK_D))
	{
		playerMove.x += 0.1f;
	}
	if (input->PushKey(DIK_A))
	{
		playerMove.x -= 0.1f;
	}

	if (input->TriggerKey(DIK_SPACE) && !isJumping)
	{
		velocityY = jumpPower; // 上方向への初速を与える
		isJumping = true;      // ジャンプ状態をオンにする
	}

	if (playerMove.x != 0.0f || playerMove.z != 0.0f)
	{
		float playerSpeed = 0.1f;

		// 斜め移動時に移動速度が速くならないよう正規化
		float length = std::sqrt(playerMove.x * playerMove.x + playerMove.z * playerMove.z);
		playerMove.x /= length;
		playerMove.z /= length;

		// 1. 移動処理
		playerPosition.x += playerMove.x * playerSpeed;
		playerPosition.z += playerMove.z * playerSpeed;

		// 2. 移動方向を向くように回転（敵の処理と同じatan2を使用）
		playerRotate.y = std::atan2(playerMove.x, playerMove.z);
	}

	if (isJumping)
	{
		// 1. 座標に現在の速度を足す
		playerPosition.y += velocityY;

		// 2. 速度に重力をかけて減速させる（下方向の力を加える）
		velocityY += gravity;

		// 3. 着地判定（とりあえず Y=0.0f を地面の高さとします）
		if (playerPosition.y <= 0.0f)
		{
			playerPosition.y = 0.0f; // 地面にめり込まないように補正
			velocityY = 0.0f;     // 速度をリセット
			isJumping = false;    // ジャンプ状態を解除
		}
	}

	particleEmitterCylinderEffect->UpdateCylinderEffect();

	float pos = 0.0f;
	camera->Update();
	object3d->Update();
	partChangeMessage_->Update();
	if (isEnemyAlive)
	{
		enemy3d->Update();
		enemy3d->SetRotate(enemyRotate);
		enemy3d->SetTranslate(enemyPosition);
	}

	Vector3 diff =
	{
		playerPosition.x - enemyPosition.x,
		playerPosition.y - enemyPosition.y,
		playerPosition.z - enemyPosition.z
	};

	// 2. プレイヤーと敵の距離を計算する
	float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

	if (input->TriggerKey(DIK_F))
	{
		// 敵との距離が近く、かつ敵が生きているなら攻撃ヒット
		if (distance < 2.0f && isEnemyAlive)
		{
			isEnemyAlive = false; // 敵を消す
			particleEmitterHitEffect->InputHitEffect(); // ヒットエフェクトを出す
		}
	}

	// 敵の移動スピード（好みの速さに調整してください）
	float enemySpeed = 0.05f;

	// 3. プレイヤーと敵が少し離れている場合のみ移動処理を行う（完全に重なるのを防ぐため）
	if (distance > 0.1f)
	{
		// ベクトルを正規化（長さを1にする）して方向だけを取り出す
		Vector3 direction = {
			diff.x / distance,
			diff.y / distance,
			diff.z / distance
		};

		// 4. 敵の座標に「方向 × スピード」を足して移動させる
		enemyPosition.x += direction.x * enemySpeed;
		// 空を飛ばせたくない場合は Y 軸の追従をコメントアウトするか、重力処理を別途入れてください
		enemyPosition.y += direction.y * enemySpeed;
		enemyPosition.z += direction.z * enemySpeed;

		// (おまけ) 敵がプレイヤーの方向を向くように回転（Y軸回転）
		// atan2を使ってXとZの向きから角度を算出します
		enemyRotate.y = std::atan2(direction.x, direction.z);
	}

	object3d->DrawSkeleton(lineRenderer);
	if (isEnemyAlive)
	{
		enemy3d->DrawSkeleton(lineRenderer);
	}

	object3d->SetRotate(playerRotate);
	object3d->SetTranslate(playerPosition);

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
	partChangeMessage_->Update();

}

void GamePlayScene::Draw()
{
	auto dxCommon = DirectXCommon::GetInstance();
	auto srvManager = SrvManager::GetInstance();
	auto imGuiManager = ImGuiManager::GetInstance();

	postEffect->PreDraw();

	srvManager->PreDraw();

	Object3dCommon::GetInstance()->DrawCommon();
	object3d->Draw();
	partChangeMessage_->Draw();
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
	// ★ここに追加：パーティクルの描画
	//ParticleManager::GetInstance()->Draw(camera);

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
	sceneManager_->DrawTransitionEffect();
	// ImGuiの描画
	imGuiManager->ImGuiDraw();

	DirectXCommon::GetInstance()->PostDraw();
}

void GamePlayScene::DrawWork()
{
	// 作業パート専用のオブジェクトやUIはここに追加する
	workPartMessage_->Draw();
	workNoPartMessage_->Draw();
}

void GamePlayScene::DrawCombat()
{
	fightPartMessage_->Draw();
	// 戦闘パート専用のオブジェクトやUIはここに追加する
	if (isEnemyAlive)
	{
		// enemy3d->Draw(); // 敵モデルを表示するときに有効化
	}
}
