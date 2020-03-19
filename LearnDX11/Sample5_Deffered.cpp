#include "Sample5_Deffered.h"

#include "GeometryGenerator.h"

void Sample5::OnStart() {
	camera = std::make_shared<SJM::Camera>(float3(0, 0, -3), float3(0, 0, 0), AspectRatio());

	textureWidth = mClientWidth;
	textureHeight = mClientHeight;
	bufferCount = 3;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = mClientWidth;
	texDesc.Height = mClientHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	gBuffers.resize(bufferCount);
	gBufferRTVs.resize(bufferCount);
	gBufferSRVs.resize(bufferCount);
	for (uint i = 0; i < bufferCount; i++) {
		md3dDevice->CreateTexture2D(&texDesc, 0, gBuffers[i].GetAddressOf());
	}	

	// 逐个创建渲染目标视图
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	for (uint i = 0; i < bufferCount;i++) {
		HR(md3dDevice->CreateRenderTargetView(gBuffers[i].Get(),&rtvDesc,gBufferRTVs[i].GetAddressOf()));
	}

	// 逐个创建着色器资源视图
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	for (uint i = 0; i < bufferCount; i++) {
		md3dDevice->CreateShaderResourceView(gBuffers[i].Get(),&srvDesc,gBufferSRVs[i].GetAddressOf());
	}

	// 创建和GBuffer相对应的深度缓冲
	D3D11_TEXTURE2D_DESC depthTexDesc;
	depthTexDesc.Width = textureWidth;
	depthTexDesc.Height = textureHeight;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;
	HR(md3dDevice->CreateTexture2D(&depthTexDesc,0,depthTex.GetAddressOf()));

	// 创建深度资源视图
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;	
	dsvDesc.Format = depthTexDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	HR(md3dDevice->CreateDepthStencilView(depthTex.Get(),&dsvDesc,depthTexDSV.GetAddressOf()));

	// 初始化视口
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = static_cast<float>(textureWidth);
	viewPort.Height = static_cast<float>(textureHeight);
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	// 初始化Mesh和Shader
	boxMesh = GeometryGenerator::CreateBox(1,1,1);
	boxMesh->SetUpBuffer(md3dDevice.Get());
	defferedShader = std::make_shared<Shader>(L"Shader/Sample4 Deffered/Compiled/defferedWithoutLightVloume.fxo",md3dDevice.Get());

	renderGBufferShader = std::make_shared<Shader>(L"Shader/Sample4 Deffered/Compiled/renderGBuffer.fxo",md3dDevice.Get());

	// 初始化点光源
	pointLight.pos = float3(2, 5, 0);
	pointLight.lightColor = float3(1, 1, 1);
	pointLight.Constant = 1.0;
	pointLight.Linear = 0.09f;
	pointLight.Quadratic = 0.032f;

	// 初始化纹理
	planeTexture = std::make_shared<Texture2D>(md3dDevice.Get(), L"Textures/Brick_Diffuse.JPG");
	boxTexture = std::make_shared<Texture2D>(md3dDevice.Get(), L"Textures/Crate_Diffuse.jpg");

	quadMesh = GeometryGenerator::CreateQuad();
	quadMesh->SetUpBuffer(md3dDevice.Get());

	textureMapShader = std::make_shared<Shader>(L"Shader/Common/Compiled/textureMap.fxo",md3dDevice.Get());
	whiteObjectShader = std::make_shared<Shader>(L"Shader/Common/Compiled/WhiteObject.fxo", md3dDevice.Get());
}

void Sample5::UpdateScene(float deltaTime) {

}

