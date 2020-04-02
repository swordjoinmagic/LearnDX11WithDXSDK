#pragma once

#include "D3DUtils.h"
#include "Shader.h"
#include "Camera.h"

class ParticleSystem {
private:
	// 最大粒子数
	uint mMaxParticles;
	// 是否为第一次启动该粒子系统
	bool mFirstRun;
	
	// 当前游戏时间
	float mGameTime;
	// 时间间隔
	float mTimeStep;
	float mAge;

	float3 mEyePosW;
	float3 mEmitPosW;
	float3 mEmitDirW;

	// 用于控制粒子的生成/销毁的Shader
	std::shared_ptr<Shader> particleControlShader;
	// 用于控制每个粒子的运动轨迹的Shader
	std::shared_ptr<Shader> effectShader;

	// 用于第一次启动粒子系统时进行绘制的缓冲区
	ComPtr<ID3D11Buffer> mInitVB;

	// 两个缓冲区,用于流输出阶段和渲染阶段
	ComPtr<ID3D11Buffer> mDrawVB;
	ComPtr<ID3D11Buffer> mStreamOutVB;

	// 噪声贴图(CPU生成)
	ComPtr<ID3D11ShaderResourceView> mRandomTexSRV;
	// 纹理贴图数组
	ComPtr<ID3D11ShaderResourceView> mTexArraySRV;

	// 粒子结构的输入布局
	ComPtr<ID3D11InputLayout> particleIL;
public:
	ParticleSystem();

	float GetAge() const;

	void SetEyePos(const float3& eyePosW);
	void SetEmitPos(const float3& emitPosW);
	void SetEmitDir(const float3& emitDirW);

	void Init(
		ID3D11Device* device,
		std::shared_ptr<Shader> particleControlShader,
		std::shared_ptr<Shader> effectShader,
		ComPtr<ID3D11ShaderResourceView> texArraySRV,
		ComPtr<ID3D11ShaderResourceView> randomTexSRV,
		uint maxParticles);

	void Reset();

	void Update(float deltaTime,float gameTime);

	void Draw(
		ID3D11DeviceContext* deviceContext,
		std::shared_ptr<SJM::Camera> camera);
};