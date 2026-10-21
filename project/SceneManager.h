#pragma once

#include "BaseScene.h"
#include "TransitionEffect.h"

class SceneManager
{
public:

	void SetInitialScene(BaseScene* initialScene);

	// 演出を開始して、次のシーンを予約する
	void StartTransition(BaseScene* nextScene);

	// 次シーン予約
	void SetNextScene(BaseScene* nextScene) { nextScene_ = nextScene; }

	void Update();

	void Draw();

	// 各シーンの描画中に呼び、画面最前面へブロックを描く
	void DrawTransitionEffect();

	~SceneManager();

private:

	BaseScene* scene_ = nullptr;
	BaseScene* nextScene_ = nullptr;
	TransitionEffect transitionEffect_;
};

