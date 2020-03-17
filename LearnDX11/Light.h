#pragma once

#include "D3DUtils.h"

/*
	包含常用光源的结构体以及常用光照函数
*/
struct Light {
	// 理论上来说，平行光不具备位置属性
	// 这里为了渲染阴影贴图（获得光源的VP矩阵）临时设置一个
	float3 pos;
	// 平行光方向,
	float3 dir;
	float3 lightColor;
};

// 点光源
struct PointLight {
	float3 pos;
	float Constant;	// 衰减常数项
	float3 lightColor;
	float Linear;	// 衰减一次项
	float Quadratic;	// 衰减二次项
};

// 聚光灯
struct SpotLight {
	float3 pos;
	float Constant;	// 衰减常数项
	float3 dir; // 表示聚光灯朝向方向spotDir
	float Linear;	// 衰减一次项
	float3 lightColor;
	float Quadratic;	// 衰减二次项
	// 聚光灯外角的余弦值
	float outerCutOff;
	// 聚光灯内角的余弦值
	float cutOff;
};