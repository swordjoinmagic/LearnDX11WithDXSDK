#pragma once

#include "d3dApp.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture2D.h"
#include "D3DUtils.h"
#include "Light.h"
#include "SpriteRender.h"

class Sample8 : public D3DApp{
private:
	ComPtr<ID3D11Texture2D> shadowMap;
	ComPtr<ID3D11ShaderResourceView> shadowMapSRV;
	ComPtr<ID3D11DepthStencilView> shadowMapDSV;

	std::shared_ptr<Shader> renderShadowMapShader;
	std::shared_ptr<Shader> boxShader;
	std::shared_ptr<Mesh> boxMesh;
	std::shared_ptr<Texture2D> planeTexture;
	std::shared_ptr<Texture2D> boxTexture;

	Light light;

	std::shared_ptr<SpriteRender> spriteRender;

	std::shared_ptr<Shader> whiteShader;
	std::shared_ptr<Mesh> sphereMesh;
public:
	Sample8(HINSTANCE hInstance) : D3DApp(hInstance) {}
	void OnStart() override;
	void UpdateScene(float deltaTime) override;
	void Render() override;
};
