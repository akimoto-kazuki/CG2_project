struct VertexOutput
{
    float4 position : SV_POSITION; // 画面上の座標
    float4 color : COLOR; // 線の色
};

cbuffer CameraTransform : register(b0)
{
    matrix viewProjection;
};