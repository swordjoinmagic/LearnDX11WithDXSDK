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

	// 用于延迟渲染,渲染光球体阶段前面一个阶段的,Stencil Pass的模板状态
	// 状态为：
	//		1. 禁用深度写入
	//		2. 在渲染正面的三角面片时,如果深度测试失败,那么模板缓冲区对应像素位置-1
	//		3. 在渲染背面的三角面片时,如果深度测试失败,那么模板缓冲区对应的像素位置+1
	//		4. 正常渲染光球体,将模板状态改为只有模板缓冲区值不为0时,才对该像素进行渲染

	ComPtr<ID3D11DepthStencilState> stencilPassState;

	// 绘制Light Vloume时的Stencil State,状态为:
	// 1. 开启正面剔除,仅渲染背面
	// 2. 当目标像素模板值不为1时才进行渲染
	// 3. 禁用深度测试
	ComPtr<ID3D11DepthStencilState> lightPassStencilState;

	// 禁用面剔除的光栅化状态
	ComPtr<ID3D11RasterizerState> noCullFaceState;
	// 开启正面剔除的光栅化状态
	ComPtr<ID3D11RasterizerState> cullFrontFaceState;

	// 当前渲染的光源数目
	uint currentLightCount = 100;
public:
	Sample5(HINSTANCE hInstance) :D3DApp(hInstance) {}
	void OnStart() override;
	void Render() override;
	void UpdateScene(float deltaTime) override;
	void RenderGBuffer();
	void CalculateStencilPass();
	void LightPass();
	float getRandData(int min, int max) {
		float m1 = (float)(rand() % 101) / 101;
		min++;
		float m2 = (float)((rand() % (max - min + 1)) + min);
		m2 = m2 - 1;
		return m1 + m2;
	}

};