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

    // ブロックがすべて着地して画面を覆ったか
    bool IsCovered() const;

    // ブロックのSpriteを解放
    void Finalize();

private:
    enum class Phase
    {
        Inactive,
        Falling,
        Holding,
        Crumbling,
        Finished,
    };

    struct Block
    {
        Sprite* sprite = nullptr;
        float targetX = 0.0f;
        float targetY = 0.0f;
        float delay = 0.0f;
        float fallSpeed = 0.0f;
        float velocityX = 0.0f;
        float velocityY = 0.0f;
        float gravity = 0.0f;
        float rotationSpeed = 0.0f;
        bool active = false;
        bool landed = false;
    };

    std::vector<Block> blocks_;
    float timer_ = 0.0f;
    Phase phase_ = Phase::Inactive;
    bool isCovered_ = false;
};

