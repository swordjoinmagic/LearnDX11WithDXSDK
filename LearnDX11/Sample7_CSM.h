#pragma once

#include "Shader.h"
#include "Mesh.h"
#include "Texture2D.h"
#include "d3dApp.h"
#include "Light.h"
#include "SpriteRender.h"

class Sample7 : public D3DApp{
private:
	struct orthoPorjInfo {
		float right, left;
		float bottom, top;
		float Near, Far;
	};
private:
	ComPtr<ID3D11Texture2D> shadowMap;
	ComPtr<ID3D11DepthStencilView> shadowMapDSV;
	ComPtr<ID3D11ShaderResourceView> shadowMapSRV;

	std::shared_ptr<Texture2D> boxTexture;
	std::shared_ptr<Texture2D> planeTexture;
	std::shared_ptr<Shader> boxShader;
	std::shared_ptr<Mesh> boxMesh;

	std::shared_ptr<Shader> renderShadowMapShader;
	
	// 平行光
	Light light;

	// 主摄像机的八个顶点的观察坐标(以主摄像机为原点)
	float4 cameraConrners[8];

	// 描述光源摄像机的正交投影的参数
	orthoPorjInfo shadowOrthProjInfo;

	// 光源视椎体的八个顶点
	float3 lightCorner[8];

	float3 lcPos;

	std::shared_ptr<SpriteRender> spriteRender;
public:
	Sample7(HINSTANCE hInstance) : D3DApp(hInstance) {}
	void OnStart() override;
	void UpdateScene(float deltaTime) override;
	void Render() override;
	void RenderShadowMap();
	void RenderScene();

	// 计算主摄像机的八个顶点的世界坐标和
	// 八个顶点在光源空间的包围盒
	// 以及光源摄像机的位置
	void CalculateCameraCorners();
};