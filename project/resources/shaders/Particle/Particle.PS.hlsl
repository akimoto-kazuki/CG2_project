#include "Particle.hlsli"

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
    
    // ★追加・変更：UV座標にuvTransform行列を掛け算して、動くUV座標を作る
    // UVは2D(X,Y)なので、行列計算のために一時的に4次元(X,Y,0,1)にします
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    
    // 変換したUV座標（.xy）を使ってテクスチャを読み込む
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    //float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    //if (textureColor.a <= 0.5f)
    //{
    //    discard;
    //}
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
        
        float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
        
        float NdotH = dot(normalize(input.normal), halfVector);
    
        float specularPow = pow(saturate(NdotH), gMaterial.shininess);
        
        float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        
        float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);
        
        float3 cameraToPosition = normalize(input.worldPosition - gCamera.worldPosition);
        
        float3 reflectedVector = reflect(cameraToPosition, normalize(input.normal));
        
        output.color.rgb = diffuse + specular;
        
        output.color.a = gMaterial.color.a * textureColor.a;
        
        output.color *= input.color;
    }
    else
    {
        output.color = gMaterial.color * textureColor * input.color;
    }
    return output;
}
