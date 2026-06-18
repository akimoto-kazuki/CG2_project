#include "ParticleEmitter.h"
// コンストラクタ：必要な初期値をメンバ変数に保存する
ParticleEmitter::ParticleEmitter(const std::string& groupName, const Transform& transform, uint32_t count, float frequency)
	: groupName_(groupName)
	, transform_(transform)
	, count_(count)
	, frequency_(frequency)
	, frequencyTime_(0.0f) // タイマーは0からスタート
{
}

// 毎フレームの更新処理
void ParticleEmitter::UpdateEffect()
{
	// 本来はゲームループの正確な1フレームの経過時間（DeltaTime）を使用しますが、ここでは仮に 1/60秒 とします
	const float kDeltaTime = 1.0f / 60.0f;

	// ① 時刻を進める（前回の発生からの経過時間を蓄積する）
	frequencyTime_ += kDeltaTime;

	// ② 頻度より経過時間が大きくなったら発生
	if (frequencyTime_ >= frequency_)
	{
		// シングルトン化した ParticleManager のインスタンスを取得して Emit（発生）を実行
		ParticleManager::GetInstance()->EmitEffect(groupName_, transform_, count_);

		// ③ 予定時刻を引く（タイマーをリセット、またはオーバーした分を引く）
		frequencyTime_ -= frequency_;
	}
}

void ParticleEmitter::InputEffect()
{
	// シングルトン化した ParticleManager のインスタンスを取得して Emit（発生）を実行
	ParticleManager::GetInstance()->EmitEffect(groupName_, transform_, count_);
}

void ParticleEmitter::UpdateHitEffect()
{
	// 本来はゲームループの正確な1フレームの経過時間（DeltaTime）を使用しますが、ここでは仮に 1/60秒 とします
	const float kDeltaTime = 1.0f / 60.0f;

	// ① 時刻を進める（前回の発生からの経過時間を蓄積する）
	frequencyTime_ += kDeltaTime;

	// ② 頻度より経過時間が大きくなったら発生
	if (frequencyTime_ >= frequency_)
	{
		// シングルトン化した ParticleManager のインスタンスを取得して Emit（発生）を実行
		ParticleManager::GetInstance()->EmitHitEffect(groupName_, transform_, count_);

		// ③ 予定時刻を引く（タイマーをリセット、またはオーバーした分を引く）
		frequencyTime_ -= frequency_;
	}
}

void ParticleEmitter::InputHitEffect()
{
	// シングルトン化した ParticleManager のインスタンスを取得して Emit（発生）を実行
	ParticleManager::GetInstance()->EmitHitEffect(groupName_, transform_, count_);

}

void ParticleEmitter::UpdateSprakEffect()
{
	// 本来はゲームループの正確な1フレームの経過時間（DeltaTime）を使用しますが、ここでは仮に 1/60秒 とします
	const float kDeltaTime = 1.0f / 60.0f;

	// ① 時刻を進める（前回の発生からの経過時間を蓄積する）
	frequencyTime_ += kDeltaTime;

	// ② 頻度より経過時間が大きくなったら発生
	if (frequencyTime_ >= frequency_)
	{
		// シングルトン化した ParticleManager のインスタンスを取得して Emit（発生）を実行
		ParticleManager::GetInstance()->EmitSparkEffect(groupName_, transform_, count_);

		// ③ 予定時刻を引く（タイマーをリセット、またはオーバーした分を引く）
		frequencyTime_ -= frequency_;
	}
}

void ParticleEmitter::InputSprakEffect()
{
	// シングルトン化した ParticleManager のインスタンスを取得して Emit（発生）を実行
	ParticleManager::GetInstance()->EmitSparkEffect(groupName_, transform_, count_);
}
