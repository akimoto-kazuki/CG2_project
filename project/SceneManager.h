#pragma once

#include "BaseScene.h"

class SceneManager
{
public:
	// 次シーン予約
	void SetNextScene(BaseScene* nextScene) { nextScene_ = nextScene; }

	void Update();

	void Draw();

	~SceneManager();

	void SetInitialScene(BaseScene* initialScene);

private:

	BaseScene* scene_ = nullptr;
	BaseScene* nextScene_ = nullptr;

};

