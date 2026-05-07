#include "TextureManager.h"

TextureManager* TextureManager::instance = nullptr;

uint32_t TextureManager::kSRVIndexTop = 1;

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

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvManager* SrvManeger)
{
	dxCommon_ = dxCommon;
	srvManager = SrvManeger;
	textureDatas.reserve(SrvManager::kMaxSRVCount);
}

void TextureManager::LoadTexture(const std::string& filePath)
{
	assert(textureDatas.size() + kSRVIndexTop < SrvManager::kMaxSRVCount);

	if (textureDatas.contains(filePath))
	{
		return;
	}

	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));
	// ミニマップの生成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// テクスチャデータを追加
	// 追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas[filePath];

	// CG2にあるものを探したり、Commonにある読み込みを入れる
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = dxCommon_->CreatTextureResource(textureData.metadata);

	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;
	textureData.srvIndex = srvManager->Allocate();
	textureData.srvHandleCPU = srvManager->GetCPUDescriptorHandle(srvIndex);
	textureData.srvHandleGPU = srvManager->GetGPUDescriptorHandle(srvIndex);

	// metadataを基にSRVを設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

	// SRVの生成
	dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);
	// DepthStencilTextureをウィンドウのサイズで作成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = dxCommon_->CreateDepthStencilTextureResource(dxCommon_->GetDevice(), WinApp::kClientWidth, WinApp::kClientHeight);

	dxCommon_->UploadTextureData(textureData.resource, mipImages);
	
}

uint32_t TextureManager::GetTextureIndexByFilepath(const std::string& filePath)
{
	// 1. 指定したパスのデータがあるか探す
	auto it = textureDatas.find(filePath);

	// 2. 見つかったら、その構造体の中にある srvIndex を返す
	if (it != textureDatas.end())
	{
		return it->second.srvIndex;
	}
	//if (textureDatas.contains(filePath))
	//{
	//	// 読み込み済みなら要素数を返す
	//	uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin()));
	//	return textureIndex;
	//}

	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath)
{
	// 範囲外指定違反チェック
	assert(textureDatas.contains(filePath));
	TextureData& textureData = textureDatas[filePath];
	return textureData.srvHandleGPU;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath)
{
	// TODO: return ステートメントをここに挿入します
	assert(textureDatas.contains(filePath));
	TextureData& textureData = textureDatas[filePath];
	return textureData.metadata;
}
