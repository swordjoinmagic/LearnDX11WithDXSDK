#pragma once

#include "D3DUtils.h"
#include "Texture2D.h"

class CubeMap {
private:
	ComPtr<ID3D11Texture2D> textureCube;

	// 视图集合
	ComPtr<ID3D11ShaderResourceView> srv;	
	ComPtr<ID3D11DepthStencilView> dsv;
public:
	CubeMap(ID3D11Device* device,ID3D11DeviceContext* deviceContex, const std::vector<std::wstring>& imagePaths,D3D11_BIND_FLAG bindFlag);
	CubeMap(ID3D11Device* device,const std::wstring& ddsPath,D3D11_BIND_FLAG bindFlag);

	// 资源视图的Get方法
	ComPtr<ID3D11ShaderResourceView> GetSRV();
	ComPtr<ID3D11DepthStencilView> GetDSV();
};