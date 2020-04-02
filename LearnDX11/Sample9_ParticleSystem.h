#pragma once

#include "d3dApp.h"
#include "Shader.h"
#include "Texture2D.h"
#include "ParticleSystem.h"

class Sample9 : public D3DApp{
private:
	std::shared_ptr<ParticleSystem> fireParticleSystem;

	std::shared_ptr<Shader> fireControlShader;
	std::shared_ptr<Shader> fireEffectShader;
	
	ComPtr<ID3D11ShaderResourceView> mRandomTexSRV;
	ComPtr<ID3D11ShaderResourceView> mFlareTexSRV;

public:
	Sample9(HINSTANCE hInstance) : D3DApp(hInstance) {}
	
	void OnStart() override;
	void UpdateScene(float deltaTime) override;
	void Render() override;
};