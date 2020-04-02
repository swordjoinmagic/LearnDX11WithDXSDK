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