#include "Object3d.h"
#include "Object3dCommon.h"
#include "LineRenderer.h"

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

	environmentCoefficient_ = 0.0f;
	// Initialize 内に追加
	cameraResource = object3dCommon_->GetDxCommon()->CreateBufferResource(sizeof(CameraForGPU));
	cameraResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&cameraData));
	// カメラの座標を設定 (とりあえず cameraTransform の位置など)
	cameraData->worldPosition = cameraTransform.translate;

}

void Object3d::Update()
{
	animationTime_ += 1.0f / 60.0f;
	animationTime_ = std::fmod(animationTime_, animation_.duration);

	AnimationClass::ApplyAnimation(skeleton_, animation_, animationTime_);
	AnimationClass::Update(skeleton_);

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

	if (model_) 
	{
		if (model_->GetMaterialData()) 
		{
			model_->GetMaterialData()->environmentCoefficient = environmentCoefficient_;
		}
	}

	Matrix4x4 localMatrix = MakeIdentity4x4();

	transformationMatrixData->WVP = Multiply(localMatrix, worldViewProjectionMatrix);
	transformationMatrixData->World = Multiply(localMatrix, worldMatrix);
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

void Object3d::DrawSkeleton(LineRenderer* lineRenderer)
{
	if (!lineRenderer || skeleton_.joints.empty()) return;

	// オブジェクト自体のワールド行列を計算
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	// 全てのJointをループ処理
	for (const auto& joint : skeleton_.joints)
	{
		// 1. 関節のワールド行列を計算 (Jointのモデル空間行列 × オブジェクトのワールド行列)
		Matrix4x4 jointWorldMatrix = Multiply(joint.skeletonSpaceMatrix, worldMatrix);

		// 関節のワールド座標を抽出 (行列の4行目/3列分)
		Vector3 jointPos = { jointWorldMatrix.m[3][0], jointWorldMatrix.m[3][1], jointWorldMatrix.m[3][2] };

		// 2. 親Jointが存在すれば、親から自分へ線を引く
		if (joint.parent.has_value())
		{
			int32_t parentIndex = joint.parent.value();
			const auto& parentJoint = skeleton_.joints[parentIndex];

			// 親関節のワールド行列と座標を計算
			Matrix4x4 parentWorldMatrix = Multiply(parentJoint.skeletonSpaceMatrix, worldMatrix);
			Vector3 parentPos = { parentWorldMatrix.m[3][0], parentWorldMatrix.m[3][1], parentWorldMatrix.m[3][2] };

			// 親 Joint と 子 Joint を白い線で繋ぐ
			lineRenderer->AddLine(parentPos, jointPos, { 1.0f, 1.0f, 1.0f, 1.0f });
		}

		// 3. 関節の位置を分かりやすくするために「小さなXYZ軸」を描画
		float sphereRadius = 0.005f; // モデルのサイズに合わせて半径を調整してください
		Vector4 sphereColor = { 0.2f, 0.8f, 1.0f, 1.0f }; // 水色
		uint32_t division = 6; // 分割数（大きくすると滑らかな球になりますが頂点数が増えます）

		lineRenderer->AddSphere(jointPos, sphereRadius, sphereColor, division);
	}
}

void Object3d::SetModel(const std::string& filePath)
{
	model_ = ModelManager::GetInstance()->FindModel(filePath);

	if (model_)
	{
		skeleton_ = AnimationClass::CreateSkeleton(model_->GetModelData().rootNode);
	}
}

void Object3d::SetAnimation(const std::string& directoryPath, const std::string& filename)
{
	// 指定されたファイルからアニメーションデータを読み込んで保持する
	animation_ = AnimationClass::LoadAnimationFile(directoryPath, filename);
	// 時間を0にリセット
	animationTime_ = 0.0f;
}
