#include "Object3d.h"
#include "Object3dCommon.h"

using namespace MyMath;

void Object3d::Initialize(Object3dCommon* object3dCommon)
{
	this->object3dCommon_ = object3dCommon;
	this->camera = object3dCommon->GetDefaultCamera();

	//トランスフォーム
	transformationMatrixResource = object3dCommon_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	transformationMatrixResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();

	//ライティング
	directionalLightResource = object3dCommon_->GetDxCommon()->CreateBufferResource(sizeof(DirectionalLight));
	directionalLightResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;

	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	environmentCoefficient_ = 0.3f;
	// Initialize 内に追加
	cameraResource = object3dCommon_->GetDxCommon()->CreateBufferResource(sizeof(CameraForGPU));
	cameraResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&cameraData));
	// カメラの座標を設定 (とりあえず cameraTransform の位置など)
	cameraData->worldPosition = cameraTransform.translate;

}

void Object3d::Update()
{

	cameraData->worldPosition = camera->GetTranslate();
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 worldViewProjectionMatrix;
	if (camera)
	{
		const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	}
	else
	{
		worldViewProjectionMatrix = worldMatrix;
	}

	if (model_) {
		if (model_->GetMaterialData()) {
			model_->GetMaterialData()->environmentCoefficient = environmentCoefficient_;
		}
	}

	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;
}

void Object3d::Draw()
{
	auto commandList = object3dCommon_->GetDxCommon()->GetCommandList();

	// wvp用のCBufferの場所を設定]
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	// ライティング
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

	commandList->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());

	// 1. SrvManagerのインスタンスを取得
	SrvManager* srvManager = SrvManager::GetInstance();

	// 2. ステップ1で保存したインデックスから、GPU上のハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE envSrvGpuHandle = srvManager->GetGPUDescriptorHandle(environmentTextureIndex_);

	// 3. ルートパラメータ[5]番にデスクリプタテーブルとしてセット
	commandList->SetGraphicsRootDescriptorTable(5, envSrvGpuHandle);

	if (model_)
	{
		model_->Draw();
	}
}

void Object3d::SetModel(const std::string& filePath)
{
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}
