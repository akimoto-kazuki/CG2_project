#pragma once
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

private:


};

