#include "TitleScene.h"
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
#include <random>
void TitleScene::Initialize()
{

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

	// 画面全体を覆えるようにブロックを敷き詰める
	constexpr int columns = 16;
	constexpr int rows = 9;
	const float blockWidth = static_cast<float>(WinApp::kClientWidth) / columns;
	const float blockHeight = static_cast<float>(WinApp::kClientHeight) / rows;
	std::random_device seed;
	std::mt19937 random(seed());
	std::uniform_real_distribution<float> delayDistribution(0.0f, 0.5f);
	std::uniform_real_distribution<float> horizontalImpulse(-5.0f, 5.0f);
	std::uniform_real_distribution<float> upwardImpulse(-8.0f, -2.0f);
	std::uniform_real_distribution<float> gravityDistribution(0.25f, 0.45f);
	std::uniform_real_distribution<float> rotationDistribution(-0.22f, 0.22f);
	for (int y = 0; y < rows; ++y)
	{
		for (int x = 0; x < columns; ++x)
		{
			TransitionBlock block;
			block.sprite = new Sprite();
			block.sprite->Initialize("resources/white.png");
			// Sprite の基準頂点は 100x100px なので、ピクセル寸法を拡大率に変換する
			block.sprite->SetSize({ (blockWidth + 1.0f) / 100.0f, (blockHeight + 1.0f) / 100.0f });
			block.sprite->SetPosition({ x * blockWidth, y * blockHeight, 0.0f });
			block.sprite->SetColor({ 0.08f, 0.12f, 0.2f, 1.0f });
			block.delay = delayDistribution(random);
			block.initialVelocityX = horizontalImpulse(random);
			block.initialVelocityY = upwardImpulse(random);
			block.velocityX = block.initialVelocityX;
			block.velocityY = block.initialVelocityY;
			block.gravity = gravityDistribution(random);
			block.rotationSpeed = rotationDistribution(random);
			transitionBlocks_.push_back(block);
		}
	}
}

