#include "TransitionEffect.h"

#include <random>

#include "MyMath.h"
#include "WinApp.h"

using namespace MyMath;

void TransitionEffect::Initialize()
{
    constexpr int columns = 16;
    constexpr int rows = 9;
    const float blockWidth = static_cast<float>(WinApp::kClientWidth) / columns;
    const float blockHeight = static_cast<float>(WinApp::kClientHeight) / rows;

    std::random_device seed;
    std::mt19937 random(seed());
    std::uniform_real_distribution<float> delayJitter(0.0f, 0.08f);
    std::uniform_real_distribution<float> fallSpeedDistribution(34.0f, 46.0f);
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
            block.sprite->SetSize({ (blockWidth + 1.0f) / 100.0f, (blockHeight + 1.0f) / 100.0f });
            block.sprite->SetColor({ 0.08f, 0.12f, 0.2f, 1.0f });

            block.targetX = x * blockWidth;
            block.targetY = y * blockHeight;
            block.delay = y * 0.07f + delayJitter(random);
            block.fallSpeed = fallSpeedDistribution(random);
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
    constexpr int rows = 9;
    const float blockHeight = static_cast<float>(WinApp::kClientHeight) / rows;

    timer_ = 0.0f;
    isCovered_ = false;
    phase_ = Phase::Falling;

    for (Block& block : blocks_)
    {
        block.active = true;
        block.landed = false;
        block.sprite->SetRotation(0.0f);
        block.sprite->SetPosition({
            block.targetX,
            block.targetY - WinApp::kClientHeight - blockHeight,
            0.0f
        });
        block.sprite->Update();
    }
}

void TransitionEffect::Update()
{
    if (phase_ == Phase::Inactive || phase_ == Phase::Finished)
    {
        return;
    }

    timer_ += 1.0f / 60.0f;

    if (phase_ == Phase::Falling)
    {
        for (Block& block : blocks_)
        {
            if (block.landed || timer_ < block.delay)
            {
                continue;
            }

            Vector3 position = block.sprite->GetPosition();
            position.y += block.fallSpeed;
            if (position.y >= block.targetY)
            {
                position.y = block.targetY;
                block.landed = true;
            }
            block.sprite->SetPosition(position);
            block.sprite->Update();
        }

        bool allLanded = true;
        for (const Block& block : blocks_)
        {
            allLanded = allLanded && block.landed;
        }
        if (allLanded)
        {
            isCovered_ = true;
            phase_ = Phase::Holding;
            timer_ = 0.0f;
        }
        return;
    }

    if (phase_ == Phase::Holding)
    {
        if (timer_ >= 0.45f)
        {
            phase_ = Phase::Crumbling;
            timer_ = 0.0f;
        }
        return;
    }

    if (phase_ == Phase::Crumbling)
    {
        for (Block& block : blocks_)
        {
            if (!block.active)
            {
                continue;
            }

            Vector3 position = block.sprite->GetPosition();
            block.velocityY += block.gravity;
            position.x += block.velocityX;
            position.y += block.velocityY;
            block.sprite->SetPosition(position);
            block.sprite->SetRotation(block.sprite->GetRotation() + block.rotationSpeed);
            block.sprite->Update();

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
            phase_ = Phase::Finished;
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
    return phase_ == Phase::Finished || phase_ == Phase::Inactive;
}

bool TransitionEffect::IsCovered() const
{
    return isCovered_;
}

void TransitionEffect::Finalize()
{
    for (Block& block : blocks_)
    {
        delete block.sprite;
        block.sprite = nullptr;
    }

    blocks_.clear();
    phase_ = Phase::Inactive;
    isCovered_ = false;
}
