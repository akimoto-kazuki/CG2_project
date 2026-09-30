#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>

#include "PostEffect.h"
// スプライト
#include "Sprite.h"

// オブジェクト
#include "Object3d.h"
//カメラ
#include "Camera.h"
//
#include "MyMath.h"
// スカイボックス
#include "SkyBox.h"
// パーティクル
#include "ParticleEmitter.h"

#include "LineRenderer.h"

#include "Framework.h"

#include<sstream>

//libのリンク
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

class Game : public Framework
{
public:

	// 初期化
	void Initialiaze() override;
	// 終了
	void Finalize() override;
	// 毎フレーム更新
	void Update() override;
	// 描画
	void Draw() override;

private:

	PostEffect* postEffect = nullptr;
	
	Object3d* object3d = nullptr;
	Object3d* enemy3d = nullptr;
	
	Sprite* sprite = nullptr;
	
	SkyBox* skyBox = nullptr;
	
	// LineRendererの初期化
	LineRenderer* lineRenderer = nullptr;

	Camera* camera = nullptr;

	Vector3 rotate;
	Vector3 translate ;

	// obj用
	Vector3 playerPosition;
	Vector3 playerjRotate;
	float velocityY;        // Y軸方向の現在の速度
	float gravity;        // 重力（毎フレーム下に向かって引っ張る力）
	float jumpPower;        // ジャンプ力（上に飛び上がる初速）
	bool isJumping = false;        // 現在ジャンプ中かどうかのフラグ

	Vector3 enemyPosition;
	Vector3 enemyjRotate;
	// ★ここに追加：敵の生存フラグ
	bool isEnemyAlive = true;

	// spr用
	Vector3 position;
	float rotation;
	Vector4 color;
	Vector2 size;

	std::array<std::string, 2> spriteFile;

	std::vector<Sprite*> sprites_;

	// scale rotate translate
	EulerTransform particleEffectTransform ;
	EulerTransform particleHitEffectTransform ;
	EulerTransform particlesSparkEffectTransform ;
	EulerTransform particleRingEffectTransform ;
	EulerTransform particleCylinderTransform;

	// "magic" グループのパーティクルを、座標(0,0,0)から、1粒ずつ、0.1秒間隔で発生させるエミッターを作る
	ParticleEmitter* particleEmitterEffect;
	ParticleEmitter* particleEmitterHitEffect;
	ParticleEmitter* particleEmitterSparkEffect;
	ParticleEmitter* particleEmitterRingEffect;
	ParticleEmitter* particleEmitterCylinderEffect;

	
};

