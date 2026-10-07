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
void TitleScene::Initialiaze()
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
	playerjRotate = { 0.0f,3.0f,0.0f };
	velocityY = 0.0f;         // Y軸方向の現在の速度
	gravity = -0.025f;        // 重力（毎フレーム下に向かって引っ張る力）
	jumpPower = 0.3f;         // ジャンプ力（上に飛び上がる初速）
	//	敵
	enemyPosition = { -10.0f,-4.0f,10.0f };
	enemyjRotate = { 0.0f,3.0f,0.0f };
	
	// isEnemyAlive = true; bool型だから宣言しなくていい わかりやすくするために置いてる
	//  スプライト
	spritePosition = { 0.0f,0.0f,0.0f };
	spriteRotation = 0.0f;
	spriteColor = { 1.0f,1.0f,1.0f,1.0f };
	spriteSize = { 1.0f,1.0f };

	// --- テクスチャ＆モデル読み込み ---
	spriteFile[0] = "resources/white.png";
	spriteFile[1] = "resources/monsterBall.png";
	for (int i = 0; i < spriteFile.size(); i++)
	{
		TextureManager::GetInstance()->LoadTexture(spriteFile[i]);
	}
	ModelManager::GetInstance()->LoadModel("walk.gltf");
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

void TitleScene::Finalize()
{
	for (Sprite* sprite : sprites_)
	{
		delete sprite;
	}
	sprites_.clear();
	delete object3d;
	delete enemy3d;
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
		playerjRotate.y = std::atan2(playerMove.x, playerMove.z);
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
	object3d->Update();
	if (isEnemyAlive)
	{
		enemy3d->Update();
		enemy3d->SetRotate(enemyjRotate);
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
		enemyjRotate.y = std::atan2(direction.x, direction.z);
		
	}

	object3d->DrawSkeleton(lineRenderer);
	if (isEnemyAlive)
	{
		enemy3d->DrawSkeleton(lineRenderer);
	}

	object3d->SetRotate(playerjRotate);
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

	// ImGui
#ifdef USE_IMGUI			

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
	ImGui::DragFloat3("ObjectRotation", &playerjRotate.x, 0.1f);
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
	object3d->Draw();
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
		sprite->Draw();
	}

	postEffect->PostDraw();

	DirectXCommon::GetInstance()->PreDraw();

	postEffect->Draw();

	// ImGuiの描画
	imGuiManager->ImGuiDraw();

	DirectXCommon::GetInstance()->PostDraw();
}
