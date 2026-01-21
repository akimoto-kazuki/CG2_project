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

void TextureManager::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
	textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

void TextureManager::LoadTexture(const std::string& filePath)
{

	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](TextureData& textureData)
		{return textureData.filePath == filePath; }
	);

	assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount);

	if (it != textureDatas.end())
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
	textureDatas.resize(textureDatas.size() + 1);
	// 追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas.back();

	// CG2にあるものを探したり、Commonにある読み込みを入れる

	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = dxCommon_->CreatTextureResource(textureData.metadata);

	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;
	textureData.srvHandleCPU = dxCommon_->GetSRVCPUDescriptorHandle(srvIndex);
	textureData.srvHandleGPU = dxCommon_->GetSRVGPUDescriptorHandle(srvIndex);

	// metadataを基にSRVを設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

	// SRVを作成する
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon_->GetSRVCPUDescriptorHandle(1);

	// SRVの生成
	dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureSrvHandleCPU);
	// DepthStencilTextureをウィンドウのサイズで作成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = dxCommon_->CreateDepthStencilTextureResource(dxCommon_->GetDevice(), WinApp::kClientWidth, WinApp::kClientHeight);

	dxCommon_->UploadTextureData(textureData.resource, mipImages);
	
}

uint32_t TextureManager::GetTextureIndexByFilepath(const std::string& filePath)
{
	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](TextureData& textureData) {return textureData.filePath == filePath; }
	);
	if (it != textureDatas.end())
	{
		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
		return textureIndex;
	}

	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex)
{
	assert(textureIndex < dxCommon_->kMaxSRVCount);
	TextureData& textureData = textureDatas.at(textureIndex);
	return textureData.srvHandleGPU;
}
