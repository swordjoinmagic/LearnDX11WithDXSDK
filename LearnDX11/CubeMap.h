#pragma once

#include "D3DUtils.h"
#include "Texture2D.h"

class CubeMap {
private:
	ComPtr<ID3D11Texture2D> textureCube;

	// ��ͼ����
	ComPtr<ID3D11ShaderResourceView> srv;	
	ComPtr<ID3D11DepthStencilView> dsv;
public:
	CubeMap(ID3D11Device* device,ID3D11DeviceContext* deviceContex, const std::vector<std::wstring>& imagePaths,D3D11_BIND_FLAG bindFlag);
	CubeMap(ID3D11Device* device,const std::wstring& ddsPath,D3D11_BIND_FLAG bindFlag);

	// ��Դ��ͼ��Get����
	ComPtr<ID3D11ShaderResourceView> GetSRV();
	ComPtr<ID3D11DepthStencilView> GetDSV();
};