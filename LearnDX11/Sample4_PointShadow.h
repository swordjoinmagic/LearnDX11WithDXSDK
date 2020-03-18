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

	// ��Ⱦ�ƹ��Shader
	std::shared_ptr<Shader> renderLightShader;
	std::shared_ptr<Mesh> lightMesh;
	PointLight pointLight;

	// ��Ⱦ�����Shader
	std::shared_ptr<Shader> shader;
	std::shared_ptr<Mesh> boxMesh;

	// ������Ӱͼ��Shader,һ����PixelShaderΪ��
	std::shared_ptr<Shader> createShadowMapShader;

	// ��Ⱦ��������Ӱͼ��Shader
	std::shared_ptr<Shader> renderCubeMapShader;

	// �����������ͼ
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