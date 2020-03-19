#pragma once

#include "d3dApp.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture2D.h"
#include "Light.h"

class Sample5 : public D3DApp {
private:

	// GBuffer����Ŀ��
	uint textureWidth, textureHeight;
	// GBuffer������
	uint bufferCount;

	// �洢�����
	// ��������/����/��������ɫ�����͸߹ⷴ��⻬��(���ĸ�����)
	// ��GBuffer����
	std::vector<ComPtr<ID3D11Texture2D>> gBuffers;
	// gBuffer��RTV����,˳��Ϊpos,normal,albedo
	std::vector<ComPtr<ID3D11RenderTargetView>> gBufferRTVs;
	std::vector<ComPtr<ID3D11ShaderResourceView>> gBufferSRVs;

	// ��ȾGBuffer����������ͼ
	ComPtr<ID3D11Texture2D> depthTex;
	ComPtr<ID3D11DepthStencilView> depthTexDSV;

	// ��ȾGBufferʹ�õ��ӿ�
	D3D11_VIEWPORT viewPort;

	std::shared_ptr<Mesh> boxMesh;
	std::shared_ptr<Shader> defferedShader;
	std::shared_ptr<Shader> renderGBufferShader;

	std::shared_ptr<Texture2D> boxTexture;
	std::shared_ptr<Texture2D> planeTexture;
	
	std::shared_ptr<Mesh> quadMesh;
	std::shared_ptr<Shader> textureMapShader;
	std::shared_ptr<Shader> whiteObjectShader;

	PointLight pointLight;

public:
	Sample5(HINSTANCE hInstance) :D3DApp(hInstance) {}
	void OnStart() override;
	void Render() override;
	void UpdateScene(float deltaTime) override;
	void RenderGBuffer();
};