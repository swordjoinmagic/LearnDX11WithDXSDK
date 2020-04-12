#pragma once

#include "D3DUtils.h"

#include "assimp/scene.h"

class MathF {
public:
	static const float PI;
	static const float Deg2Rad;
	static float Radians(float angle);	
	// Returns random float in [0, 1).
	static float RandF();

	// Returns random float in [a, b).
	static float RandF(float a, float b);

	static XMMATRIX LoadAssimpMatrix(const aiMatrix4x4& matrix);

	static XMMATRIX LoadAssimpMatrixAndTranspose(const aiMatrix4x4& matrix);

	static aiMatrix4x4 LoadXMMatrix(const float4x4& matrix);

	static aiMatrix4x4 LoadXMMatrix(const XMMATRIX& mat4);

	static aiVector3D LerpVector3D(const aiVector3D& start, const aiVector3D& end,float factor);

};


