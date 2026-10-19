#include "SceneManager.h"

void SceneManager::Update()
{

	if (scene_)
	{
		// 実行中シーンを更新する
		scene_->Update();
	}

	transitionEffect_.Update();

	// 次シーンの予約があるなら
	if (nextScene_)
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

void SceneManager::SetInitialScene(BaseScene* initialScene)
{
	scene_ = initialScene;
	scene_->SetSceneManager(this);
	scene_->Initialize();
	// TitleScene::Initialize() が white.png を読み込んだあとにブロックを作る
	transitionEffect_.Initialize();
}

void SceneManager::StartTransition(BaseScene* nextScene)
{
	if (nextScene_ != nullptr)
	{
		delete nextScene;
		return;
	}

	nextScene_ = nextScene;
	transitionEffect_.Start();
}
