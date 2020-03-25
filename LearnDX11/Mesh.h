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
		Ӧ�ö�Ӧ��Shader�Ը�Mesh���л���
		shader - ���λ���ʹ�õ�Shader
		context - ���Ʋ�����Ҫ�õ�context�������û�������
		primitiveTopology - ���λ���ʹ�õĻ���ģʽ(������ģʽ/����ģʽ��)
		usePass - ���λ���ʹ�õ�Pass,Ϊ-1ʱ,ʹ��ȫ��Pass���л���,��ָ��ʱ,��ʾʹ�þ���ĳ��Pass���л���
				  ����: ʹ�ø�˹ģ��ʱ,Ҫ�ֱ�ָ��pass1��ֱģ����pass2��ֱģ������������ģ��
	*/
	void Draw(std::shared_ptr<Shader> shader, ID3D11DeviceContext* context, D3D_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,int usePass=-1);
	void DrawInstanced(std::shared_ptr<Shader> shader, uint instancedCount, ID3D11DeviceContext* context, D3D_PRIMITIVE_TOPOLOGY primitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, int usePass = -1);
};