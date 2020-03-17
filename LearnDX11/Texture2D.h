#pragma once

#include "D3DUtils.h"

class Texture2D {
private:
	ComPtr<ID3D11Texture2D> texture;
	
	// 视图集合
	ComPtr<ID3D11ShaderResourceView> srv;
	ComPtr<ID3D11RenderTargetView> rtv;
	ComPtr<ID3D11DepthStencilView> dsv;
public:
	// 从文件中加载,根据给定的BindFlag,自动创建对应资源视图
	// 默认创建着色器资源视图
	Texture2D(ID3D11Device* device,const std::wstring& path,D3D11_BIND_FLAG bindFlag=D3D11_BIND_SHADER_RESOURCE);
	// 自定义纹理
	Texture2D(ID3D11Device* device,D3D11_TEXTURE2D_DESC textureDesc);

	ComPtr<ID3D11ShaderResourceView> CreateShaderResourceView(ID3D11Device* device,D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc);
	ComPtr<ID3D11RenderTargetView> CreateRenderTargetView(ID3D11Device* device,D3D11_RENDER_TARGET_VIEW_DESC rtvDesc);
	ComPtr<ID3D11DepthStencilView> CreateDepthStencilView(ID3D11Device* device,D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc);

	// 资源视图的Get方法
	ComPtr<ID3D11ShaderResourceView> GetSRV();
	ComPtr<ID3D11RenderTargetView> GetRTV();
	ComPtr<ID3D11DepthStencilView> GetDSV();
};