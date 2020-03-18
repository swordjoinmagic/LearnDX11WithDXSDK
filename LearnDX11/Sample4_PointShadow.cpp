#include "Sample4_PointShadow.h"

#include "GeometryGenerator.h"

void Sample4::OnStart() {
	camera = std::make_shared<SJM::Camera>(float3(0, 3, -3), float3(0, 0, 0), AspectRatio());

	// 初始化光源的Shader和Mehs
	lightMesh = GeometryGenerator::CreateSphere(0.2,32,32);
	lightMesh->SetUpBuffer(md3dDevice.Get());
	renderLightShader = std::make_shared<Shader>(L"Shader/Common/Compiled/WhiteObject.fxo",md3dDevice.Get());

	// 初始化渲染物体的Shader和物体Mesh
	boxMesh = GeometryGenerator::CreateBox(1,1,1);
	boxMesh->SetUpBuffer(md3dDevice.Get());
	shader = std::make_shared<Shader>(L"Shader/Sample3 ShadowMap/Compiled/shadow.fxo",md3dDevice.Get());
	
	// 初始化纹理
	planeTexture = std::make_shared<Texture2D>(md3dDevice.Get(), L"Textures/Brick_Diffuse.JPG");
	boxTexture = std::make_shared<Texture2D>(md3dDevice.Get(), L"Textures/Crate_Diffuse.jpg");

	// 初始化产生阴影图的Shader
	createShadowMapShader = std::make_shared<Shader>(L"Shader/Sample3 ShadowMap/Compiled/createPointLightShadow.fxo",md3dDevice.Get());

	renderCubeMapShader = std::make_shared<Shader>(L"Shader/Sample1 CubeMap/Compiled/cubeMap.fxo",md3dDevice.Get());

	// 初始化点光源
	pointLight.pos = float3(0, 5, 0);
	pointLight.lightColor = float3(1, 1, 1);
	pointLight.Constant = 1.0;
	pointLight.Linear = 0.09f;
	pointLight.Quadratic = 0.032f;

#pragma region 立方体阴影图生成相关
	// 初始化深度立方体贴图
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 1024;
	texDesc.Height = 1024;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 6;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	HR(md3dDevice->CreateTexture2D(&texDesc, 0, cubeDpethTex.GetAddressOf()));

	// 初始化深度立方体贴图的DSV和SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = 1;
	HR(md3dDevice->CreateShaderResourceView(cubeDpethTex.Get(), &srvDesc, cubeDepthTexSRV.GetAddressOf()));

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.ArraySize = 1;
	cubeDepthTexRTVs.resize(6);
	// 逐个创建RTV
	for (uint i = 0; i < 6;i++) {
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		HR(md3dDevice->CreateRenderTargetView(cubeDpethTex.Get(),&rtvDesc,cubeDepthTexRTVs[i].GetAddressOf()))
	}

	D3D11_TEXTURE2D_DESC depthTexDesc;
	depthTexDesc.Width = texDesc.Width;
	depthTexDesc.Height = texDesc.Height;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 6;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	HR(md3dDevice->CreateTexture2D(&depthTexDesc, 0, depthTex.GetAddressOf()));
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Texture2DArray.MipSlice = 0;
	dsvDesc.Texture2DArray.ArraySize = 1;
	depthViews.resize(6);
	for (uint i = 0; i < 6; i++) {
		dsvDesc.Texture2DArray.FirstArraySlice = i;
		HR(md3dDevice->CreateDepthStencilView(depthTex.Get(),&dsvDesc,depthViews[i].GetAddressOf()));
	}

	HR(md3dDevice->CreateShaderResourceView(depthTex.Get(), &srvDesc, depthTexSRV.GetAddressOf()));

#pragma endregion


	// 初始化视口
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = static_cast<float>(1024);
	viewPort.Height = static_cast<float>(1024);
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

}

