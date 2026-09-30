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

void GamePlayScene::Initialiaze()
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
	playerPosition = { 0.0f,0.0f,10.0f };
	playerjRotate = { 0.0f,3.0f,0.0f };
	velocityY = 0.0f;         // Y軸方向の現在の速度
	gravity = -0.025f;        // 重力（毎フレーム下に向かって引っ張る力）
	jumpPower = 0.3f;         // ジャンプ力（上に飛び上がる初速）
	//	敵
	enemyPosition = { 0.0f,0.0f,10.0f };
	enemyjRotate = { 0.0f,3.0f,0.0f };
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
}

void GamePlayScene::Finalize()
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
	//auto input = Input::GetInstance();
	//auto imGuiManager = ImGuiManager::GetInstance();
}

void GamePlayScene::Draw()
{

}
