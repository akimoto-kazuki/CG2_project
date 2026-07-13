#pragma once
#include "MyMath.h"
#include "WinApp.h"

using namespace MyMath;

class Camera
{
private:

	Transform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 viewMatrix;

	Matrix4x4 projectionMatrix;
	float fov;
	float aspectRatio;
	float nearClip;
	float farClip;

	Matrix4x4 viewProjectionMatrix;

public:

	void Update();
	Camera();
	// Set
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }
	void SetFov(const float setFov) { fov = setFov; }
	void SetAspectRatio(const float setAspectRatio) { aspectRatio = setAspectRatio; }
	void SerNearClip(const float setNeatClip) { nearClip = setNeatClip; }
	void SetFarClip(const float setFarClip) { farClip = setFarClip; }

	// Get
	const Matrix4x4& GetWorldMatrix()const { return worldMatrix; }
	const Matrix4x4& GetViewMatrix()const { return viewMatrix; }
	const Matrix4x4& GetProjectionMatrix()const { return projectionMatrix; }
	const Matrix4x4& GetViewProjectionMatrix()const { return viewProjectionMatrix; }

	const Vector3& GetRotate()const { return transform.rotate; }
	const Vector3& GetTranslate()const { return transform.translate; }
};

