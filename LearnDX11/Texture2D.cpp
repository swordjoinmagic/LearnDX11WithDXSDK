#include "Texture2D.h"

Texture2D::Texture2D(ID3D11Device* device,const std::wstring& path, D3D11_BIND_FLAG bindFlag) {					
	// 从WIC格式文件中创建Texture2D
	HR(D3DX11CreateTextureFromFile(device,
		path.c_str(),
		nullptr, nullptr,
		(ID3D11Resource**)texture.GetAddressOf(),
		nullptr));

	// 初始化着色器资源视图
	HR(device->CreateShaderResourceView(texture.Get(),0,srv.GetAddressOf()));

	// 根据BindFlag创建对应的视图
	if ( (bindFlag | D3D11_BIND_RENDER_TARGET) == bindFlag ) {
		// 创建渲染目标视图
		HR(device->CreateRenderTargetView(texture.Get(),0,rtv.GetAddressOf()));
	}
	if ((bindFlag | D3D11_BIND_DEPTH_STENCIL) == bindFlag) {
		// 创建深度/模板缓冲视图
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = 0;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		HR(device->CreateDepthStencilView(texture.Get(),&dsvDesc,dsv.GetAddressOf()));
	}
	
}

Texture2D::Texture2D(ID3D11Device* device, D3D11_TEXTURE2D_DESC textureDesc) {
	HR(device->CreateTexture2D(&textureDesc,nullptr,texture.GetAddressOf()));
}

ComPtr<ID3D11ShaderResourceView> Texture2D::CreateShaderResourceView(ID3D11Device* device, D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc) {
	HR(device->CreateShaderResourceView(texture.Get(),&srvDesc,srv.GetAddressOf()));
	return srv;
}
ComPtr<ID3D11RenderTargetView> Texture2D::CreateRenderTargetView(ID3D11Device* device, D3D11_RENDER_TARGET_VIEW_DESC rtvDesc) {
	HR(device->CreateRenderTargetView(texture.Get(),&rtvDesc,rtv.GetAddressOf()));
	return rtv;
}
ComPtr<ID3D11DepthStencilView> Texture2D::CreateDepthStencilView(ID3D11Device* device, D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc) {	
	HR(device->CreateDepthStencilView(texture.Get(),&dsvDesc,dsv.GetAddressOf()));
	return dsv;
}

ComPtr<ID3D11ShaderResourceView> Texture2D::GetSRV() {
	if (srv == nullptr) throw;
	return srv;
}
ComPtr<ID3D11RenderTargetView> Texture2D::GetRTV() {
	if (rtv == nullptr) throw;
	return rtv;
}
ComPtr<ID3D11DepthStencilView> Texture2D::GetDSV() {
	if (dsv == nullptr) throw;
	return dsv;
}