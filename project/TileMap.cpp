#include "TileMap.h"

#include <cmath>

void TileMap::Initialize(uint32_t environmentTextureIndex)
{
    Finalize();

    for (int layerIndex = 0; layerIndex < kVerticalLayers; ++layerIndex)
    {
        const int layer = layerIndex + kMinVerticalLayer;
        for (int y = 0; y < kHeight; ++y)
        {
            for (int x = 0; x < kWidth; ++x)
            {
                TileType type = TileType::Air;
                if (layer == kMinVerticalLayer)
                {
                    type = TileType::Bedrock;
                }
                else if (layer >= -5 && layer <= -4)
                {
                    type = TileType::Stone;
                }
                else if (layer >= -3 && layer <= 0)
                {
                    type = TileType::Dirt;
                }
                tiles_[layerIndex][y][x] = type;
            }
        }
    }

    // 地上の木ブロック
    tiles_[ToLayerIndex(1)][3][3] = TileType::Wood;
    tiles_[ToLayerIndex(2)][3][3] = TileType::Wood;
    tiles_[ToLayerIndex(1)][3][4] = TileType::Wood;
    tiles_[ToLayerIndex(1)][5][6] = TileType::Wood;
    tiles_[ToLayerIndex(2)][5][6] = TileType::Wood;

    // 地下にある鉱石サンプル
    tiles_[ToLayerIndex(-4)][5][6] = TileType::Ore;
    tiles_[ToLayerIndex(-5)][5][6] = TileType::Ore;

    CreateRenderObjects(environmentTextureIndex);
}

void TileMap::Finalize()
{
    for (int layerIndex = 0; layerIndex < kVerticalLayers; ++layerIndex)
    {
        for (int y = 0; y < kHeight; ++y)
        {
            for (int x = 0; x < kWidth; ++x)
            {
                delete blockObjects_[layerIndex][y][x];
                blockObjects_[layerIndex][y][x] = nullptr;
            }
        }
    }
}

void TileMap::CreateRenderObjects(uint32_t environmentTextureIndex)
{
    for (int layerIndex = 0; layerIndex < kVerticalLayers; ++layerIndex)
    {
        const int layer = layerIndex + kMinVerticalLayer;
        for (int y = 0; y < kHeight; ++y)
        {
            for (int x = 0; x < kWidth; ++x)
            {
                const float worldX = (x - (kWidth - 1) * 0.5f) * kTileSize;
                const float worldZ = kMapCenterZ + (y - (kHeight - 1) * 0.5f) * kTileSize;

                const TileType type = tiles_[layerIndex][y][x];
                if (type != TileType::Air && type != TileType::Bedrock)
                {
                    Object3d* block = new Object3d();
                    block->Initialize();
                    block->SetModel("block/block.obj");
                    block->SetEnvironmentTextureIndex(environmentTextureIndex);
                    block->SetScale({ kTileSize, kTileSize, kTileSize });
                    block->SetTranslate({ worldX, (layer + 0.5f) * kTileSize, worldZ });
                    switch (type)
                    {
                    case TileType::Dirt:
                        block->SetColor({ 0.55f, 0.30f, 0.12f, 1.0f });
                        break;
                    case TileType::Stone:
                        block->SetColor({ 0.55f, 0.58f, 0.62f, 1.0f });
                        break;
                    case TileType::Ore:
                        block->SetColor({ 1.0f, 0.70f, 0.12f, 1.0f });
                        break;
                    case TileType::Wood:
                        block->SetColor({ 0.35f, 0.16f, 0.06f, 1.0f });
                        break;
                    default:
                        break;
                    }
                    blockObjects_[layerIndex][y][x] = block;
                }
            }
        }
    }
}

void TileMap::Update()
{
    for (int layerIndex = 0; layerIndex < kVerticalLayers; ++layerIndex)
    {
        for (int y = 0; y < kHeight; ++y)
        {
            for (int x = 0; x < kWidth; ++x)
            {
                if (blockObjects_[layerIndex][y][x])
                {
                    blockObjects_[layerIndex][y][x]->Update();
                }
            }
        }
    }
}

void TileMap::Draw()
{
    for (int layerIndex = 0; layerIndex < kVerticalLayers; ++layerIndex)
    {
        for (int y = 0; y < kHeight; ++y)
        {
            for (int x = 0; x < kWidth; ++x)
            {
                if (blockObjects_[layerIndex][y][x])
                {
                    blockObjects_[layerIndex][y][x]->Draw();
                }
            }
        }
    }
}

TileMap::TileType TileMap::GetTile(int x, int y) const
{
    return GetTile(x, y, 0);
}

TileMap::TileType TileMap::GetTile(int x, int y, int layer) const
{
    if (x < 0 || x >= kWidth || y < 0 || y >= kHeight)
    {
        return TileType::Wall;
    }
    if (layer < kMinVerticalLayer || layer > kMaxVerticalLayer)
    {
        return TileType::Wall;
    }

    return tiles_[ToLayerIndex(layer)][y][x];
}

