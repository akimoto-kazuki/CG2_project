#include "Sprite.h"
#include "SpriteCommon.h"

using namespace MyMath;

void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath)
{
	this->spriteCommon = spriteCommon;

	position_ = { 0.0f,0.0f };

	vertexResource = spriteCommon->GetDxCommon()->CreatBufferResource(sizeof(VertexData) * 6);
	indexResource = spriteCommon->GetDxCommon()->CreatBufferResource(sizeof(uint32_t) * 6);

	vertexBufferView.BufferLocation = vertexResource.Get()->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	indexBufferView.BufferLocation = indexResource.Get()->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	// 書き込むためのアドレスを取得
	vertexResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	indexResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	materialResource = spriteCommon->GetDxCommon()->CreatBufferResource(sizeof(Vector4));
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	// 今回は赤を書き込んでみる
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();

	transformationMatrixResource = spriteCommon->GetDxCommon()->CreatBufferResource(sizeof(Matrix4x4));

	transformationMatrixResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();

	textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilepath(textureFilePath);

}

void Sprite::Update()
{
	// 左下
	vertexData[0].position = { 0.0f, 1.0f, 0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	// 上
	vertexData[1].position = { 0.0f,  0.0f, 0.0f, 1.0f };
	vertexData[1].texcoord = { 0.0f,0.0f };
	// 右下
	vertexData[2].position = { 1.0f, 1.0f, 0.0f, 1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };
	// 上
	vertexData[3].position = { 1.0f,  0.0f, 0.0f, 1.0f };
	vertexData[3].texcoord = { 1.0f,0.0f };

	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;


	Transform transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	transform.rotate = { 0.0f,0.0f,rotation };
	transform.scale = { size.x,size.y,1.0f };
	transform.translate = { position_.x,position_.y,0.0f };

	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
	transformationMatrixData->WVP = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->World = worldMatrix;
}

void Sprite::Draw()
{
	spriteCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);  // VBVを設定
	// インデックス
	spriteCommon->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);

	// マテリアルCBufferの場所を設定
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// wvp用のCBufferの場所を設定]
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = spriteCommon->GetDxCommon()->GetSRVGPUDescriptorHandle(1);
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex));
	// 描画！（DrawCall／ドローコール）。3頂点で1つのインスタンス。インスタンスについては今後
	spriteCommon->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}