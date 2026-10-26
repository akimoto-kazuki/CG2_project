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
#include "TileMap.h"

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
	enum class GamePhase { Work, Combat };
	GamePhase phase_ = GamePhase::Work;
	// 作業・戦闘の両パートで共有するマップ
	TileMap tileMap_;
	void UpdateCommon();
	void UpdateWork();
	void UpdateCombat();
	void UpdateCombatIntro();
	void TryMineFront();
	void TryMineBelow();
	void TryMineTile(int tileX, int tileY, int layer);
	void DrawWork();
	void DrawCombat();

	// --- カメラ ---
	Camera* camera = nullptr;
	Vector3 rotate;
	Vector3 translate;
#if defined(_DEBUG)
	Vector3 cameraOrbitTarget_ = { 0.0f, 0.0f, 10.0f };
	float cameraOrbitYaw_ = 0.0f;
	float cameraOrbitPitch_ = 0.0f;
	float cameraOrbitDistance_ = 20.0f;
	void UpdateMouseCamera();
#endif
	
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
	static constexpr float kPlayerCollisionRadius = 0.3f;
	static constexpr float kPlayerHeight = 1.6f;
	Vector3 playerPosition;
	Vector3 playerRotate;
	int playerTileX_ = 0;
	int playerTileY_ = 0;
	bool playerIsInsideMap_ = false;
	TileMap::ToolType currentTool_ = TileMap::ToolType::Shovel;
	int dirtCount_ = 0;
	int stoneCount_ = 0;
	int oreCount_ = 0;
	int woodCount_ = 0;
	float velocityY;			// Y軸方向の現在の速度
	float gravity;				// 重力（毎フレーム下に向かって引っ張る力）
	float jumpPower;			// ジャンプ力（上に飛び上がる初速）
	bool isJumping = false;     // 現在ジャンプ中かどうかのフラグ
	// 敵
	Vector3 enemyPosition;
	Vector3 enemyRotate;
	bool isEnemyAlive = true;   // 敵の生存フラグ

	bool isCombatIntroActive_ = false;
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
	ParticleEmitter* particleEmitterEffect = nullptr;
};
