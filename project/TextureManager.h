#pragma once

#include "externals/DirectXTex/DirectXTex.h"
#include "DirectXCommon.h"
#include <string>
#include <wrl.h>

class TextureManager
{
private:

	struct TextureData
	{
		std::string filePath;
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};

	

public:
	// シングルトンインスタンスの取得
	static TextureManager* GetInstance();
	// 終了
	void Finalize();

	void Initialize(DirectXCommon* dxCommon);
	std::vector<TextureData> textureDatas;

	/// <summary>
	/// テクスチャファイルの読み込み
	/// </summary>
	/// <param name= "filePath">テクスチャファイルのパス</param>

	void LoadTexture(const std::string& filePath);

	uint32_t GetTextureIndexByFilepath(const std::string& filePath);

	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

private:
	static TextureManager* instance;

	DirectXCommon* dxCommon_ = nullptr;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = default;
	TextureManager& operator=(TextureManager&) = delete;

	static uint32_t kSRVIndexTop;
	
};