bool TileMap::WorldToTile(const Vector3& worldPosition, int& tileX, int& tileY) const
{
    // マップの左端・手前端を基準に、ワールド座標からマス番号へ変換する
    tileX = static_cast<int>(std::floor(worldPosition.x / kTileSize + kWidth * 0.5f));
    tileY = static_cast<int>(std::floor((worldPosition.z - kMapCenterZ) / kTileSize + kHeight * 0.5f));

    return tileX >= 0 && tileX < kWidth && tileY >= 0 && tileY < kHeight;
}

Vector3 TileMap::TileToWorld(int tileX, int tileY, float worldY) const
{
    return {
        (tileX - (kWidth - 1) * 0.5f) * kTileSize,
        worldY,
        kMapCenterZ + (tileY - (kHeight - 1) * 0.5f) * kTileSize
    };
}

bool TileMap::CanOccupyWorldPosition(const Vector3& worldPosition, float radius, float height) const
{
	// プレイヤーのXZ平面上の四隅を調べ、どれかがブロックと重なれば通行不可にする
    const float offsets[] = { -radius, radius };
    for (float offsetX : offsets)
    {
        for (float offsetZ : offsets)
        {
            const Vector3 checkPosition = {
                worldPosition.x + offsetX,
                worldPosition.y,
                worldPosition.z + offsetZ
            };

            int tileX = 0;
            int tileY = 0;
            if (!WorldToTile(checkPosition, tileX, tileY))
            {
                return false;
            }

            const float playerBottom = worldPosition.y;
            const float playerTop = worldPosition.y + height;
            for (int layer = kMinVerticalLayer; layer <= kMaxVerticalLayer; ++layer)
            {
                if (GetTile(tileX, tileY, layer) == TileType::Air)
                {
                    continue;
                }

                const float blockBottom = layer * kTileSize;
                const float blockTop = (layer + 1) * kTileSize;
                if (playerBottom < blockTop && playerTop > blockBottom)
                {
                    return false;
                }
            }
        }
    }

    return true;
}

bool TileMap::FindLandingSurface(const Vector3& worldPosition, float radius,
	float previousFeetY, float nextFeetY, float& landingY) const
{
	// 足元の範囲にある床のうち、今回の落下中に通過した一番高い面を着地点にする
	bool foundSurface = false;
	landingY = 0.0f;

	const int minX = static_cast<int>(std::floor((worldPosition.x - radius) / kTileSize + kWidth * 0.5f));
	const int maxX = static_cast<int>(std::floor((worldPosition.x + radius) / kTileSize + kWidth * 0.5f));
	const int minY = static_cast<int>(std::floor((worldPosition.z - radius - kMapCenterZ) / kTileSize + kHeight * 0.5f));
	const int maxY = static_cast<int>(std::floor((worldPosition.z + radius - kMapCenterZ) / kTileSize + kHeight * 0.5f));

	for (int tileY = minY; tileY <= maxY; ++tileY)
	{
		for (int tileX = minX; tileX <= maxX; ++tileX)
		{
			if (tileX < 0 || tileX >= kWidth || tileY < 0 || tileY >= kHeight)
			{
				continue;
			}

			for (int layer = kMinVerticalLayer; layer <= kMaxVerticalLayer; ++layer)
			{
				if (GetTile(tileX, tileY, layer) == TileType::Air)
				{
					continue;
				}

				const float surfaceY = (layer + 1) * kTileSize;
				const float tileCenterX = (tileX - (kWidth - 1) * 0.5f) * kTileSize;
				const float tileCenterZ = kMapCenterZ + (tileY - (kHeight - 1) * 0.5f) * kTileSize;
				const bool overlapsX = worldPosition.x + radius > tileCenterX - kTileSize * 0.5f &&
					worldPosition.x - radius < tileCenterX + kTileSize * 0.5f;
				const bool overlapsZ = worldPosition.z + radius > tileCenterZ - kTileSize * 0.5f &&
					worldPosition.z - radius < tileCenterZ + kTileSize * 0.5f;

				const bool crossedSurface = previousFeetY >= surfaceY && nextFeetY <= surfaceY;
				if (overlapsX && overlapsZ && crossedSurface && (!foundSurface || surfaceY > landingY))
				{
					landingY = surfaceY;
					foundSurface = true;
				}
			}
		}
	}

	return foundSurface;
}

bool TileMap::MineBlock(int x, int y, int layer, ToolType tool, TileType& minedType)
{
	if (x < 0 || x >= kWidth || y < 0 || y >= kHeight ||
		layer < kMinVerticalLayer || layer > kMaxVerticalLayer)
	{
		return false;
	}

	const int layerIndex = ToLayerIndex(layer);
	const TileType targetType = tiles_[layerIndex][y][x];
	// 素材ごとに対応する道具を限定する
	const bool canMine =
		(targetType == TileType::Dirt && tool == ToolType::Shovel) ||
		((targetType == TileType::Stone || targetType == TileType::Ore) && tool == ToolType::Pickaxe) ||
		(targetType == TileType::Wood && tool == ToolType::Axe);
	if (!canMine)
	{
		return false;
	}

	minedType = targetType;
	tiles_[layerIndex][y][x] = TileType::Air;
	delete blockObjects_[layerIndex][y][x];
	blockObjects_[layerIndex][y][x] = nullptr;
	return true;
}
