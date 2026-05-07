#pragma once

#include "externals/DirectXTex/DirectXTex.h"
#include "DirectXCommon.h"
#include <string>
#include <wrl.h>
#include <unordered_map>
#include "SrvManager.h"

class TextureManager
{
private:

	struct TextureData
	{
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		uint32_t srvIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};

	

public:
	// シングルトンインスタンスの取得
	static TextureManager* GetInstance();
	// 終了
	void Finalize();

	void Initialize(DirectXCommon* dxCommon,SrvManager* SrvManeger);
	std::unordered_map<std::string,TextureData> textureDatas;

	/// <summary>
	/// テクスチャファイルの読み込み
	/// </summary>
	/// <param name= "filePath">テクスチャファイルのパス</param>

	void LoadTexture(const std::string& filePath);

	uint32_t GetTextureIndexByFilepath(const std::string& filePath);

	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);

	const DirectX::TexMetadata& GetMetaData(const std::string& filePath);

private:
	static TextureManager* instance;

	SrvManager* srvManager = nullptr;

	DirectXCommon* dxCommon_ = nullptr;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = default;
	TextureManager& operator=(TextureManager&) = delete;

	static uint32_t kSRVIndexTop;
	
};

