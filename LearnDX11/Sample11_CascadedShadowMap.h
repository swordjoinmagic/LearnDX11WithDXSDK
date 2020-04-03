#pragma once

#include "d3dApp.h"
#include "Shader.h"
#include "Texture2D.h"
#include "Mesh.h"
#include "Light.h"
#include "SpriteRender.h"

class Sample11 : public D3DApp{
private:
	// 级联阴影分割数组,0号元素为主摄像机的near,最后一个元素为主摄像机的Far
	std::vector<float> m_cascadeEnd;

	// 级联分割数量,暂定为分割为三个部分
	uint cascadedSize = 3;

	// 级联阴影图数组
	ComPtr<ID3D11Texture2D> cascadedShadowTextureArray;
	std::vector<ComPtr<ID3D11DepthStencilView>> cascadedShadowDSV;
	// 级联阴影图SRV
	ComPtr<ID3D11ShaderResourceView> cascadedShadowSRV;

	// 用于显示到屏幕上的SRV
	std::vector<ComPtr<ID3D11ShaderResourceView>> srvs;

	std::shared_ptr<Texture2D> boxTexture;
	std::shared_ptr<Texture2D> planeTexture;
	std::shared_ptr<Shader> boxShader;
	std::shared_ptr<Mesh> boxMesh;

	std::shared_ptr<Shader> oldBoxShader; 

	std::shared_ptr<Shader> renderShadowMapShader;

	// 平行光
	Light light;

	std::shared_ptr<SpriteRender> spriteRender;

	ComPtr<ID3D11RasterizerState> cullFrontFaceState;

	uint textureSize = 1024;
	D3D11_VIEWPORT renderShadowMapViewPort;

	std::vector<float3> boxPositions;

	std::vector<XMMATRIX> lightVPMatrixs;
	std::vector<float3> cameraPoes;
public:
	Sample11(HINSTANCE hInstance) : D3DApp(hInstance) {}
	void OnStart() override;
	void UpdateScene(float deltaTime) override;
	void Render() override;
	void RenderShadowMap(XMMATRIX lightVPMatrix);
	void RenderScene();
	
	/*
		根据near和far这两个参数(和当前主摄像机的fov属性),
		计算当前能覆盖可观察空间的包围盒,并计算光源的正交摄像机此时应该处于的位置,
		返回从世界坐标变换到光源空间的VP矩阵
	*/
	XMMATRIX CalculateCameraCorners(
		float nearPlane,float farPlane,
		float3* camearWorldPos
	);

};