#include "SceneManager.h"
#include <cassert>

void SceneManager::Update()
{

	if (scene_)
	{
		// 実行中シーンを更新する
		scene_->Update();
	}

	transitionEffect_.Update();

	// 次シーンの予約があるなら
	if (nextScene_ && transitionEffect_.IsCovered())
	{
		// 旧シーンの終了
		scene_->Finalize();
		delete scene_;

		// シーン切り替え
		scene_ = nextScene_;
		nextScene_ = nullptr;

		scene_->SetSceneManager(this);

		// 次シーンを初期化する
		scene_->Initialize();
	}
}

void SceneManager::Draw()
{
	if (scene_)
	{
		scene_->Draw();
	}
}

void SceneManager::DrawTransitionEffect()
{
	transitionEffect_.Draw();
}

SceneManager::~SceneManager()
{
	// 最後のシーンの終了と解放
	if (scene_)
	{
		scene_->Finalize();
		delete scene_;
	}
	delete nextScene_;
	transitionEffect_.Finalize();
}

void SceneManager::SetInitialScene(SceneType initialSceneType)
{
	scene_ = SceneFactory::CreateScene(initialSceneType);
	assert(scene_ && "SceneFactory failed to create the initial scene");
	scene_->SetSceneManager(this);
	scene_->Initialize();
	// TitleScene::Initialize() が white.png を読み込んだあとにブロックを作る
	transitionEffect_.Initialize();
}

void SceneManager::StartTransition(SceneType nextSceneType)
{
	BaseScene* nextScene = SceneFactory::CreateScene(nextSceneType);
	if (!nextScene)
	{
		assert(false && "SceneFactory failed to create the requested scene");
		return;
	}
	if (nextScene_ != nullptr)
	{
		delete nextScene;
		return;
	}

	nextScene_ = nextScene;
	transitionEffect_.Start();
}

void SceneManager::SetNextScene(SceneType nextSceneType)
{
	BaseScene* nextScene = SceneFactory::CreateScene(nextSceneType);
	if (!nextScene)
	{
		assert(false && "SceneFactory failed to create the requested scene");
		return;
	}
	if (nextScene_)
	{
		delete nextScene;
		return;
	}
	nextScene_ = nextScene;
}
