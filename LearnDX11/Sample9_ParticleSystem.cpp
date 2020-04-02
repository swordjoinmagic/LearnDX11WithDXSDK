#include "Sample9_ParticleSystem.h"

void Sample9::OnStart() {
	camera = std::make_shared<SJM::Camera>(float3(0, 0, -3), float3(0, 0, 0), AspectRatio());
	camera->Far = 100;

	// 初始化Shader
	fireControlShader = std::make_shared<Shader>(L"Shader/Sample6 ParticleSystem/Fire/Compiled/controlShader.fxo",md3dDevice.Get(),false);
	fireEffectShader = std::make_shared<Shader>(L"Shader/Sample6 ParticleSystem/Fire/Compiled/effectShader.fxo",md3dDevice.Get(),false);
	
	// 初始化SRV
	mRandomTexSRV = d3dHelper::CreateRandomTexture1DSRV(md3dDevice.Get());
	
	std::vector<std::wstring> flares;
	flares.push_back(L"Textures/flare0.dds");
	mFlareTexSRV = d3dHelper::CreateTexture2DArraySRV(
		md3dDevice.Get(),md3dImmediateContext.Get(),flares
	);

	fireParticleSystem = std::make_shared<ParticleSystem>();
	fireParticleSystem->Init(
		md3dDevice.Get(),
		fireControlShader,
		fireEffectShader,
		mFlareTexSRV,
		mRandomTexSRV,
		500
	);
	fireParticleSystem->SetEmitPos(float3(0,0,0));

}

void Sample9::UpdateScene(float deltaTime) {
	fireParticleSystem->Update(deltaTime,timer.TotalTime());
}

void Sample9::Render() {

	fireParticleSystem->SetEyePos(camera->pos);
	fireParticleSystem->Draw(md3dImmediateContext.Get(),camera);
	md3dImmediateContext->OMSetBlendState(0,NULL, 0xffffffff);

}
