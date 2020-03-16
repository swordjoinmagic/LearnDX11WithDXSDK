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