void Sample5::Render() {
	// 渲染GBuffer
	RenderGBuffer();

	ID3D11ShaderResourceView* null[] = {nullptr,nullptr,nullptr};

	// 重置渲染目标/视口,并对颜色和深度缓冲区进行清除
	md3dImmediateContext->OMSetRenderTargets(1,mRenderTargetView.GetAddressOf(),mDepthStencilView.Get());
	md3dImmediateContext->RSSetViewports(1,&mScreenViewport);
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
	auto view = camera->GetViewMatrix(); auto proj = camera->GetProjMatrix();

	//auto model = XMMatrixScaling(1, 1, 1) * XMMatrixTranslation(0,3,0);
	//auto mvp = model * view * proj;
	//whiteObjectShader->SetMatrix4x4("mvp", mvp);
	//boxMesh->Draw(whiteObjectShader, md3dImmediateContext.Get());

	//std::vector<float3> quadPositions = {
	//	{-5,3,0},
	//	{-10,3,0},
	//	{-15,3,0}
	//};
	//for (uint i = 0; i < quadPositions.size(); i++) {
	//	// 显示GBuffer进行测试
	//	auto quadModel = XMMatrixScaling(3,3,3) * XMMatrixTranslation(quadPositions[i].x, quadPositions[i].y, quadPositions[i].z);
	//	auto mvp = quadModel * view * proj;
	//	textureMapShader->SetMatrix4x4("mvp", mvp);
	//	textureMapShader->SetShaderResource("mainTex", gBufferSRVs[i].Get());
	//	quadMesh->Draw(textureMapShader,md3dImmediateContext.Get());
	//}


	// 根据GBuffer对平铺四边形进行着色
	defferedShader->SetFloat3("viewPos",camera->pos);
	defferedShader->SetRawValue("pointLight",&pointLight,sizeof(PointLight));
	defferedShader->SetShaderResource("gWorldPosTex",gBufferSRVs[0].Get());
	defferedShader->SetShaderResource("gWorldNormalTex", gBufferSRVs[1].Get());
	defferedShader->SetShaderResource("gAlbedoGloss", gBufferSRVs[2].Get());
	quadMesh->Draw(defferedShader,md3dImmediateContext.Get());

	md3dImmediateContext->PSSetShaderResources(0,3,null);
}

void Sample5::RenderGBuffer() {
	// 设置渲染目标
	md3dImmediateContext->OMSetRenderTargets(bufferCount,gBufferRTVs[0].GetAddressOf(),depthTexDSV.Get());
	// 设置视口
	md3dImmediateContext->RSSetViewports(1,&viewPort);
	// 清除颜色缓冲和深度缓冲
	for (uint i = 0; i < bufferCount; i++) {
		md3dImmediateContext->ClearRenderTargetView(gBufferRTVs[i].Get(), reinterpret_cast<const float*>(&Colors::Black));
	}
	md3dImmediateContext->ClearDepthStencilView(depthTexDSV.Get(), D3D11_CLEAR_DEPTH, 1.0, 0);

	auto view = camera->GetViewMatrix();
	auto proj = camera->GetProjMatrix();

	auto planeModel = XMMatrixScaling(20, 0.1, 20);
	auto transInvModel = XMMatrixTranspose(GetInverseMatrix(planeModel));
	auto mvp = planeModel * view * proj;
	renderGBufferShader->SetMatrix4x4("mvp",mvp);
	renderGBufferShader->SetMatrix4x4("model",planeModel);
	renderGBufferShader->SetMatrix4x4("transInvModel",transInvModel);
	renderGBufferShader->SetFloat("gloss",256);
	renderGBufferShader->SetTexture2D("mainTex",planeTexture);
	boxMesh->Draw(renderGBufferShader,md3dImmediateContext.Get());

	auto boxModel = XMMatrixTranslation(0,3,0);
	transInvModel = XMMatrixTranspose(GetInverseMatrix(boxModel));
	mvp = boxModel * view * proj;
	renderGBufferShader->SetMatrix4x4("mvp", mvp);
	renderGBufferShader->SetMatrix4x4("model", boxModel);
	renderGBufferShader->SetMatrix4x4("transInvModel", transInvModel);
	renderGBufferShader->SetFloat("gloss", 32);
	renderGBufferShader->SetTexture2D("mainTex",boxTexture);
	boxMesh->Draw(renderGBufferShader,md3dImmediateContext.Get());
}

