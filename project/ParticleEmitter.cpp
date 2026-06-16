#include "ParticleEmitter.h"
// コンストラクタ：必要な初期値をメンバ変数に保存する
ParticleEmitter::ParticleEmitter(const std::string& groupName, const Vector3& position, uint32_t count, float frequency)
	: groupName_(groupName)
	, position_(position)
	, count_(count)
	, frequency_(frequency)
	, frequencyTime_(0.0f) // タイマーは0からスタート
{
}

// 毎フレームの更新処理
void ParticleEmitter::Update()
{
	// 本来はゲームループの正確な1フレームの経過時間（DeltaTime）を使用しますが、ここでは仮に 1/60秒 とします
	const float kDeltaTime = 1.0f / 60.0f;

	// ① 時刻を進める（前回の発生からの経過時間を蓄積する）
	frequencyTime_ += kDeltaTime;

	// ② 頻度より経過時間が大きくなったら発生
	if (frequencyTime_ >= frequency_)
	{
		// シングルトン化した ParticleManager のインスタンスを取得して Emit（発生）を実行
		ParticleManager::GetInstance()->Emit(groupName_, position_, count_);

		// ③ 予定時刻を引く（タイマーをリセット、またはオーバーした分を引く）
		frequencyTime_ -= frequency_;
	}
}