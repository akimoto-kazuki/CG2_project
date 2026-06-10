#include "SkyBox.h"
#include "SkyBoxCommon.h" // プロジェクトのファイル名に合わせて大文字小文字を調整してください
#include "SrvManager.h"
#include <cassert>

using namespace MyMath;

void SkyBox::Initialize(SkyBoxCommon* skyboxCommon)
{
	assert(skyboxCommon);
	this->skyboxCommon_ = skyboxCommon;

	// Object3dCommonと同様に、初期化時点でデフォルトカメラがあればセットしておく
	this->camera = skyboxCommon_->GetDefaultCamera();

	auto dxCommon = skyboxCommon_->GetDxCommon();

	// 1. WVP行列用の定数バッファ作成 (Object3d.cpp からの移植)
	transformationMatrixResource = dxCommon->CreatBufferResource(sizeof(TransformationMatrix));
	transformationMatrixResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();

	// 2. 立方体(Cube)の頂点データ定義（8頂点）
	// お使いの SkyBox.h の VertexData { Vector4 position } の定義に完全一致させています
	VertexData vertices[8] = {
		{ { -1.0f,  1.0f, -1.0f, 1.0f } }, // 0
		{ {  1.0f,  1.0f, -1.0f, 1.0f } }, // 1
		{ {  1.0f, -1.0f, -1.0f, 1.0f } }, // 2
		{ { -1.0f, -1.0f, -1.0f, 1.0f } }, // 3
		{ { -1.0f,  1.0f,  1.0f, 1.0f } }, // 4
		{ {  1.0f,  1.0f,  1.0f, 1.0f } }, // 5
		{ {  1.0f, -1.0f,  1.0f, 1.0f } }, // 6
		{ { -1.0f, -1.0f,  1.0f, 1.0f } }, // 7
	};

	// 立方体を作るためのインデックス定義（36個・三角形12個分）
	uint32_t indices[36] = {
		0, 1, 2,  0, 2, 3, // 前面
		1, 5, 6,  1, 6, 2, // 右面
		5, 4, 7,  5, 7, 6, // 後面
		4, 0, 3,  4, 3, 7, // 左面
		4, 5, 1,  4, 1, 0, // 上面
		3, 2, 6,  3, 6, 7  // 下面
	};

	// 3. 頂点バッファの作成とデータ転送 (Model.cpp からの移植)
	vertexResource = dxCommon->CreatBufferResource(sizeof(VertexData) * _countof(vertices));
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * _countof(vertices);
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	vertexResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices, sizeof(vertices));
	vertexResource.Get()->Unmap(0, nullptr);

	// 4. インデックスバッファの作成とデータ転送
	indexResource = dxCommon->CreatBufferResource(sizeof(uint32_t) * _countof(indices));
	indexBufferView.BufferLocation = indexResource.Get()->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * _countof(indices);
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexData = nullptr;
	indexResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	std::memcpy(indexData, indices, sizeof(indices));
	indexResource.Get()->Unmap(0, nullptr);

	// 3. マテリアル用の定数バッファ作成 (Model.cpp と同様の手順)
	materialResource = dxCommon->CreatBufferResource(sizeof(Material));
	materialResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	// 初期値を設定（色は白、ライティングはOFF）
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = 0;
	materialData->uvTransform = MakeIdentity4x4();
	materialData->shininess = 0.0f;

}

void SkyBox::Update()
{
	// カメラが未設定の場合は処理をスキップ
	if (!camera)
	{
		return;
	}

	// ★Skybox固有の行列計算
	// 1. スケールは背景として十分に大きく（例: 500.0f）
	Vector3 scale = { 500.0f, 500.0f, 500.0f };
	// 2. 回転はなし
	Vector3 rotate = { 0.0f, 0.0f, 0.0f };
	// 3. 平行移動に「カメラの座標」を追従させることで、プレイヤーがどれだけ移動しても中心からずれない背景にします
	Vector3 translate = camera->GetTranslate();

	// 行列の合成 (Object3d.cpp と同様の処理)
	Matrix4x4 worldMatrix = MakeAffineMatrix(scale, rotate, translate);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, camera->GetViewProjectionMatrix());

	// 定数バッファへ転送
	transformationMatrixData->World = worldMatrix;
	transformationMatrixData->WVP = worldViewProjectionMatrix;
}

void SkyBox::Draw()
{
	auto commandList = skyboxCommon_->GetDxCommon()->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView); // インデックスバッファの設定

	// ★スライド7の資料に合わせたデータ転送
	// [0]: マテリアル定数バッファ (b0)
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// [1]: 行列定数バッファ (b1)
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	// [2]: テクスチャ (t0) -> お持ちの関数の名前に修正！
	commandList->SetGraphicsRootDescriptorTable(2, SrvManager::GetInstance()->GetGPUDescriptorHandle(textureIndex_));

	// 描画（インデックスバッファを使って36頂点描画）
	commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
}