#include "GameTimer.h"

// 构造函数用于查询性能计数器的频率
GameTimer::GameTimer() : 
	mSecondsPerCount(0.0),mDeltaTime(-1.0),mBaseTime(0),
	mPausedTime(0),mPrevTime(0),mCurrTime(0),mStopped(false){

	// 查询频率
	INT64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	mSecondsPerCount = 1.0 / (double)countsPerSec;
}

// 计算deltaTime
void GameTimer::Tick() {
	if (mStopped) {
		mDeltaTime = 0.0;
		return;
	}

	INT64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	mCurrTime = currTime;

	// 本帧与上一帧的时间差
	mDeltaTime = (mCurrTime - mPrevTime)*mSecondsPerCount;

	// 保存当前帧到prev上(用于上次计算)
	mPrevTime = mCurrTime;

	/// 使时间差为非负值
	if (mDeltaTime < 0.0)
		mDeltaTime = 0.0;

}

// 消息循环之前调用的函数
// 因为第一次调用时因为没有prev,会导致deltaTime=currTime,
// 所以游戏开始前使用reset函数设置prev为当前currTime
void GameTimer::Reset() {
	INT64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	mBaseTime = currTime;
	mPrevTime = currTime;
	mStopTime = 0;
	mStopped = false;
}

// 暂停计时器
void GameTimer::Stop() {
	if (!mStopped) {
		INT64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		// 将当前时刻计为停止时刻
		mStopTime = currTime;
		mStopped = true;
	}
}

// 用于恢复被暂停的计时器
void GameTimer::Start() {
	INT64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	// 如果正在解除暂停状态
	if (mStopped) {
		// 计算暂停时刻
		// 累计暂停时间
		mPausedTime += startTime - mStopTime;

		// 重新开始计时器
		mPrevTime = startTime;

		// 重置暂停状态
		mStopped = false;
		mStopTime = 0;
	}
}

// 获得自调用Reset函数起,不计算暂停时间,总的游戏时间
float GameTimer::TotalTime() const{
	// 特殊判断当前是否处于暂停状态
	if (mStopped) {
		return (float)((mStopTime-mPausedTime-mBaseTime)*mSecondsPerCount);
	} else {
		return (float)((mCurrTime - mPausedTime - mBaseTime)*mSecondsPerCount);
	}
}

float GameTimer::DeltaTime() const{
	return static_cast<float>(mDeltaTime);
}