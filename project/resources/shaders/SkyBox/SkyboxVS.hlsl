#include "Skybox.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b1);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float3 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    // スライド通りの処理
    output.position = mul(input.position, gTransformationMatrix.WVP).xyww;
    output.texcoord = input.position.xyz;
    
    output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrix.World));
   
    output.worldPosition = mul(input.position, gTransformationMatrix.World).xyz;
    
    return output;
}