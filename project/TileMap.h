#pragma once

#include "Object3d.h"

#include <array>
#include <cstdint>

class TileMap
{
public:

    static constexpr int kWidth = 10;   // 横のマス数
    static constexpr int kHeight = 10;  // 奥行きのマス数
    static constexpr int kMinVerticalLayer = -6;
    static constexpr int kMaxVerticalLayer = 2;
    static constexpr int kVerticalLayers = kMaxVerticalLayer - kMinVerticalLayer + 1;
    static constexpr float kTileSize = 1.0f;
    static constexpr float kMapCenterZ = 10.0f;

    enum class TileType
    {
        Air,    // 空きマス
        Dirt,   // シャベルで壊せる土
        Stone,  // つるはしで壊せる石
        Ore,    // つるはしで壊せる鉱石
        Wood,   // 斧で壊せる地上ブロック
        Bedrock,// 壊せない最下層
        Wall    // 範囲外を表す壁（マップ上には描画しない）
    };

    enum class ToolType
    {
        Shovel,
        Pickaxe,
        Axe
    };

    void Initialize(uint32_t environmentTextureIndex);
    void Finalize();
    void Update();
    void Draw();

    TileType GetTile(int x, int y) const;
    TileType GetTile(int x, int y, int layer) const;
    bool WorldToTile(const Vector3& worldPosition, int& tileX, int& tileY) const;
    Vector3 TileToWorld(int tileX, int tileY, float worldY = 0.0f) const;
    bool CanOccupyWorldPosition(const Vector3& worldPosition, float radius, float height) const;
    bool FindLandingSurface(const Vector3& worldPosition, float radius,
		float previousFeetY, float nextFeetY, float& landingY) const;
    bool MineBlock(int x, int y, int layer, ToolType tool, TileType& minedType);

private:
    // tiles_[高さの層][奥行き][横]。高さの層番号は-6〜2。
    std::array<std::array<std::array<TileType, kWidth>, kHeight>, kVerticalLayers> tiles_{};
    std::array<std::array<std::array<Object3d*, kWidth>, kHeight>, kVerticalLayers> blockObjects_{};

    static constexpr int ToLayerIndex(int layer) { return layer - kMinVerticalLayer; }
    void CreateRenderObjects(uint32_t environmentTextureIndex);
};

