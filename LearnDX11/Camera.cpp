#include "Camera.h"
#include "MathF.h"
using namespace SJM;

XMMATRIX Camera::GetProjMatrix() {
	return XMMatrixPerspectiveFovLH(fov,aspect,Near,Far);
}

XMMATRIX Camera::GetViewMatrix() {
	return XMMatrixLookAtLH(XMLoadFloat3(&pos), XMLoadFloat3(&pos) + XMLoadFloat3(&forward),XMLoadFloat3(&up));
}

void Camera::updateCameraDirection() {
	XMVECTOR f = XMVectorSet(
		cos(MathF::Radians(Yaw))*cos(MathF::Radians(Pitch)),
		sin(MathF::Radians(Pitch)),
		sin(MathF::Radians(Yaw)) * cos(MathF::Radians(Pitch)),
		0.0f
	);

	XMStoreFloat3(&forward, XMVector3Normalize(f));
	XMStoreFloat3(&right,XMVector3Normalize(XMVector3Cross(f, XMLoadFloat3(&worldUp))));
	XMStoreFloat3(&up,XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&right),f)));
}

void Camera::ProcessKeyBoard(CameraMovement direction,float deltaTime) {
	float velocity = MovementSpeed * deltaTime;

	XMVECTOR p = XMLoadFloat3(&pos);
	XMVECTOR f = XMLoadFloat3(&forward);
	XMVECTOR r = XMLoadFloat3(&right);

	switch (direction) {
	case SJM::FORWAD:
		XMStoreFloat3(&pos, p + f * velocity);
		break;
	case SJM::BACKWARD:
		XMStoreFloat3(&pos, p - f * velocity);
		break;
	case SJM::LEFT:
		XMStoreFloat3(&pos, p + r * velocity);
		break;
	case SJM::RIGHT:
		XMStoreFloat3(&pos, p - r * velocity);
		break;
	}
}

void Camera::ProcessMouseScroll(float yOffset) {
	if (fovAngle >= 1.0f && fovAngle <= 45.0f) {
		fovAngle -= yOffset;
	}
	if (fovAngle <= 1.0f) {
		fovAngle = 1.0f;
	}
	if (fov >= 45.0f) {
		fovAngle = 45.0f;
	}
	fov = fovAngle * MathF::Radians(fovAngle);
}

void Camera::ProcessMouseMovement(float xOffset,float yOffset,bool constrainPitch) {
	xOffset *= MouseSensitivity;
	yOffset *= MouseSensitivity;

	Yaw += xOffset;
	Pitch += yOffset;

	if (constrainPitch) {
		if (Pitch > 89.0f) Pitch = 89.0f;
		if (Pitch < -89.0f) Pitch = -89.0f;
	}

	updateCameraDirection();
}