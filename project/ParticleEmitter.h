#pragma once
#include <string>
#include "ParticleManager.h" // ParticleManagerのEmitを呼ぶためにインクルード
//
#include "MyMath.h"

using namespace MyMath;

class ParticleEmitter
{
public:
	// コンストラクタで必要な基本情報を初期設定する
	ParticleEmitter(const std::string& groupName, const Transform& transform, uint32_t count, float frequency);
	~ParticleEmitter() = default;

	// 毎フレームの更新処理（時間経過でパーティクルを自動発生させる）
	// 通常エフェクト
	void UpdateEffect();
	void InputEffect();
	// ヒットエフェクト
	void UpdateHitEffect();
	void InputHitEffect();
	// 火花のようなエフェクト
	void UpdateSprakEffect();
	void InputSprakEffect();
	// cylinder
	void UpdateCylinderEffect();
	void InputCylinderEffect();

private:
	std::string groupName_;     // 発生させるパーティクルグループの名前
	Transform transform_;          // エミッターの設置座標
	uint32_t count_;            // 1回につき発生させる個数
	float frequency_;           // 発生間隔（秒単位。例: 0.5f なら0.5秒に1回発生）
	float frequencyTime_ = 0.0f; // 発生間隔を制御するためのタイマー（経過時間カウンタ）
};