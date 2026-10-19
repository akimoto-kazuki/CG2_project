#include "TransitionEffect.h"
#include <random>
#include "WinApp.h"
#include "MyMath.h"

using namespace MyMath;

void TransitionEffect::Initialize()
{
    constexpr int columns = 16;
    constexpr int rows = 9;

    const float blockWidth =
        static_cast<float>(WinApp::kClientWidth) / columns;
    const float blockHeight =
        static_cast<float>(WinApp::kClientHeight) / rows;

    std::random_device seed;
    std::mt19937 random(seed());

    std::uniform_real_distribution<float> delayDistribution(0.0f, 0.5f);
    std::uniform_real_distribution<float> horizontalDistribution(-5.0f, 5.0f);
    std::uniform_real_distribution<float> upwardDistribution(-8.0f, -2.0f);
    std::uniform_real_distribution<float> gravityDistribution(0.25f, 0.45f);
    std::uniform_real_distribution<float> rotationDistribution(-0.22f, 0.22f);

    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < columns; ++x)
        {
            Block block;
            block.sprite = new Sprite();
            block.sprite->Initialize("resources/white.png");

            // Spriteの基準サイズは100×100px
            block.sprite->SetSize({
                (blockWidth + 1.0f) / 100.0f,
                (blockHeight + 1.0f) / 100.0f
                });
            block.sprite->SetPosition({
                x * blockWidth,
                y * blockHeight,
                0.0f
                });
            block.sprite->SetColor({ 0.08f, 0.12f, 0.2f, 1.0f });

            block.delay = delayDistribution(random);
            block.velocityX = horizontalDistribution(random);
            block.velocityY = upwardDistribution(random);
            block.gravity = gravityDistribution(random);
            block.rotationSpeed = rotationDistribution(random);

            blocks_.push_back(block);
        }
    }
}

void TransitionEffect::Start()
{
    timer_ = 0.0f;
    isPlaying_ = true;
    isFinished_ = false;

    constexpr int columns = 16;
    constexpr int rows = 9;
    const float blockWidth =
        static_cast<float>(WinApp::kClientWidth) / columns;
    const float blockHeight =
        static_cast<float>(WinApp::kClientHeight) / rows;

    for (size_t i = 0; i < blocks_.size(); ++i)
    {
        Block& block = blocks_[i];

        // 最初は全ブロックを表示して画面を覆う
        block.active = true;
        block.velocityY = -4.0f;
        block.sprite->SetRotation(0.0f);

        const int x = static_cast<int>(i % columns);
        const int y = static_cast<int>(i / columns);
        block.sprite->SetPosition({
            x * blockWidth,
            y * blockHeight,
            0.0f
            });
    }
}

void TransitionEffect::Update()
{
    if (!isPlaying_)
    {
        return;
    }

    timer_ += 1.0f / 60.0f;

    // 画面を覆った状態で少し待つ
    constexpr float holdTime = 0.75f;

    for (Block& block : blocks_)
    {
        if (!block.active || timer_ < holdTime + block.delay)
        {
            continue;
        }

        Vector3 position = block.sprite->GetPosition();

        block.velocityY += block.gravity;
        position.x += block.velocityX;
        position.y += block.velocityY;

        block.sprite->SetPosition(position);
        block.sprite->SetRotation(
            block.sprite->GetRotation() + block.rotationSpeed
        );

        if (position.x < -150.0f ||
            position.x > WinApp::kClientWidth + 150.0f ||
            position.y < -150.0f ||
            position.y > WinApp::kClientHeight + 150.0f)
        {
            block.active = false;
        }
    }

    bool anyActive = false;
    for (const Block& block : blocks_)
    {
        anyActive = anyActive || block.active;
    }

    if (!anyActive)
    {
        isPlaying_ = false;
        isFinished_ = true;
    }

    // 表示中のブロックの頂点情報を更新
    for (Block& block : blocks_)
    {
        if (block.active)
        {
            block.sprite->Update();
        }
    }
}

void TransitionEffect::Draw() const
{
    for (const Block& block : blocks_)
    {
        if (block.active)
        {
            block.sprite->Draw();
        }
    }
}

bool TransitionEffect::IsFinished() const
{
    return isFinished_;
}

void TransitionEffect::Finalize()
{
    for (Block& block : blocks_)
    {
        delete block.sprite;
        block.sprite = nullptr;
    }

    blocks_.clear();
}