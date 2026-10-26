#include "Object3d.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
    float4 color;
};

ConstantBuffer<TransformationMatrix> gtransformationMatrix : register(b0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 instancePosition : INSTANCEPOSITION0;
    float4 instanceColor : INSTANCECOLOR0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    float4 worldPosition = input.position + float4(input.instancePosition, 0.0f);
    output.position = mul(worldPosition, gtransformationMatrix.WVP);
    
    output.texcoord = input.texcoord;
    
    output.normal = normalize(mul(input.normal, (float3x3) gtransformationMatrix.World));
   
    output.worldPosition = mul(worldPosition, gtransformationMatrix.World).xyz;
    output.color = gtransformationMatrix.color * input.instanceColor;
    
    return output;
    
}
