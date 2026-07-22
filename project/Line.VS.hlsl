#include "Line.hlsli"

// 頂点バッファから受け取る入力データ
struct VertexInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    
    // 3D空間の座標(input.position)にカメラのVP行列を掛けて、2Dの画面座標に変換
    output.position = mul(float4(input.position, 1.0f), viewProjection);
    
    // 色はそのままピクセルシェーダーへ流す
    output.color = input.color;
    
    return output;
}