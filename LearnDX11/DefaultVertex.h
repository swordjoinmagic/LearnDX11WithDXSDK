#pragma once

#include "D3DUtils.h"

struct DefaultVertex {
public:
	float3 pos;
	float3 normal;
	float3 tangent;
	float2 texcoord;
	DefaultVertex() {}
	DefaultVertex(
		const float3& _pos,
		const float3& _normal,
		const float3& _tangent,
		const float2& texcoord) :
		pos(_pos), normal(_normal),
		tangent(_tangent), texcoord(texcoord) {}
	DefaultVertex(
		float x, float y, float z,
		float normalX, float normalY, float normalZ,
		float tangentX, float tangentY, float tangentZ,
		float u, float v) {
		pos = float3(x, y, z);
		normal = float3(normalX, normalY, normalZ);
		tangent = float3(tangentX, tangentY, tangentZ);
		texcoord = float2(u, v);
	}
};

struct Particle {
public:
	XMFLOAT3 InitialPos;
	XMFLOAT3 InitialVel;
	XMFLOAT2 Size;
	float Age;
	unsigned int Type;
public:
	Particle() { ZeroMemory(this, sizeof(Particle)); }
};

// 带骨骼的顶点
struct SkinVertex {
public:
	float3 pos;
	float3 normal;
	float3 tangent;

	// 最多有4个骨骼影响该顶点
	int boneID[4] = { 0 };
	float weights[4] = { 0 };

	float2 texcoord;
};