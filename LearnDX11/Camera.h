#pragma once

#include "D3DUtils.h"
#include "MathF.h"

namespace SJM {

	enum CameraMovement {
		FORWAD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	const float YAW = 90.0f;
	const float PITCH = 0.0f;
	const float SPEED = 10.0f;
	const float SENSITIVITY = 0.1f;
	const float ZOOM = 45.0f;
	const float NEARDISTANCE = 0.1f;
	const float FARDISTANCE = 50;

	class Camera {
	private:
		// 以弧度为单位
		float fov;
		float3 worldUp;
	public:
		float3 pos;
		float3 rotation;

		// 摄像机正面方向
		float3 forward;
		// 摄像机指向右的方向
		float3 right;
		// 摄像机指向上的方向
		float3 up;

		// 俯仰角(绕x轴旋转角度)和偏航角(绕y轴旋转角度)
		float Yaw, Pitch;

		// 摄像机移动速度以及鼠标转向时的敏感度
		float MovementSpeed;
		float MouseSensitivity;

		// 以角度为单位
		float fovAngle;
		float Near, Far;
		float aspect;
		Camera(float3 _pos, float3 _rotation, float _aspect) :
			pos(_pos), rotation(_rotation), fovAngle(ZOOM), Near(NEARDISTANCE), Far(FARDISTANCE), aspect(_aspect),
			worldUp(0, 1, 0),Yaw(YAW),Pitch(PITCH),forward(0,0,1),
			MovementSpeed(SPEED),MouseSensitivity(SENSITIVITY){
			fov = MathF::Radians(fovAngle);
			updateCameraDirection();
		}

		XMMATRIX GetViewMatrix();
		XMMATRIX GetProjMatrix();

		// 处理键盘事件
		void ProcessKeyBoard(CameraMovement direction, float deltaTime);

		// 鼠标移动导致的摄像机偏航角和俯仰角变化
		void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
		
		// 处理鼠标滚轮
		void ProcessMouseScroll(float yOffset);

	private:
		// 根据当前俯仰角和偏航角更新当前摄像机方向
		void updateCameraDirection();
	};
}