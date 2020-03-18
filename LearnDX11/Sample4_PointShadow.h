#pragma once

#include "d3dApp.h"

#include "Shader.h"
#include "Mesh.h"
#include "Texture2D.h"
#include "Light.h"

class Sample4 : public D3DApp{
private:
	std::shared_ptr<Texture2D> boxTexture;
	std::shared_ptr<Texture2D> planeTexture;

	// 渲染灯光的Shader
	std::shared_ptr<Shader> renderLightShader;
	std::shared_ptr<Mesh> lightMesh;
	PointLight pointLight;

	// 渲染物体的Shader
	std::shared_ptr<Shader> shader;
	std::shared_ptr<Mesh> boxMesh;

	// 产生阴影图的Shader,一般设PixelShader为空
	std::shared_ptr<Shader> createShadowMapShader;

	// 渲染立方体阴影图的Shader
	std::shared_ptr<Shader> renderCubeMapShader;

	// 立方体深度贴图
	ComPtr<ID3D11Texture2D> cubeDpethTex;
	ComPtr<ID3D11Texture2D> depthTex;
	std::vector<ComPtr<ID3D11DepthStencilView>> depthViews;
	ComPtr<ID3D11ShaderResourceView> depthTexSRV;
	std::vector<ComPtr<ID3D11RenderTargetView>> cubeDepthTexRTVs;
	ComPtr<ID3D11ShaderResourceView> cubeDepthTexSRV;
	
	D3D11_VIEWPORT viewPort;
public:
	Sample4(HINSTANCE hInstance) : D3DApp(hInstance) {}
	void OnStart() override;
	void UpdateScene(float deltaTime) override;
	void Render() override;
	void RenderScene(XMMATRIX& view,XMMATRIX& proj); 
	void RenderSceneWithShadowShader(XMMATRIX& view, XMMATRIX& proj);
};