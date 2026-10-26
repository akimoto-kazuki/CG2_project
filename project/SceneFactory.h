#pragma once

#include "BaseScene.h"

// 生成できるシーンの種類
enum class SceneType
{
	Title,
	GamePlay,
	GameClear,
	GameOver,
};

class SceneFactory
{
public:
	// シーンの生成を一か所にまとめる
	static BaseScene* CreateScene(SceneType sceneType);
};
