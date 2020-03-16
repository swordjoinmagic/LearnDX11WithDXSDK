#include "CubeMap.h"

CubeMap::CubeMap(ID3D11Device* device, ID3D11DeviceContext* deviceContex, const std::vector<std::wstring>& imagePaths, D3D11_BIND_FLAG bindFlag) {

	// ���ڱ������ɵ�6��Texture2D����
	std::vector<ComPtr<ID3D11Texture2D>> texArray(6,nullptr);
	// ����������6��������������
	std::vector<D3D11_TEXTURE2D_DESC> texDescArray(6);

	// �����ļ���ַ���δ���Texture����
	for (uint i = 0; i < 6;i++) {
		HR(D3DX11CreateTextureFromFile(
			device,
			imagePaths[i].c_str(),
			nullptr, nullptr,
			(ID3D11Resource**)texArray[i].GetAddressOf(),
			nullptr));		
		// �������,���ں������CubeMap������
		// (��Ϊ����������������ĸ�ʽ����һ����,������ߵ�)
		texArray[i]->GetDesc(&texDescArray[i]);
	}

	// �����������������
	D3D11_TEXTURE2D_DESC cubeMapDesc;
	cubeMapDesc.Width = texDescArray[0].Width;
	cubeMapDesc.Height = texDescArray[0].Height;
	cubeMapDesc.MipLevels = 1;	// mipmapĬ�ϼ���Ϊ1
	cubeMapDesc.ArraySize = 6;
	cubeMapDesc.Format = texDescArray[0].Format;
	cubeMapDesc.SampleDesc.Count = 1;
	cubeMapDesc.SampleDesc.Quality = 0;
	cubeMapDesc.Usage = D3D11_USAGE_DEFAULT;
	cubeMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	cubeMapDesc.CPUAccessFlags = 0;
	cubeMapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	// ����CubeMap�������
	HR(device->CreateTexture2D(&cubeMapDesc,nullptr,textureCube.GetAddressOf()));

	// ���������������һ��ֵ��ȥ(�������ǵ�mipMap����Դ)
	for (uint i = 0; i < 6;i++) {
		for (uint j = 0; j < cubeMapDesc.MipLevels;j++) {
			deviceContex->CopySubresourceRegion(
				textureCube.Get(),
				D3D11CalcSubresource(j,i,cubeMapDesc.MipLevels),
				0,0,0,
				texArray[i].Get(),
				j,
				nullptr
			);
		}
	}

	// ����srv
	HR(device->CreateShaderResourceView(textureCube.Get(),0,srv.GetAddressOf()));

	if ((bindFlag | D3D11_BIND_DEPTH_STENCIL) == bindFlag) {
		// Ϊ��������������DSV
		HR(device->CreateDepthStencilView(textureCube.Get(),0,dsv.GetAddressOf()));
	}

	
}
CubeMap::CubeMap(ID3D11Device* device, const std::wstring& ddsPath, D3D11_BIND_FLAG bindFlag) {

	// ����DDS�ļ���������������
	//HR(CreateDDSTextureFromFile(device, ddsPath.c_str(), (ID3D11Resource**)textureCube.GetAddressOf(), srv.GetAddressOf()));
	HR(D3DX11CreateTextureFromFile(device,ddsPath.c_str(),0,0,(ID3D11Resource**)textureCube.GetAddressOf(),0));
	HR(device->CreateShaderResourceView(textureCube.Get(),0,srv.GetAddressOf()));

	if ((bindFlag | D3D11_BIND_DEPTH_STENCIL) == bindFlag) {
		// Ϊ��������������DSV
		HR(device->CreateDepthStencilView(textureCube.Get(), 0, dsv.GetAddressOf()));
	}
}


ComPtr<ID3D11ShaderResourceView> CubeMap::GetSRV() {
	return this->srv;
}

ComPtr<ID3D11DepthStencilView> CubeMap::GetDSV() {
	return this->dsv;
}