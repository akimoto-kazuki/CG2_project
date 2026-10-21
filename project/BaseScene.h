#pragma once
class SceneManager;
class BaseScene
{
public:

	virtual ~BaseScene() = default;

	// 初期化
	virtual void Initialize() = 0;
	// 終了
	virtual void Finalize() = 0;
	// 毎フレーム更新
	virtual void Update() = 0;
	// 描画
	virtual void Draw() = 0;

	virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }

	protected:
	// 各シーンからシーン切り替えを依頼できるよう、基底クラスで共有する
	SceneManager* sceneManager_ = nullptr;

};