void Sample4::UpdateScene(float deltaTime) {

	XMVECTOR lightPos = XMLoadFloat3(&pointLight.pos);
	XMVECTOR right = XMLoadFloat3(&camera->right);
	XMVECTOR up = XMLoadFloat3(&camera->up);

	if (GetAsyncKeyState(VK_LEFT) < 0) {
		lightPos += right * deltaTime * 5;
	}
	if (GetAsyncKeyState(VK_RIGHT) < 0) {
		lightPos -= right * deltaTime * 5;
	}
	if (GetAsyncKeyState(VK_DOWN) < 0) {
		lightPos -= up * deltaTime * 5;
	}
	if (GetAsyncKeyState(VK_UP) < 0) {
		lightPos += up * deltaTime * 5;
	}

	XMStoreFloat3(&pointLight.pos,lightPos);	
}

void Sample4::Render() {
#pragma region 渲染立方体阴影图

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

	md3dImmediateContext->RSSetViewports(1,&viewPort);
	// 更改渲染目标至立方体纹理,并渲染六次场景
	for (int i = 0; i < 6; i++) {
		md3dImmediateContext->PSSetShaderResources(0, 1, null);
		md3dImmediateContext->VSSetShaderResources(0, 1, null);

		md3dImmediateContext->OMSetRenderTargets(1, cubeDepthTexRTVs[i].GetAddressOf(), depthViews[i].Get());
		// 清除颜色缓冲区
		md3dImmediateContext->ClearRenderTargetView(cubeDepthTexRTVs[i].Get(), reinterpret_cast<const float*>(&Colors::Black));
		// 清除深度和模板缓冲
		md3dImmediateContext->ClearDepthStencilView(depthViews[i].Get(), D3D11_CLEAR_DEPTH, 1.0, 0);

		XMVECTOR lightPos = XMLoadFloat3(&pointLight.pos);
		XMVECTOR lookAtPos = lightPos + looks[i].v;
		// 根据六个view矩阵渲染场景
		auto view = XMMatrixLookAtLH(lightPos, lookAtPos, ups[i].v);
		auto proj = XMMatrixPerspectiveFovLH(MathF::Radians(90), 1, 0.1, 10);

		// 渲染场景至立方体贴图
		RenderSceneWithShadowShader(view, proj);
	}

#pragma endregion

	auto view = camera->GetViewMatrix();
	auto proj = camera->GetProjMatrix();

	md3dImmediateContext->RSSetViewports(1,&mScreenViewport);
	md3dImmediateContext->PSSetShaderResources(0, 1, null);
	md3dImmediateContext->VSSetShaderResources(0, 1, null);
	// 重置渲染目标
	md3dImmediateContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());
	// 清空颜色缓冲区
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	// 清除深度和模板缓冲
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);

	RenderScene(view,proj);

	// 渲染光源
	auto lightModel = XMMatrixTranslation(pointLight.pos.x, pointLight.pos.y, pointLight.pos.z);
	auto mvp = lightModel * view * proj;
	renderLightShader->SetMatrix4x4("mvp",mvp);
	lightMesh->Draw(renderLightShader,md3dImmediateContext.Get());

	// 渲染阴影立方体
	auto cubeShadowModel = XMMatrixTranslation(-3,3,0);
	mvp = cubeShadowModel * view * proj;
	renderCubeMapShader->SetShaderResource("skyBox",cubeDepthTexSRV.Get());
	renderCubeMapShader->SetMatrix4x4("mvp",mvp);
	boxMesh->Draw(renderCubeMapShader,md3dImmediateContext.Get());
}

