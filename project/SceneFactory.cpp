#include "SceneFactory.h"

#include "GameClearScene.h"
#include "GameOverScene.h"
#include "GamePlayScene.h"
#include "TitleScene.h"

BaseScene* SceneFactory::CreateScene(SceneType sceneType)
{
	switch (sceneType)
	{
	case SceneType::Title:
		return new TitleScene();
	case SceneType::GamePlay:
		return new GamePlayScene();
	case SceneType::GameClear:
		return new GameClearScene();
	case SceneType::GameOver:
		return new GameOverScene();
	default:
		return nullptr;
	}
}
