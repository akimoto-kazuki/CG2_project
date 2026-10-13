#pragma once
#include <array>
#include <vector>
#include <string>

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
class TitleScene : public BaseScene
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
	Object3d* object3d = nullptr;
	Object3d* enemy3d = nullptr;
	Object3d* space3d = nullptr;
	Object3d* title3d = nullptr;
	Object3d* titleBack3d = nullptr;
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
	float playerMoveDirX = 1.0f;
	// 敵
	Vector3 enemyPosition;
	Vector3 enemyRotate;
	bool isEnemyAlive = true;   // 敵の生存フラグ
	// スペース
	Vector3 spacePosition;
	Vector3 spaceRotate;
	// タイトル名
	Vector3 titlePosition;
	Vector3 titleRotate;
	// タイトル背景
	Vector3 titleBackPosition;
	Vector3 titleBackRotate;
	Vector3 titleBackSize;

	// --- スプライト用 ---
	Vector3 spritePosition;
	float spriteRotation;
	Vector4 spriteColor;
	Vector2 spriteSize;
	std::array<std::string, 3> spriteFile;
	std::vector<Sprite*> sprites_;

	// 画面を覆って崩れるブロック演出
	struct TransitionBlock
	{
		Sprite* sprite = nullptr;
		float delay = 0.0f;
		float initialVelocityX = 0.0f;
		float initialVelocityY = 0.0f;
		float velocityX = 0.0f;
		float velocityY = 0.0f;
		float gravity = 0.0f;
		float rotationSpeed = 0.0f;
		bool active = false;
	};
	std::vector<TransitionBlock> transitionBlocks_;
	float transitionTimer_ = 0.0f;
	bool isTransitionPlaying_ = false;
	bool transitionBlocksStarted_ = false;

};
