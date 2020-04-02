#pragma once

#include "D3DUtils.h"
#include "Shader.h"
#include "Camera.h"

class ParticleSystem {
private:
	// ���������
	uint mMaxParticles;
	// �Ƿ�Ϊ��һ������������ϵͳ
	bool mFirstRun;
	
	// ��ǰ��Ϸʱ��
	float mGameTime;
	// ʱ����
	float mTimeStep;
	float mAge;

	float3 mEyePosW;
	float3 mEmitPosW;
	float3 mEmitDirW;

	// ���ڿ������ӵ�����/���ٵ�Shader
	std::shared_ptr<Shader> particleControlShader;
	// ���ڿ���ÿ�����ӵ��˶��켣��Shader
	std::shared_ptr<Shader> effectShader;

	// ���ڵ�һ����������ϵͳʱ���л��ƵĻ�����
	ComPtr<ID3D11Buffer> mInitVB;

	// ����������,����������׶κ���Ⱦ�׶�
	ComPtr<ID3D11Buffer> mDrawVB;
	ComPtr<ID3D11Buffer> mStreamOutVB;

	// ������ͼ(CPU����)
	ComPtr<ID3D11ShaderResourceView> mRandomTexSRV;
	// ������ͼ����
	ComPtr<ID3D11ShaderResourceView> mTexArraySRV;

	// ���ӽṹ�����벼��
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