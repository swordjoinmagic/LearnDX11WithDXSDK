#pragma once

#include "D3DUtils.h"

class Texture2D {
private:
	ComPtr<ID3D11Texture2D> texture;
	
	// ��ͼ����
	ComPtr<ID3D11ShaderResourceView> srv;
	ComPtr<ID3D11RenderTargetView> rtv;
	ComPtr<ID3D11DepthStencilView> dsv;
public:
	// ���ļ��м���,���ݸ�����BindFlag,�Զ�������Ӧ��Դ��ͼ
	// Ĭ�ϴ�����ɫ����Դ��ͼ
	Texture2D(ID3D11Device* device,const std::wstring& path,D3D11_BIND_FLAG bindFlag=D3D11_BIND_SHADER_RESOURCE);
	// �Զ�������
	Texture2D(ID3D11Device* device,D3D11_TEXTURE2D_DESC textureDesc);

	ComPtr<ID3D11ShaderResourceView> CreateShaderResourceView(ID3D11Device* device,D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc);
	ComPtr<ID3D11RenderTargetView> CreateRenderTargetView(ID3D11Device* device,D3D11_RENDER_TARGET_VIEW_DESC rtvDesc);
	ComPtr<ID3D11DepthStencilView> CreateDepthStencilView(ID3D11Device* device,D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc);

	// ��Դ��ͼ��Get����
	ComPtr<ID3D11ShaderResourceView> GetSRV();
	ComPtr<ID3D11RenderTargetView> GetRTV();
	ComPtr<ID3D11DepthStencilView> GetDSV();
};