void Sample4::RenderScene(XMMATRIX& view,XMMATRIX& proj) {

	std::vector<float3> positions = {
		{0,0,0},
		{10,0,0},
		{-10,0,0},
		{0,10,0},
		{0,0,10},
		{0,0,-10}
	};
	std::vector<float3> scales = {
		{20,0.5,20},
		{0.5,20,20},
		{0.5,20,20},
		{20,0.5,20},
		{20,20,0.5},
		{20,20,0.5}
	};
	
	for (int i = 0; i < 6; i++) {
		auto planeModel = XMMatrixScaling(scales[i].x, scales[i].y, scales[i].z) * XMMatrixTranslation(positions[i].x, positions[i].y, positions[i].z);
		auto planeTransInvModel = XMMatrixTranspose(GetInverseMatrix(planeModel));
		auto mvp = planeModel * view * proj;
		shader->SetMatrix4x4("mvp", mvp);
		shader->SetMatrix4x4("model", planeModel);
		shader->SetMatrix4x4("transInvModel", planeTransInvModel);
		shader->SetRawValue("pointLight", &pointLight, sizeof(PointLight));
		shader->SetFloat3("viewPos", camera->pos);
		shader->SetShaderResource("mainTex", planeTexture->GetSRV().Get());
		shader->SetFloat("farPlane",10);
		shader->SetShaderResource("shadowMap",cubeDepthTexSRV.Get());
		boxMesh->Draw(shader, md3dImmediateContext.Get());
	}

	std::vector<float3> boxPositions = {
		{0,3,0},
		{3,5,0},
		{-3,5,0},
		{0,7,0}
	};
	std::vector<float3> boxScales = {
		{1,1,1},
		{2,0.5,1},
		{1,0.5,2},
		{0.5,0.5,2}
	};

	for (uint i = 0; i < 4; i++) {
		auto boxModel = XMMatrixScaling(boxScales[i].x, boxScales[i].y, boxScales[i].z) * XMMatrixTranslation(boxPositions[i].x, boxPositions[i].y, boxPositions[i].z);
		auto boxTransInvModel = XMMatrixTranspose(GetInverseMatrix(boxModel));
		auto mvp = boxModel * view * proj;
		shader->SetMatrix4x4("mvp", mvp);
		shader->SetMatrix4x4("model", boxModel);
		shader->SetMatrix4x4("transInvModel", boxTransInvModel);
		shader->SetShaderResource("mainTex", boxTexture->GetSRV().Get());
		shader->SetFloat("farPlane", 10);
		shader->SetShaderResource("shadowMap", cubeDepthTexSRV.Get());
		boxMesh->Draw(shader, md3dImmediateContext.Get());
	}
}

void Sample4::RenderSceneWithShadowShader(XMMATRIX& view, XMMATRIX& proj) {
	createShadowMapShader->SetFloat("farPlane", 10);
	createShadowMapShader->SetFloat3("lightPos",pointLight.pos);
	std::vector<float3> positions = {
		{0,0,0},
		{10,0,0},
		{-10,0,0},
		{0,10,0},
		{0,0,10},
		{0,0,-10}
	};
	std::vector<float3> scales = {
		{20,0.5,20},
		{0.5,20,20},
		{0.5,20,20},
		{20,0.5,20},
		{20,20,0.5},
		{20,20,0.5}
	};

	for (int i = 0; i < 6; i++) {
		auto planeModel = XMMatrixScaling(scales[i].x, scales[i].y, scales[i].z) * XMMatrixTranslation(positions[i].x, positions[i].y, positions[i].z);
		auto planeTransInvModel = XMMatrixTranspose(GetInverseMatrix(planeModel));
		auto mvp = planeModel * view * proj;
		createShadowMapShader->SetMatrix4x4("mvp", mvp);
		createShadowMapShader->SetMatrix4x4("model",planeModel);
		boxMesh->Draw(createShadowMapShader, md3dImmediateContext.Get());
	}

	std::vector<float3> boxPositions = {
		{0,3,0},
		{3,5,0},
		{-3,5,0},
		{0,7,0}
	};
	std::vector<float3> boxScales = {
		{1,1,1},
		{2,0.5,1},
		{1,0.5,2},
		{0.5,0.5,2}
	};

	for (uint i = 0; i < 4; i++) {
		auto boxModel = XMMatrixScaling(boxScales[i].x, boxScales[i].y, boxScales[i].z) * XMMatrixTranslation(boxPositions[i].x, boxPositions[i].y, boxPositions[i].z);
		auto boxTransInvModel = XMMatrixTranspose(GetInverseMatrix(boxModel));
		auto mvp = boxModel * view * proj;
		createShadowMapShader->SetMatrix4x4("mvp", mvp);
		createShadowMapShader->SetMatrix4x4("model",boxModel);
		boxMesh->Draw(createShadowMapShader, md3dImmediateContext.Get());
	}
}