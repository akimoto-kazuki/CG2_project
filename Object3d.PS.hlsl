#include "Object3d.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
};

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

struct Camera
{
    float3 worldPosition;
};

ConstantBuffer<Camera> gCamera : register(b2);

ConstantBuffer<Material> gMaterial : register(b0);

Texture2D<float4> gTexture : register(t0);

SamplerState gSampler : register(s0);

ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
   
    
    if (textureColor.a <= 0.5f)
    {
        discard;
    }
    if (textureColor.a == 0.0f)
    {
        discard;
    }
    
    if (gMaterial.enableLighting != 0)
    {
        
        float NdotL = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
        
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        
        float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    
        float3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
    
        float RdotE = dot(reflectLight, toEye);
    
        float specularPow = pow(saturate(RdotE), gMaterial.shininess);
        
        float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        
        float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);
        
        output.color.rgb = diffuse + specular;
        
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    return output;
}

