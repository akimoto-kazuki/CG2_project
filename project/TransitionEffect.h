#pragma once

#include <array>
#include <vector>
#include <string>

#include "Sprite.h"
// 画面遷移のクラス
class TransitionEffect
{
public:

    // ブロックを作り、演出を開始する
    void Initialize();

    // 演出開始。ここでは次のシーンの指定はしない
    void Start();

    // 毎フレーム更新
    void Update();

    // ブロックを描画
    void Draw() const;

    // すべてのブロックが画面外に出たら終了
    bool IsFinished() const;

    // ブロックのSpriteを解放
    void Finalize();

private:

    struct Block
    {
        Sprite* sprite = nullptr;

        // 崩れ始めるまでの時間差
        float delay = 0.0f;

        // 崩れるときの速度
        float velocityX = 0.0f;
        float velocityY = 0.0f;
        float gravity = 0.0f;
        float rotationSpeed = 0.0f;

        bool active = false;
    };

    std::vector<Block> blocks_;

    float timer_ = 0.0f;
    bool isPlaying_ = false;
    bool isFinished_ = true;
};

