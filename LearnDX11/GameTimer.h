#pragma once
#ifndef GAMETIMER_H
#define GAMETIMER_H

#include <windows.h>

// 用于游戏计时的类
class GameTimer {
public:
	GameTimer();

	// 以秒作为单位
	float TotalTime() const;
	float DeltaTime() const;

	// 在开始消息循环之前调用
	void Reset();
	// 解除计时器暂停时调用
	void Start();
	// 暂停计时器时调用
	void Stop();
	// 每帧用于计算deltaTime
	void Tick();
private:
	double mSecondsPerCount;
	double mDeltaTime;

	// 用于计算游戏过程中的总时间(不计入stopTime的总时间)
	INT64 mBaseTime;
	INT64 mPausedTime;
	INT64 mStopTime;

	// 用于计算deltaTime
	INT64 mPrevTime;
	INT64 mCurrTime;

	bool mStopped;
};

#endif // !GAMETIMER_H
