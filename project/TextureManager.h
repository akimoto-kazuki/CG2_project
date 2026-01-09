#pragma once

#include "externals/DirectXTex/DirectXTex.h"
#include "DirectXCommon.h"
#include <string>

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

	void Initialize();
	std::vector<TextureData> textureDatas;

	/// <summary>
	/// テクスチャファイルの読み込み
	/// </summary>
	/// <param name= "filePath">テクスチャファイルのパス</param>

	void LoadTexture(const std::string& filePath);

private:
	static TextureManager* instance;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = default;
	TextureManager& operator=(TextureManager&) = delete;

	
};

