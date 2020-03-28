#pragma once

#include "D3DUtils.h"

/*
	�������ù�Դ�Ľṹ���Լ����ù��պ���
*/
struct Light {
	// ��������˵��ƽ�йⲻ�߱�λ������
	// ����Ϊ����Ⱦ��Ӱ��ͼ����ù�Դ��VP������ʱ����һ��
	float3 pos;
	float empty1;
	// ƽ�йⷽ��
	float3 dir;
	float empty2;
	float3 lightColor;
	float empty3;
};

// ���Դ
struct PointLight {
	float3 pos;
	float Constant;	// ˥��������
	float3 lightColor;
	float Linear;	// ˥��һ����
	float Quadratic;	// ˥��������
	float3 empty;
};

// �۹��
struct SpotLight {
	float3 pos;
	float Constant;	// ˥��������
	float3 dir; // ��ʾ�۹�Ƴ�����spotDir
	float Linear;	// ˥��һ����
	float3 lightColor;
	float Quadratic;	// ˥��������
	// �۹����ǵ�����ֵ
	float outerCutOff;
	// �۹���ڽǵ�����ֵ
	float cutOff;
};