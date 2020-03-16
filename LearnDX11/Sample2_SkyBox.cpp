#include "Sample2_SkyBox.h"

#include "GeometryGenerator.h"

void Sample2::OnStart() {
	camera = std::make_shared<SJM::Camera>(float3(0, 0, -3), float3(0, 0, 0), AspectRatio());

	// ��ʼ��Mesh
	boxMesh = GeometryGenerator::CreateBox(1,1,1);
	boxMesh->SetUpBuffer(md3dDevice.Get());

	// ��ʼ��Shader
	skyShader = std::make_shared<Shader>(L"Shader/Sample1 CubeMap/Compiled/skyBox.fxo",md3dDevice.Get());
	boxShader = std::make_shared<Shader>(L"Shader/Common/Compiled/textureMap.fxo",md3dDevice.Get());

	// ��ʼ����������ͼ
	cubeMap = std::make_shared<CubeMap>(md3dDevice.Get(),L"Textures/snowcube1024.dds",D3D11_BIND_SHADER_RESOURCE);
	texture = std::make_shared<Texture2D>(md3dDevice.Get(),L"Textures/Brick_Diffuse.JPG",D3D11_BIND_SHADER_RESOURCE);

	// ��ʼ����̬��������ͼ
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 1024;
	texDesc.Height = 1024;
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 6;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	HR(md3dDevice->CreateTexture2D(&texDesc,nullptr,dynamicCubeMap.GetAddressOf()));

	// ��ʼ��RTV��SRV
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.ArraySize = 1;	
	dynamicCubeMapRTVs.resize(6);
	// �������RTV
	for (uint i = 0; i < 6;i++) {
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		HR(md3dDevice->CreateRenderTargetView(dynamicCubeMap.Get(),&rtvDesc,dynamicCubeMapRTVs[i].GetAddressOf()));
	}
	// Ϊ��̬��������ͼ����SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = 1;
	md3dDevice->CreateShaderResourceView(dynamicCubeMap.Get(),&srvDesc,dynamicCubeMapSRV.GetAddressOf());

	// ������Ӧ�������ͼ
	D3D11_TEXTURE2D_DESC depthTexDesc;
	depthTexDesc.Width = texDesc.Width;
	depthTexDesc.Height = texDesc.Height;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;
	HR(md3dDevice->CreateTexture2D(&depthTexDesc,0,depthMap.GetAddressOf()));
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = depthTexDesc.Format;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;;
	dsvDesc.Texture2D.MipSlice = 0;
	HR(md3dDevice->CreateDepthStencilView(depthMap.Get(),&dsvDesc,depthView.GetAddressOf()));

	// ��ʼ���ӿ�
	dynamicViewPort.TopLeftX = 0;
	dynamicViewPort.TopLeftY = 0;
	dynamicViewPort.Width = static_cast<float>(1024);
	dynamicViewPort.Height = static_cast<float>(1024);
	dynamicViewPort.MinDepth = 0.0f;
	dynamicViewPort.MaxDepth = 1.0f;
	
	// ��ʼ��Mesh��SHader
	dynamicCubeMesh = GeometryGenerator::CreateBox(1,1,1);
	dynamicCubeMesh->SetUpBuffer(md3dDevice.Get());
	dynamciCubeMapShader = std::make_shared<Shader>(L"Shader/Sample1 CubeMap/Compiled/cubeMap.fxo",md3dDevice.Get());
}

void Sample2::Render() {

	ID3D11ShaderResourceView* null[] = { nullptr };

	static XMVECTORF32 ups[6] = {
		{{ 0.0f, 1.0f, 0.0f, 0.0f }},	// +X
		{{ 0.0f, 1.0f, 0.0f, 0.0f }},	// -X
		{{ 0.0f, 0.0f, -1.0f, 0.0f }},	// +Y
		{{ 0.0f, 0.0f, 1.0f, 0.0f }},	// -Y
		{{ 0.0f, 1.0f, 0.0f, 0.0f }},	// +Z
		{{ 0.0f, 1.0f, 0.0f, 0.0f }}	// -Z
	};

	static XMVECTORF32 looks[6] = {
		{{ 1.0f, 0.0f, 0.0f, 0.0f }},	// +X
		{{ -1.0f, 0.0f, 0.0f, 0.0f }},	// -X
		{{ 0.0f, 1.0f, 0.0f, 0.0f }},	// +Y
		{{ 0.0f, -1.0f, 0.0f, 0.0f }},	// -Y
		{{ 0.0f, 0.0f, 1.0f, 0.0f }},	// +Z
		{{ 0.0f, 0.0f, -1.0f, 0.0f }},	// -Z
	};	

	if (isFirst) {
		// ������ȾĿ��������������,����Ⱦ���γ���
		for (int i = 0; i < 6; i++) {
			md3dImmediateContext->PSSetShaderResources(0, 1, null);
			md3dImmediateContext->VSSetShaderResources(0, 1, null);

			md3dImmediateContext->OMSetRenderTargets(1, dynamicCubeMapRTVs[i].GetAddressOf(), depthView.Get());
			// �����ɫ������
			md3dImmediateContext->ClearRenderTargetView(dynamicCubeMapRTVs[i].Get(), reinterpret_cast<const float*>(&Colors::Black));
			// �����Ⱥ�ģ�建��
			md3dImmediateContext->ClearDepthStencilView(depthView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
			// �����ӿ�
			md3dImmediateContext->RSSetViewports(1, &dynamicViewPort);

			// ��������view������Ⱦ����
			auto view = XMMatrixLookAtLH(XMVectorSet(0, 0, 0, 1), looks[i].v, ups[i].v);
			auto proj = XMMatrixPerspectiveFovLH(MathF::Radians(90), 1, 0.1, 1000);

			// ��Ⱦ��������������ͼ
			RenderScene(view, proj);
		}
		isFirst = false;
	}


	md3dImmediateContext->PSSetShaderResources(0, 1, null);
	md3dImmediateContext->VSSetShaderResources(0, 1, null);
	// �����ӿ�
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
	// ������ȾĿ��
	md3dImmediateContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());
	// �����ɫ������
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	// �����Ⱥ�ģ�建��
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
	auto view = camera->GetViewMatrix();
	auto proj = camera->GetProjMatrix();
	RenderScene(view,proj);

	// ��Ⱦ����̬��������ͼ��������
	auto model = XMMatrixScaling(3,3,3);
	auto mvp = model * view * proj;
	dynamciCubeMapShader->SetShaderResource("skyBox",dynamicCubeMapSRV.Get());
	dynamciCubeMapShader->SetMatrix4x4("mvp",mvp);
	dynamicCubeMesh->Draw(dynamciCubeMapShader,md3dImmediateContext.Get());	
}

void Sample2::RenderScene(XMMATRIX view, XMMATRIX proj) {
	XMMATRIX model = XMMatrixTranslation(5, 0, 0);

	// ��Ⱦ������
	XMMATRIX boxMvp = model * view * proj;
	boxShader->SetMatrix4x4("mvp", boxMvp);
	boxShader->SetShaderResource("mainTex", texture->GetSRV().Get());
	boxMesh->Draw(boxShader, md3dImmediateContext.Get());

	// ��Ⱦ��պ�
	view.r[3] = XMVectorSet(0, 0, 0, 1);
	XMMATRIX skyBoxMvp = view * proj;
	skyShader->SetMatrix4x4("vp", skyBoxMvp);
	skyShader->SetShaderResource("skyBox", cubeMap->GetSRV().Get());
	boxMesh->Draw(skyShader, md3dImmediateContext.Get());
}