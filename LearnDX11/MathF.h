#pragma once
class MathF {
public:
	static const float PI;
	static const float Deg2Rad;
	static float Radians(float angle);	
	// Returns random float in [0, 1).
	static float RandF();

	// Returns random float in [a, b).
	static float RandF(float a, float b);
};

