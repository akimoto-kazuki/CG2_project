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
	// 作業パートで作れる武器。None は素材集め後、まだ武器を作っていない状態。
	enum class WeaponType { None, WoodSword, OreSword };
	GamePhase phase_ = GamePhase::Work;
	WeaponType equippedWeapon_ = WeaponType::None; // 現在装備中の武器。戦闘の攻撃力もこの値で決める。
	// false は通常敵、true は強化後に挑む最後のボス。
	bool isBossFight_ = false;
	bool gameClearTransitionRequested_ = false; // ボス撃破後にクリア画面への遷移を重複して呼ばないための印。
	int enemyHealth_ = 0; // 現在の敵HP。攻撃を受けるたびに減る。
	int enemyMaxHealth_ = 0; // 戦闘開始時に決まる敵の最大HP。HUD表示にも使う。
	float playerAttackCooldown_ = 0.0f; // 次の攻撃までの待ち時間。連打で攻撃が速くなりすぎないようにする。
	// 作業・戦闘の両パートで共有するマップ
	TileMap tileMap_;
	void UpdateSharedGameplay();
	// Debug専用の詳細な状況・デバッグ情報を画面上部に表示する。
	void DrawGameplayHud();
	// 現在の進行状態から、次に行う作業を左上に案内する。
	void DrawObjectiveNavigation();
	// Debug/Release共通で現在のフェーズに必要な操作方法を表示する。
	void DrawControlGuide();
	// Debug/Release共通で画面下部に9枠の手持ちスロットを表示する。
	void DrawHotbar();
	void UpdateWork();
	void UpdateCombat();
	void StartCombat();
	void TryPlayerAttack();
	void HandleEnemyDefeated();
	void CraftWeapon();
	void UpdateCombatIntro();
	void UpdateCombatIntroLayout();
	void TakePlayerDamage();
	void UpdateDeathAnimation();
	void UpdateDamageFlash();
	void UpdateHealthBar();
	void TryMineFront(int verticalOffset = 0);
	void TryMineBelow();
	void TryMineTile(int tileX, int tileY, int layer);
	void DrawWork();
	void DrawCombat();

	// --- カメラ ---
	Camera* camera = nullptr;
	Vector3 rotate;
	Vector3 translate;
	Vector3 cameraOrbitTarget_ = { 0.0f, 0.0f, 10.0f };
	float cameraOrbitYaw_ = 0.0f;
	float cameraOrbitPitch_ = 0.0f;
	float cameraOrbitDistance_ = 20.0f;
	void UpdateMouseCamera();
	
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
	// 現在の道具。操作切替を無効にしている間は斧を使う。
	TileMap::ToolType currentTool_ = TileMap::ToolType::Axe;
	int dirtCount_ = 0;
	int stoneCount_ = 0;
	int oreCount_ = 0;
	int woodCount_ = 0;
	// 次の戦闘に向けた作業目標。達成判定と作業パートの案内表示で共用する。
	static constexpr int kWorkGoalWoodCount = 3;
	bool IsWorkGoalComplete() const;
	float velocityY;			// Y軸方向の現在の速度
	float gravity;				// 重力（毎フレーム下に向かって引っ張る力）
	float jumpPower;			// ジャンプ力（上に飛び上がる初速）
	bool isJumping = false;     // 現在ジャンプ中かどうかのフラグ
	// 敵
	Vector3 enemyPosition;
	Vector3 enemyRotate;
	bool isEnemyAlive = true;   // 敵の生存フラグ
	static constexpr int kMaxPlayerHealth = 3;
	int playerHealth_ = kMaxPlayerHealth;
	float enemyAttackCooldown_ = 0.0f;
	float enemyHitFlashTimer_ = 0.0f; // 攻撃を受けた敵を白く点滅させる残り時間。
	float damageFlashTimer_ = 0.0f;
	float deathAnimationTimer_ = 0.0f;
	float deathStartRotationX_ = 0.0f;
	bool isPlayerDead_ = false;
	bool gameOverTransitionRequested_ = false;

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
	Sprite* damageFlashOverlay_ = nullptr;
	Sprite* deathFadeOverlay_ = nullptr;
	Sprite* healthBarBackground_ = nullptr;
	Sprite* healthBarFill_ = nullptr;

	// --- パーティクルエミッター ---
	EulerTransform particleEffectTransform;
	ParticleEmitter* particleEmitterEffect = nullptr;
};
