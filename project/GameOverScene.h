#pragma once

#include "BaseScene.h"
#include "Sprite.h"

class GameOverScene : public BaseScene
{
public:
	void Initialize() override;
	void Finalize() override;
	void Update() override;
	void Draw() override;

private:
	Sprite* background_ = nullptr;
	Sprite* accent_ = nullptr;
	Sprite* textOverlay_ = nullptr;
	float timer_ = 0.0f;
};
