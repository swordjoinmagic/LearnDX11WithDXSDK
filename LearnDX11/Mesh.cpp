#include "Mesh.h"

Mesh::~Mesh() {}
Mesh::Mesh(const std::vector<DefaultVertex>& vertics, const std::vector<uint>& indices, ID3D11Device* device) :vertics(vertics), indices(indices) {
	HR(CreateBuffer(
		D3D11_USAGE_IMMUTABLE,
		sizeof(DefaultVertex)*vertics.size(),
		D3D11_BIND_VERTEX_BUFFER,
		0, 0,
		sizeof(DefaultVertex),
		&vertics[0],
		device,
		verticesBuffer.GetAddressOf()
	));
	HR(CreateBuffer(
		D3D11_USAGE_IMMUTABLE,
		sizeof(uint)*indices.size(),
		D3D11_BIND_INDEX_BUFFER,
		0, 0,
		sizeof(uint),
		&indices[0],
		device,
		indicesBuffer.GetAddressOf()
	));

	indexCount = indices.size();
}

void Mesh::SetUpBuffer(ID3D11Device* device) {
	HR(CreateBuffer(
		D3D11_USAGE_IMMUTABLE,
		sizeof(DefaultVertex)*vertics.size(),
		D3D11_BIND_VERTEX_BUFFER,
		0, 0,
		sizeof(DefaultVertex),
		&vertics[0],
		device,
		verticesBuffer.GetAddressOf()
	));
	HR(CreateBuffer(
		D3D11_USAGE_IMMUTABLE,
		sizeof(uint)*indices.size(),
		D3D11_BIND_INDEX_BUFFER,
		0, 0,
		sizeof(uint),
		&indices[0],
		device,
		indicesBuffer.GetAddressOf()
	));
}

void Mesh::Draw(std::shared_ptr<Shader> shader, ID3D11DeviceContext* context, D3D_PRIMITIVE_TOPOLOGY primitiveTopology, int usePass) {
	// ���û���ģʽ
	context->IASetPrimitiveTopology(primitiveTopology);
	// ���ö������������
	uint stride = sizeof(DefaultVertex);
	uint offset = 0;
	context->IASetVertexBuffers(0, 1, verticesBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(indicesBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	if (usePass != -1) {
		// ʹ��ĳ�������Pass���л���
		shader->UsePass(usePass, context);
		context->DrawIndexed(indexCount, 0, 0);
	} else {
		// ʹ��ȫ��Pass��һ���Ƹ�Mesh(����: �������ʱ,pass1���ƺڱ�,pass2��������)
		for (uint i = 0; i < shader->GetPassCount(); i++) {
			shader->UsePass(i,context);
			context->DrawIndexed(indexCount,0,0);
		}
	}
}

void Mesh::DrawInstanced(std::shared_ptr<Shader> shader, uint instancedCount, ID3D11DeviceContext* context, D3D_PRIMITIVE_TOPOLOGY primitive, int usePass) {
	// ���û���ģʽ
	context->IASetPrimitiveTopology(primitive);
	// ���ö������������
	uint stride = sizeof(DefaultVertex);
	uint offset = 0;
	context->IASetVertexBuffers(0, 1, verticesBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(indicesBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	if (usePass != -1) {
		// ʹ��ĳ�������Pass���л���
		shader->UsePass(usePass, context);
		context->DrawIndexedInstanced(indexCount, instancedCount, 0, 0, 0);
	} else {
		// ʹ��ȫ��Pass��һ���Ƹ�Mesh(����: �������ʱ,pass1���ƺڱ�,pass2��������)
		for (uint i = 0; i < shader->GetPassCount(); i++) {
			shader->UsePass(i, context);
			context->DrawIndexedInstanced(indexCount, instancedCount, 0, 0, 0);
		}
	}
}