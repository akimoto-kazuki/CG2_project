#include "Camera.h"

void Camera::Update()
{
	worldMatrix = MakeAffineMatrix(transeform.scale, transeform.rotate, transeform.translate);
	viewMatrix = Inverse(worldMatrix);

	projectionMatrix = MakePerspectiveFovMatrix(fov, aspectRatio, nearClip, farClip);

	viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
}

Camera::Camera()
	: transeform({{1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f}})
	, fov(0.45f)
	, aspectRatio(float(WinApp::kClientWidth)/float(WinApp::kClientHeight))
	, nearClip(0.1f)
	, farClip(100.0f)
	, worldMatrix(MakeAffineMatrix(transeform.scale, transeform.rotate, transeform.translate))
	, viewMatrix(Inverse(worldMatrix))
	, projectionMatrix(MakePerspectiveFovMatrix(fov, aspectRatio, nearClip, farClip))
	, viewProjectionMatrix(Multiply(viewMatrix, projectionMatrix))
{}
