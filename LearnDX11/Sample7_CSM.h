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
	
	// ƽ�й�
	Light light;

	// ��������İ˸�����Ĺ۲�����(���������Ϊԭ��)
	float4 cameraConrners[8];

	// ������Դ�����������ͶӰ�Ĳ���
	orthoPorjInfo shadowOrthProjInfo;

	// ��Դ��׵��İ˸�����
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

	// ������������İ˸���������������
	// �˸������ڹ�Դ�ռ�İ�Χ��
	// �Լ���Դ�������λ��
	void CalculateCameraCorners();
};