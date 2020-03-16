#include "D3DUtils.h"
#include "MathF.h"

const float MathF::PI = XM_PI;
const float MathF::Deg2Rad = MathF::PI / 180.0f;

float MathF::Radians(float angle) {
	return angle * MathF::Deg2Rad;
}