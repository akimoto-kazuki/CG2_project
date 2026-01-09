#include "TextureManager.h"

TextureManager* TextureManager::instance = nullptr;

TextureManager* TextureManager::GetInstance() 
{
	if (instance == nullptr)
	{
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Finalize()
{
	delete instance;
	instance = nullptr;
}

void TextureManager::Initialize()
{
	textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

void TextureManager::LoadTexture(const std::string& filePath)
{
	DirectX::ScratchImage image{};
	// テクスチャデータを追加
	textureDatas.resize(textureDatas.size() + 1);
	// 追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas.back();

	// CG2にあるものを探したり、Commonにある読み込みを入れる

	textureData.filePath = textureDatas.back().filePath;
	textureData.metadata = textureDatas.back().metadata;
	textureData.resource = textureDatas.back().resource;

	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1);
	textureData.srvHandleCPU = textureDatas.back().srvHandleCPU;
	textureData.srvHandleGPU = textureDatas.back().srvHandleGPU;

	DirectX::ScratchImage mipImages{};
}