void TitleScene::Finalize()
{
	for (Sprite* sprite : sprites_)
	{
		delete sprite;
	}
	sprites_.clear();
	for (TransitionBlock& block : transitionBlocks_)
	{
		delete block.sprite;
	}
	transitionBlocks_.clear();
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

void TitleScene::Update()
{
	auto input = Input::GetInstance();
	auto imGuiManager = ImGuiManager::GetInstance();

	// ImGuiの設定 始
	imGuiManager->ImGuiBegin();

	// キー入力 始
	input->Update();
	if (input->TriggerKey(DIK_SPACE) && !isTransitionPlaying_)
	{
		isTransitionPlaying_ = true;
		transitionBlocksStarted_ = false;
		transitionTimer_ = 0.0f;
		for (size_t i = 0; i < transitionBlocks_.size(); ++i)
		{
			TransitionBlock& block = transitionBlocks_[i];
			// Space を押した時点で全ブロックを表示し、画面を覆う
			block.active = true;
			block.velocityX = block.initialVelocityX;
			block.velocityY = block.initialVelocityY;
			block.sprite->SetRotation(0.0f);
			int x = static_cast<int>(i % 16);
			int y = static_cast<int>(i / 16);
			block.sprite->SetPosition({ x * (static_cast<float>(WinApp::kClientWidth) / 16), y * (static_cast<float>(WinApp::kClientHeight) / 9), 0.0f });
		}
	}
	if (isTransitionPlaying_)
	{
		transitionTimer_ += 1.0f / 60.0f;
		constexpr float messageDuration = 0.75f;
		if (!transitionBlocksStarted_ && transitionTimer_ >= messageDuration)
		{
			transitionBlocksStarted_ = true;
		}
		for (size_t i = 0; i < transitionBlocks_.size(); ++i)
		{
			TransitionBlock& block = transitionBlocks_[i];
			if (!block.active || transitionTimer_ < messageDuration + block.delay)
			{
				continue;
			}
			Vector3 position = block.sprite->GetPosition();
			block.velocityY += block.gravity;
			position.x += block.velocityX;
			position.y += block.velocityY;
			block.sprite->SetPosition(position);
			block.sprite->SetRotation(block.sprite->GetRotation() + block.rotationSpeed);
			if (position.y < -150.0f || position.y > WinApp::kClientHeight + 150.0f ||
				position.x < -150.0f || position.x > WinApp::kClientWidth + 150.0f)
			{
				block.active = false;
			}
		}
		bool hasActiveBlocks = false;
		for (const TransitionBlock& block : transitionBlocks_)
		{
			hasActiveBlocks = hasActiveBlocks || block.active;
		}
		if (transitionBlocksStarted_ && !hasActiveBlocks)
		{
			isTransitionPlaying_ = false;
			transitionBlocksStarted_ = false;
		}
	}
	if (input->PushKey(DIK_0))
	{
		OutputDebugStringA("Hit 0\n");
	}

	Vector3 playerMove = { 0.0f, 0.0f, 0.0f };

	// 敵が右の画面外(12.0f)に完全に出たら、左向きにして右画面外から再スタート
	if (playerMoveDirX == 1.0f && enemyPosition.x >= 12.0f)
	{
		playerMoveDirX = -1.0f; // 左向きに変更

		// 左へ進んで戻ってくるため、プレイヤーを先頭(左)、敵を後ろ(右)に配置し直す
		// （初期位置の距離差 3.0f を維持しています）
		playerPosition.x = 12.0f;
		enemyPosition.x = 15.0f;
	}
	// 敵が左の画面外(-12.0f)に完全に出たら、右向きにして左画面外から再スタート
	else if (playerMoveDirX == -1.0f && enemyPosition.x <= -12.0f)
	{
		playerMoveDirX = 1.0f; // 右向きに変更

		// 右へ進んで戻ってくるため、プレイヤーを先頭(右)、敵を後ろ(左)に配置し直す
		playerPosition.x = -12.0f;
		enemyPosition.x = -15.0f;
	}

	// 常に 0.1f ではなく、進行方向(playerMoveDirX)を掛けた値を足す
	playerMove.x += 0.1f * playerMoveDirX;

	if (playerMove.x != 0.0f || playerMove.z != 0.0f)
	{
		float playerSpeed = 0.05f;

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

	float pos = 0.0f;
	camera->Update();
	// プレイヤー
	object3d->Update();
	object3d->SetRotate(playerRotate);
	object3d->SetTranslate(playerPosition);
	// 敵
	if (isEnemyAlive)
	{
		enemy3d->Update();
		enemy3d->SetRotate(enemyRotate);
		enemy3d->SetTranslate(enemyPosition);
	}
	// スペース
	space3d->Update();
	space3d->SetTranslate(spacePosition);
	space3d->SetRotate(spaceRotate);
	// タイトル
	title3d->Update();
	title3d->SetTranslate(titlePosition);
	title3d->SetRotate(titleRotate);
	// タイトル背景
	titleBack3d->Update();
	titleBack3d->SetTranslate(titleBackPosition);
	titleBack3d->SetRotate(titleBackRotate);
	titleBack3d->SetScale(titleBackSize);

	Vector3 diff =
	{
		playerPosition.x - enemyPosition.x,
		playerPosition.y - enemyPosition.y,
		playerPosition.z - enemyPosition.z
	};

	// 2. プレイヤーと敵の距離を計算する
	float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

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
	for (size_t i = 0; i < transitionBlocks_.size(); ++i)
	{
		TransitionBlock& block = transitionBlocks_[i];
		if (block.active)
		{
			int x = static_cast<int>(i % 16);
			int y = static_cast<int>(i / 16);
			// 崩れる順番が来るまでは画面上の元の位置に留める
			if (transitionTimer_ < 0.75f + block.delay)
				block.sprite->SetPosition({ x * (static_cast<float>(WinApp::kClientWidth) / 16), y * (static_cast<float>(WinApp::kClientHeight) / 9), 0.0f });
			block.sprite->Update();
		}
	}

	camera->SetRotate(rotate);
	camera->SetTranslate(translate);

#ifdef USE_IMGUI
	// ImGui
	if (isTransitionPlaying_)
	{
		ImDrawList* drawList = ImGui::GetForegroundDrawList();
		const char* message = "NOW TRANSITIONING...";
		const ImVec2 textSize = ImGui::CalcTextSize(message);
		const ImVec2 textPosition((WinApp::kClientWidth - textSize.x) * 0.5f, 44.0f);
		drawList->AddRectFilled(ImVec2(textPosition.x - 18.0f, textPosition.y - 10.0f), ImVec2(textPosition.x + textSize.x + 18.0f, textPosition.y + textSize.y + 10.0f), IM_COL32(0, 0, 0, 190), 8.0f);
		drawList->AddText(textPosition, IM_COL32(255, 255, 255, 255), message);
	}		
	ImGui::ShowDemoWindow();

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
	imGuiManager->ImGuiEnd();
}

void TitleScene::Draw()
{
	auto dxCommon = DirectXCommon::GetInstance();
	auto srvManager = SrvManager::GetInstance();
	auto imGuiManager = ImGuiManager::GetInstance();

	postEffect->PreDraw();

	srvManager->PreDraw();

	Object3dCommon::GetInstance()->DrawCommon();
	titleBack3d->Draw();
	object3d->Draw();
	space3d->Draw();
	title3d->Draw();
	if (isEnemyAlive)
	{
		enemy3d->Draw();
	}

	// 4. 描画
	SkyBoxCommon::GetInstance()->DrawCommon(); // Skybox用のルートシグネチャ・PSOに切り替え
	//skyBox->Draw();             // 引数なしでスッキリ呼び出せます！
	lineRenderer->Draw(camera);

	SpriteCommon::GetInstance()->DrawCommon();

	for (Sprite* sprite : sprites_)
	{
		//sprite->Draw();
	}
	for (const TransitionBlock& block : transitionBlocks_)
	{
		if (block.active)
		{
			block.sprite->Draw();
		}
	}

	postEffect->PostDraw();

	DirectXCommon::GetInstance()->PreDraw();

	postEffect->Draw();

	// ImGuiの描画
	imGuiManager->ImGuiDraw();

	DirectXCommon::GetInstance()->PostDraw();
}
