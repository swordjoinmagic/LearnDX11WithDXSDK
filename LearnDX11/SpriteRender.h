#pragma once

#include "D3DUtils.h"
#include "Texture2D.h"
#include "Shader.h"
#include "Mesh.h"

class SpriteRender {
private:
	std::shared_ptr<Shader> spriteShader;
	std::shared_ptr<Mesh> spriteMesh;

	float screenWidth,screenHeight;

	ComPtr<ID3D11DepthStencilState> noDepthTest;
public:
	SpriteRender(ComPtr<ID3D11Device> d3dDevice,float screenWidth,float screenHeight);
	void DrawSprite(
		ID3D11DeviceContext* deviceContext,
		ID3D11ShaderResourceView* textureSRV,
		float3 pos,
		float2 size = float2(10,10),
		float rotation = 0,
		float3 color = float3(1,1,1)
	);
};