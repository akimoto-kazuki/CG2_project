#pragma once
#include "DirectXCommon.h"
#include "Logger.h"
#include "StringUtility.h"
#include "MyMath.h"
#include "ModelManager.h"
#include "Model.h"
#include "Camera.h"

class Object3dCommon;
class Camera;

class Object3d
{

	struct Material
	{
		MyMath::Vector4 color;
		int32_t enableLighting;
		float padding[3];
		MyMath::Matrix4x4 uvTransform;
	};

	struct TransformationMatrix
	{
		MyMath::Matrix4x4 WVP;
		MyMath::Matrix4x4 World;
	};

	struct DirectionalLight
	{
		MyMath::Vector4 color;
		MyMath::Vector3 direction;
		float intensity;
	};

	struct CameraForGPU 
	{
		MyMath::Vector3 worldPosition;
	};

public:
	
	// 初期化
	void Initialize(Object3dCommon* object3dCommon);

	void Update();

	void Draw();

	float GetRotation()const { return rotation; }
	void SetRotation(float rotation) { this->rotation = rotation; }

	void SetModel(const std::string& filePath);

	void SetScale(const Vector3& scale) { transform.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }

	void SetCamera(Camera* camera) { this->camera = camera; }

	const Vector3& GetScale()const { transform.scale; }
	const Vector3& GetRotate()const { transform.rotate; }
	const Vector3& GetTranslate()const { transform.translate; }

private:

	Object3dCommon* object3dCommon_ = nullptr;
	Camera* camera = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = nullptr;
	TransformationMatrix* transformationMatrixData = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = nullptr;
	DirectionalLight* directionalLightData = nullptr;

	MyMath::Transform transform;

	MyMath::Transform cameraTransform;

	uint32_t textureIndex = 0;

	float rotation = 0.0f;

	Model* model_ = nullptr;

	// private メンバに追加
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource = nullptr;
	CameraForGPU* cameraData = nullptr;

};

