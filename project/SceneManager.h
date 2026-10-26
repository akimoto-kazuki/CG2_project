#pragma once

#include "BaseScene.h"
#include "TransitionEffect.h"
#include "SceneFactory.h"

class SceneManager
{
public:

	void SetInitialScene(SceneType initialSceneType);

	// 演出を開始して、次のシーンを予約する
	void StartTransition(SceneType nextSceneType);

	// 次シーン予約
	void SetNextScene(SceneType nextSceneType);

	void Update();

	void Draw();

	// 各シーンの描画中に呼び、画面最前面へブロックを描く
	void DrawTransitionEffect();

	// 画面遷移エフェクトが終わったか
	bool IsTransitionFinished() const { return transitionEffect_.IsFinished(); }

	~SceneManager();

private:

	BaseScene* scene_ = nullptr;
	BaseScene* nextScene_ = nullptr;
	TransitionEffect transitionEffect_;
};

