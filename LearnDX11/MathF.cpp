#include "D3DUtils.h"
#include "MathF.h"

const float MathF::PI = XM_PI;
const float MathF::Deg2Rad = MathF::PI / 180.0f;

float MathF::Radians(float angle) {
	return angle * MathF::Deg2Rad;
}

// Returns random float in [0, 1).
float MathF::RandF() {
	return (float)(rand()) / (float)RAND_MAX;
}

// Returns random float in [a, b).
float MathF::RandF(float a, float b) {
	return a + RandF()*(b - a);
}

XMMATRIX MathF::LoadAssimpMatrixAndTranspose(const aiMatrix4x4& matrix) {
	return XMMatrixSet(
		matrix.a1, matrix.a2, matrix.a3, matrix.a4,
		matrix.b1, matrix.b2, matrix.b3, matrix.b4,
		matrix.c1, matrix.c2, matrix.c3, matrix.c4,
		matrix.d1, matrix.d2, matrix.d3, matrix.d4
	);
}

XMMATRIX MathF::LoadAssimpMatrix(const aiMatrix4x4& matrix) {
	return XMMatrixSet(
		matrix.a1, matrix.b1, matrix.c1, matrix.d1,
		matrix.a2, matrix.b2, matrix.c2, matrix.d2,
		matrix.a3, matrix.b3, matrix.c3, matrix.d3,
		matrix.a4, matrix.b4, matrix.c4, matrix.d4
	);
}

aiMatrix4x4 MathF::LoadXMMatrix(const float4x4& matrix) {
	return aiMatrix4x4(
		matrix._11, matrix._21, matrix._31, matrix._41,
		matrix._12, matrix._22, matrix._32, matrix._42,
		matrix._13, matrix._23, matrix._33, matrix._43,
		matrix._14, matrix._24, matrix._34, matrix._44
	);
}

aiMatrix4x4 MathF::LoadXMMatrix(const XMMATRIX& mat4) {
	float4x4 matrix;
	XMStoreFloat4x4(&matrix,mat4);
	return aiMatrix4x4(
		matrix._11, matrix._21, matrix._31, matrix._41,
		matrix._12, matrix._22, matrix._32, matrix._42,
		matrix._13, matrix._23, matrix._33, matrix._43,
		matrix._14, matrix._24, matrix._34, matrix._44
	);
}

aiVector3D MathF::LerpVector3D(const aiVector3D& start, const aiVector3D& end, float factor) {
	return start + (end - start) * factor;
}
