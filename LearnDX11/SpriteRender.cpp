#include "SpriteRender.h"
#include "MathF.h"

SpriteRender::SpriteRender(ComPtr<ID3D11Device> d3dDevice, float screenWidth, float screenHeight) {
	std::vector<DefaultVertex> vertices;
	vertices.resize(6);
	vertices[0].pos = float3(0, 1, 0);
	vertices[0].texcoord = float2(0,1);

	vertices[1].pos = float3(1, 0, 0);
	vertices[1].texcoord = float2(1, 0);

	vertices[2].pos = float3(0, 0, 0);
	vertices[2].texcoord = float2(0, 0);

	vertices[3].pos = float3(0, 1, 0);
	vertices[3].texcoord = float2(0, 1);

	vertices[4].pos = float3(1, 1, 0);
	vertices[4].texcoord = float2(1, 1);

	vertices[5].pos = float3(1, 0, 0);
	vertices[5].texcoord = float2(1, 0);

	std::vector<uint> indices;
	indices.resize(6);
	for (uint i = 0; i < 6; i++) indices[i] = i;
	
	spriteMesh = std::make_shared<Mesh>(vertices,indices);
	spriteMesh->SetUpBuffer(d3dDevice.Get());

	spriteShader = std::make_shared<Shader>(L"Shader/Common/Compiled/textureMap.fxo", d3dDevice.Get());

	this->screenHeight = screenHeight;
	this->screenWidth = screenWidth;

	CD3D11_DEPTH_STENCIL_DESC depthStencilState = CD3D11_DEPTH_STENCIL_DESC(D3D11_DEFAULT);
	depthStencilState.DepthEnable = false;
	HR(d3dDevice->CreateDepthStencilState(&depthStencilState,noDepthTest.GetAddressOf()));
}

void SpriteRender::DrawSprite(
	ID3D11DeviceContext* deviceContext,
	ID3D11ShaderResourceView* textureSRV,
	float3 pos,
	float2 size,
	float rotation,
	float3 color
) {
	// 关闭深度测试
	deviceContext->OMSetDepthStencilState(noDepthTest.Get(),0);

	auto model = XMMatrixScaling(size.x,size.y,1) * XMMatrixRotationZ(MathF::Radians(rotation)) * XMMatrixTranslation(pos.x, pos.y, pos.z);
	auto proj = XMMatrixOrthographicLH(screenWidth,screenHeight,-1,1);
	auto mp = model * proj;
	spriteShader->SetMatrix4x4("mvp",mp);
	spriteShader->SetShaderResource("mainTex",textureSRV);
	spriteMesh->Draw(spriteShader,deviceContext);

	// 重置深度测试状态
	deviceContext->OMSetDepthStencilState(NULL,0);
}