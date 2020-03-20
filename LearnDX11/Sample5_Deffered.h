#pragma once

#include "d3dApp.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture2D.h"
#include "Light.h"

class Sample5 : public D3DApp {
private:

	// GBuffer纹理的宽高
	uint textureWidth, textureHeight;
	// GBuffer的数量
	uint bufferCount;

	// 存储物体的
	// 世界坐标/法线/漫反射颜色分量和高光反射光滑度(第四个分量)
	// 的GBuffer集合
	std::vector<ComPtr<ID3D11Texture2D>> gBuffers;
	// gBuffer的RTV集合,顺序为pos,normal,albedo
	std::vector<ComPtr<ID3D11RenderTargetView>> gBufferRTVs;
	std::vector<ComPtr<ID3D11ShaderResourceView>> gBufferSRVs;

	// 渲染GBuffer所需的深度贴图
	ComPtr<ID3D11Texture2D> depthTex;
	ComPtr<ID3D11DepthStencilView> depthTexDSV;

	// 渲染GBuffer使用的视口
	D3D11_VIEWPORT viewPort;

	std::shared_ptr<Mesh> boxMesh;
	std::shared_ptr<Shader> defferedShader;
	std::shared_ptr<Shader> renderGBufferShader;

	std::shared_ptr<Texture2D> boxTexture;
	std::shared_ptr<Texture2D> planeTexture;
	
	std::shared_ptr<Mesh> quadMesh;
	std::shared_ptr<Shader> textureMapShader;
	std::shared_ptr<Shader> whiteObjectShader;

	std::shared_ptr<Mesh> sphereMesh;

	PointLight pointLight;

	std::vector<PointLight> pointLights;
	std::vector<float> lightVloumes;
	const uint pointLightCount = 100;
	std::shared_ptr<Shader> defferdShaderLightVloume;

	ComPtr<ID3D11BlendState> blendState;

	std::shared_ptr<Shader> colorObjectShader;

	ComPtr<ID3D11DepthStencilState> depthState;



public:
	Sample5(HINSTANCE hInstance) :D3DApp(hInstance) {}
	void OnStart() override;
	void Render() override;
	void UpdateScene(float deltaTime) override;
	void RenderGBuffer();
	float getRandData(int min, int max) {
		float m1 = (float)(rand() % 101) / 101;
		min++;
		float m2 = (float)((rand() % (max - min + 1)) + min);
		m2 = m2 - 1;
		return m1 + m2;
	}

};