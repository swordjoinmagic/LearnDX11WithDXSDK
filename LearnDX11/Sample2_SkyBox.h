#pragma once

#include "d3dApp.h"
#include "Mesh.h"
#include "Shader.h"

#include "Texture2D.h"
#include "CubeMap.h"

class Sample2 : public D3DApp{
private:
	std::shared_ptr<Mesh> boxMesh;
	std::shared_ptr<Shader> skyShader;
	std::shared_ptr<Shader> boxShader;

	std::shared_ptr<CubeMap> cubeMap;
	std::shared_ptr<Texture2D> texture;

	// 动态立方体天空盒
	ComPtr<ID3D11Texture2D> dynamicCubeMap;
	ComPtr<ID3D11Texture2D> depthMap;
	ComPtr<ID3D11DepthStencilView> depthView;
	std::vector<ComPtr<ID3D11RenderTargetView>> dynamicCubeMapRTVs;
	ComPtr<ID3D11ShaderResourceView> dynamicCubeMapSRV;
	std::shared_ptr<Mesh> dynamicCubeMesh;
	std::shared_ptr<Shader> dynamciCubeMapShader;
	D3D11_VIEWPORT dynamicViewPort;

	bool isFirst = true;
public:
	Sample2(HINSTANCE hInstance) : D3DApp(hInstance) {}
	void OnStart() override;
	void Render() override;
	void RenderScene(XMMATRIX view,XMMATRIX proj);
};