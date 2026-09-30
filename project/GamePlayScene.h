#pragma once
#include <array>
#include <vector>
#include <string>
//カメラ
#include "Camera.h"
// オブジェクト
#include "Object3d.h"
// ポストエフェクト
#include "PostEffect.h"
// スカイボックス
#include "SkyBox.h"
// ラインレンダー
#include "LineRenderer.h"
// スプライト
#include "Sprite.h"
// パーティクル
#include "ParticleEmitter.h"

class GamePlayScene
{
public:

	// 初期化
	void Initialiaze();
	// 終了
	void Finalize();
	// 毎フレーム更新
	void Update();
	// 描画
	void Draw();

private:

	// --- カメラ ---
	Camera* camera = nullptr;
	Vector3 rotate;
	Vector3 translate;
	
	// --- 描画オブジェクト ---
	// PostEffect
	PostEffect* postEffect = nullptr;
	// Object3D
	Object3d* object3d = nullptr;
	Object3d* enemy3d = nullptr;
	// スカイボックス
	SkyBox* skyBox = nullptr;
	// ラインレンダー
	LineRenderer* lineRenderer = nullptr;

	// --- 3Dオブジェクト用パラメータ ---
	// プレイヤー
	Vector3 playerPosition;
	Vector3 playerjRotate;
	float velocityY;			// Y軸方向の現在の速度
	float gravity;				// 重力（毎フレーム下に向かって引っ張る力）
	float jumpPower;			// ジャンプ力（上に飛び上がる初速）
	bool isJumping = false;     // 現在ジャンプ中かどうかのフラグ
	// 敵
	Vector3 enemyPosition;
	Vector3 enemyjRotate;
	bool isEnemyAlive = true;   // 敵の生存フラグ

	// --- スプライト用 ---
	Vector3 spritePosition;
	float spriteRotation;
	Vector4 spriteColor;
	Vector2 spriteSize;
	std::array<std::string, 2> spriteFile;
	std::vector<Sprite*> sprites_;

	// --- パーティクルエミッター ---
	EulerTransform particleEffectTransform;
	EulerTransform particleHitEffectTransform;
	EulerTransform particlesSparkEffectTransform;
	EulerTransform particleRingEffectTransform;
	EulerTransform particleCylinderTransform;

	ParticleEmitter* particleEmitterEffect = nullptr;
	ParticleEmitter* particleEmitterHitEffect = nullptr;
	ParticleEmitter* particleEmitterSparkEffect = nullptr;
	ParticleEmitter* particleEmitterRingEffect = nullptr;
	ParticleEmitter* particleEmitterCylinderEffect = nullptr;
};
