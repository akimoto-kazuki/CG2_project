#pragma once

#include "Framework.h"
#include "GamePlayScene.h"

class Game : public Framework
{
public:

	// 初期化
	void Initialiaze() override;
	// 終了
	void Finalize() override;
	// 毎フレーム更新
	void Update() override;
	// 描画
	void Draw() override;

private:

	GamePlayScene* gamePlayScene_ = nullptr;
};

