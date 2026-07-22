#include "Line.hlsli"

float4 main(VertexOutput input) : SV_TARGET
{
    // 頂点シェーダーから送られてきた色を、そのまま画面に塗る
    return input.color;
}