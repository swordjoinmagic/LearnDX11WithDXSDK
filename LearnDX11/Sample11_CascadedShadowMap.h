#pragma once

#include "d3dApp.h"
#include "Shader.h"
#include "Texture2D.h"
#include "Mesh.h"
#include "Light.h"
#include "SpriteRender.h"

class Sample11 : public D3DApp{
private:
	// ������Ӱ�ָ�����,0��Ԫ��Ϊ���������near,���һ��Ԫ��Ϊ���������Far
	std::vector<float> m_cascadeEnd;

	// �����ָ�����,�ݶ�Ϊ�ָ�Ϊ��������
	uint cascadedSize = 3;

	// ������Ӱͼ����
	ComPtr<ID3D11Texture2D> cascadedShadowTextureArray;
	std::vector<ComPtr<ID3D11DepthStencilView>> cascadedShadowDSV;
	// ������ӰͼSRV
	ComPtr<ID3D11ShaderResourceView> cascadedShadowSRV;

	// ������ʾ����Ļ�ϵ�SRV
	std::vector<ComPtr<ID3D11ShaderResourceView>> srvs;

	std::shared_ptr<Texture2D> boxTexture;
	std::shared_ptr<Texture2D> planeTexture;
	std::shared_ptr<Shader> boxShader;
	std::shared_ptr<Mesh> boxMesh;

	std::shared_ptr<Shader> oldBoxShader; 

	std::shared_ptr<Shader> renderShadowMapShader;

	// ƽ�й�
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
		����near��far����������(�͵�ǰ���������fov����),
		���㵱ǰ�ܸ��ǿɹ۲�ռ�İ�Χ��,�������Դ�������������ʱӦ�ô��ڵ�λ��,
		���ش���������任����Դ�ռ��VP����
	*/
	XMMATRIX CalculateCameraCorners(
		float nearPlane,float farPlane,
		float3* camearWorldPos
	);

};