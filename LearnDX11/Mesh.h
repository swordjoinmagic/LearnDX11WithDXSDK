#pragma once

#include "D3DUtils.h"
#include "DefaultVertex.h"
#include "Shader.h"
class Mesh {
private:
	ComPtr<ID3D11Buffer> verticesBuffer;
	ComPtr<ID3D11Buffer> indicesBuffer;
	uint indexCount;
	 
public:
	std::vector<DefaultVertex> vertics;
	std::vector<uint> indices;

	Mesh(const std::vector<DefaultVertex>& vertices, const std::vector<uint>& indices) :vertics(vertices), indices(indices) { indexCount = indices.size(); }
	Mesh(const std::vector<DefaultVertex>& vertics, const std::vector<uint>& indices, ID3D11Device* device);
	virtual ~Mesh();

	void SetUpBuffer(ID3D11Device* device);

	/*
		应用对应的Shader对该Mesh进行绘制
		shader - 本次绘制使用的Shader
		context - 绘制操作需要用到context对象设置环境变量
		primitiveTopology - 本次绘制使用的绘制模式(三角形模式/线形模式等)
		usePass - 本次绘制使用的Pass,为-1时,使用全部Pass进行绘制,当指定时,表示使用具体某个Pass进行绘制
				  举例: 使用高斯模糊时,要分别指定pass1垂直模糊和pass2竖直模糊来进行两次模糊
	*/
	void Draw(std::shared_ptr<Shader> shader, ID3D11DeviceContext* context, D3D_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,int usePass=-1);
	void DrawInstanced(std::shared_ptr<Shader> shader, uint instancedCount, ID3D11DeviceContext* context, D3D_PRIMITIVE_TOPOLOGY primitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, int usePass = -1);
};