#include "Sample5_Deffered.h"

#include "GeometryGenerator.h"

void Sample5::OnStart() {
	camera = std::make_shared<SJM::Camera>(float3(0, 0, -3), float3(0, 0, 0), AspectRatio());
	camera->Far = 100;

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

	pointLights.resize(100);
	lightVloumes.resize(100);
	// 初始化100个点光源,x = [-5,5],y = [0,6],z = [-5,5]
	for (int i = 0; i < 100;i++) {
		PointLight p;
		p.pos = float3(
			getRandData(-5,5),
			getRandData(0,10),
			getRandData(-5,5)
			);
		p.lightColor = float3(
			getRandData(0,1),
			getRandData(0, 1),
			getRandData(0, 1)
		);
		p.Constant = 1.0;
		p.Linear = 1.0f;
		p.Quadratic = 0.32f;
		
		pointLights[i] = p;
		
		float lightMax = std::fmaxf(std::fmaxf(p.lightColor.x,p.lightColor.y),p.lightColor.z);
		lightVloumes[i] =
			(
				-p.Linear +
				std::sqrtf(p.Linear * p.Linear - 4 * p.Quadratic * (p.Constant - (256.0/5.0) * lightMax))
			) / 
			(2*p.Quadratic);
	}

	pointLights[0].pos = float3(2,5,0);
	pointLights[0].lightColor = float3(1,0,0);
	float lightMax = std::fmaxf(std::fmaxf(pointLights[0].lightColor.x, pointLights[0].lightColor.y), pointLights[0].lightColor.z);
	lightVloumes[0] = (
		-pointLights[0].Linear +
		std::sqrtf(pointLights[0].Linear * pointLights[0].Linear - 4 * pointLights[0].Quadratic * (pointLights[0].Constant - (256.0 / 5.0) * lightMax))
		) /
		(2 * pointLights[0].Quadratic);
	pointLights[1].pos = float3(-2, 5, 0);
	pointLights[1].lightColor = float3(0, 1, 0);
	lightMax = std::fmaxf(std::fmaxf(pointLights[1].lightColor.x, pointLights[1].lightColor.y), pointLights[1].lightColor.z);
	lightVloumes[1] = (
		-pointLights[1].Linear +
		std::sqrtf(pointLights[1].Linear * pointLights[1].Linear - 4 * pointLights[1].Quadratic * (pointLights[1].Constant - (256.0 / 5.0) * lightMax))
		) /
		(2 * pointLights[1].Quadratic);


	// 初始化光源的Mesh
	sphereMesh = GeometryGenerator::CreateSphere(1,32,32);
	sphereMesh->SetUpBuffer(md3dDevice.Get());

	defferdShaderLightVloume = std::make_shared<Shader>(L"Shader/Sample4 Deffered/Compiled/defferedLightVloume.fxo",md3dDevice.Get());

	D3D11_RENDER_TARGET_BLEND_DESC rtBlendDesc;
	rtBlendDesc.BlendEnable = true;
	rtBlendDesc.SrcBlend = D3D11_BLEND_ONE;
	rtBlendDesc.DestBlend = D3D11_BLEND_ONE;
	rtBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtBlendDesc.DestBlendAlpha = D3D11_BLEND_ONE;
	rtBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0] = rtBlendDesc;
	HR(md3dDevice->CreateBlendState(&blendDesc,blendState.GetAddressOf()));

	colorObjectShader = std::make_shared<Shader>(L"Shader/Common/Compiled/ColorObject.fxo",md3dDevice.Get());
	
	CD3D11_DEPTH_STENCIL_DESC depthStencilState = CD3D11_DEPTH_STENCIL_DESC(D3D11_DEFAULT);
	depthStencilState.DepthEnable = false;	
	HR(md3dDevice->CreateDepthStencilState(&depthStencilState,depthState.GetAddressOf()));
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
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
	auto view = camera->GetViewMatrix(); auto proj = camera->GetProjMatrix();

	// 根据GBuffer对平铺四边形进行着色
	//defferedShader->SetFloat3("viewPos",camera->pos);
	//defferedShader->SetRawValue("pointLight",&pointLights[0],sizeof(PointLight)*100);
	//defferedShader->SetShaderResource("gWorldPosTex",gBufferSRVs[0].Get());
	//defferedShader->SetShaderResource("gWorldNormalTex", gBufferSRVs[1].Get());
	//defferedShader->SetShaderResource("gAlbedoGloss", gBufferSRVs[2].Get());
	//quadMesh->Draw(defferedShader,md3dImmediateContext.Get());
	


	// 渲染光源
	// 取消深度测试
	md3dImmediateContext->OMSetDepthStencilState(depthState.Get(), 0xff);
	// 重置深度缓冲区为GBuffer渲染后的深度缓冲区
	md3dImmediateContext->OMSetRenderTargets(1,mRenderTargetView.GetAddressOf(),mDepthStencilView.Get());	
	float blendFactor[4] = {1,1,1,1};
	md3dImmediateContext->OMSetBlendState(blendState.Get(),blendFactor, 0xffffffff);
	defferdShaderLightVloume->SetFloat3("viewPos", camera->pos);
	defferdShaderLightVloume->SetShaderResource("gWorldPosTex",gBufferSRVs[0].Get());
	defferdShaderLightVloume->SetShaderResource("gWorldNormalTex", gBufferSRVs[1].Get());
	defferdShaderLightVloume->SetShaderResource("gAlbedoGloss", gBufferSRVs[2].Get());
	for (uint i = 0; i < 100;i++) {
		auto lightModel = XMMatrixScaling(lightVloumes[i], lightVloumes[i], lightVloumes[i]) * XMMatrixTranslation(pointLights[i].pos.x, pointLights[i].pos.y, pointLights[i].pos.z);
		auto mvp = lightModel * view * proj;
		defferdShaderLightVloume->SetMatrix4x4("mvp",mvp);
		defferdShaderLightVloume->SetRawValue("pointLight",&pointLights[i],sizeof(PointLight));
		sphereMesh->Draw(defferdShaderLightVloume,md3dImmediateContext.Get());

	}
	md3dImmediateContext->PSSetShaderResources(0, 3, null);


	// 重置Blend和深度测试指令
	md3dImmediateContext->OMSetBlendState(NULL,NULL, 0xffffffff);
	md3dImmediateContext->OMSetDepthStencilState(NULL,0);

	//auto mvp = XMMatrixScaling(1,1,1) * view * proj;
	//colorObjectShader->SetMatrix4x4("mvp",mvp);
	//colorObjectShader->SetFloat4("color",float4(1,0,0,1));
	//boxMesh->Draw(colorObjectShader,md3dImmediateContext.Get());
	//colorObjectShader->SetFloat4("color",float4(0,1,0,1));
	//boxMesh->Draw(colorObjectShader, md3dImmediateContext.Get());
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

