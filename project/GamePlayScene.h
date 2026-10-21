#pragma once
#include <array>
#include <vector>
#include <string>

// シーンマネージャー
#include "SceneManager.h"
// ベースシーン
#include "BaseScene.h"
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

class GamePlayScene : public BaseScene
{
public:

	// 初期化
	void Initialize() override;
	// 終了
	void Finalize() override;
	// 毎フレーム更新
	void Update() override;
	// 描画
	void Draw() override;

private:

	// --- カメラ ---
	Camera* camera = nullptr;
	Vector3 rotate;
	Vector3 translate;
	
	// --- 描画オブジェクト ---
	// PostEffect
	PostEffect* postEffect = nullptr;
	// Object3D
	// プレイヤー
	Object3d* object3d = nullptr;
	// 敵
	Object3d* enemy3d = nullptr;
	// 戦闘開始時の案内モデル
	Object3d* enemyApproachMessage_ = nullptr;
	Object3d* survivalGoalMessage_ = nullptr;
	// spaceを押す案内モデル
	Object3d* spaceNextMessage_ = nullptr;
	Object3d* spaceStartMessage_ = nullptr;
	// スカイボックス
	SkyBox* skyBox = nullptr;
	// ラインレンダー
	LineRenderer* lineRenderer = nullptr;

	// --- 3Dオブジェクト用パラメータ ---
	// プレイヤー
	Vector3 playerPosition;
	Vector3 playerRotate;
	float velocityY;			// Y軸方向の現在の速度
	float gravity;				// 重力（毎フレーム下に向かって引っ張る力）
	float jumpPower;			// ジャンプ力（上に飛び上がる初速）
	bool isJumping = false;     // 現在ジャンプ中かどうかのフラグ
	// 敵
	Vector3 enemyPosition;
	Vector3 enemyRotate;
	bool isEnemyAlive = true;   // 敵の生存フラグ

	bool isCombatIntroActive_ = true;
	bool showSurvivalMessage_ = false;
	int combatIntroStep_ = 0;

	// --- スプライト用 ---
	Vector3 spritePosition;
	float spriteRotation;
	Vector4 spriteColor;
	Vector2 spriteSize;
	std::array<std::string, 2> spriteFile;
	std::vector<Sprite*> sprites_;
	Sprite* startOverlay_ = nullptr;

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
