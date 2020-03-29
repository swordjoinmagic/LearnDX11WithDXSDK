#include "Sample8_ShadowTest.h"
#include "GeometryGenerator.h"

void Sample8::OnStart() {
	// 初始化摄像机
	camera = std::make_shared<SJM::Camera>(float3(0, 3, -5), float3(0, 0, 0), AspectRatio());

#pragma region 初始化深度贴图
	// 初始化深度贴图
	D3D11_TEXTURE2D_DESC shadowMapDesc;
	shadowMapDesc.Width = mClientWidth;
	shadowMapDesc.Height = mClientHeight;
	shadowMapDesc.MipLevels = 1;
	shadowMapDesc.ArraySize = 1;
	shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	shadowMapDesc.SampleDesc.Count = 1;
	shadowMapDesc.SampleDesc.Quality = 0;
	shadowMapDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowMapDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowMapDesc.CPUAccessFlags = 0;
	shadowMapDesc.MiscFlags = 0;
	HR(md3dDevice->CreateTexture2D(&shadowMapDesc,0,shadowMap.GetAddressOf()));

	// 初始化深度贴图的SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = shadowMapDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	HR(md3dDevice->CreateShaderResourceView(shadowMap.Get(), &srvDesc, shadowMapSRV.GetAddressOf()));

	// 初始化深度贴图的DSV
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	HR(md3dDevice->CreateDepthStencilView(shadowMap.Get(), &dsvDesc, shadowMapDSV.GetAddressOf()));
#pragma endregion

	boxMesh = GeometryGenerator::CreateBox(1, 1, 1);
	boxMesh->SetUpBuffer(md3dDevice.Get());
	boxShader = std::make_shared<Shader>(L"Shader/Sample5 CSM/Compiled/shadow.fxo", md3dDevice.Get());
	boxTexture = std::make_shared<Texture2D>(md3dDevice.Get(), L"Textures/Crate_Diffuse.jpg");
	planeTexture = std::make_shared<Texture2D>(md3dDevice.Get(), L"Textures/Brick_Diffuse.JPG");

	// 初始化平行光
	light.dir = float3(1, 1, 0);
	light.lightColor = float3(1, 1, 1);
	light.pos = float3(2, 4, 0);
	

	// 初始化SpriteRender
	spriteRender = std::make_shared<SpriteRender>(md3dDevice, mClientWidth, mClientHeight);

	// 初始化渲染深度图的Shader
	renderShadowMapShader = std::make_shared<Shader>(L"Shader/Sample5 CSM/Compiled/renderShadowMap.fxo", md3dDevice.Get());

	whiteShader = std::make_shared<Shader>(L"Shader/Common/Compiled/WhiteObject.fxo", md3dDevice.Get());
	sphereMesh = GeometryGenerator::CreateSphere(0.2,32,32);
	sphereMesh->SetUpBuffer(md3dDevice.Get());
}

void Sample8::UpdateScene(float deltaTime) {
	XMVECTOR lightPos = XMLoadFloat3(&light.pos);
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

	XMStoreFloat3(&light.pos, lightPos);
}

void Sample8::Render() {
	ID3D11RenderTargetView* noneRTV[] = { nullptr };
	ID3D11ShaderResourceView* noneSRV[] = {nullptr,nullptr};

#pragma region 渲染深度图
	// 清空着色器资源
	md3dImmediateContext->PSSetShaderResources(0,2,noneSRV);
	// 重设渲染目标
	md3dImmediateContext->OMSetRenderTargets(1, noneRTV, shadowMapDSV.Get());
	// 清除深度缓冲区
	md3dImmediateContext->ClearDepthStencilView(shadowMapDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);	

	XMVECTOR eyePos = XMLoadFloat3(&light.pos);
	XMVECTOR lightDir = XMLoadFloat3(&light.dir);
	XMVECTOR upDir = XMVectorSet(0, 1, 0, 0);
	XMVECTOR focusPos = XMVectorSet(0,0,0,1);

	auto lightView = XMMatrixLookAtLH(eyePos, focusPos, upDir);
	auto lightProj = XMMatrixOrthographicLH(10, 10, 0.1, 50);

	auto model = XMMatrixTranslation(0, 3, 0);
	auto mvp = model * lightView * lightProj;
	renderShadowMapShader->SetMatrix4x4("mvp", mvp);
	boxMesh->Draw(renderShadowMapShader, md3dImmediateContext.Get());

	model = XMMatrixScaling(20, 0.5, 20);
	mvp = model * lightView * lightProj;
	renderShadowMapShader->SetMatrix4x4("mvp", mvp);
	boxMesh->Draw(renderShadowMapShader, md3dImmediateContext.Get());

#pragma endregion

#pragma region 渲染场景
	// 设置渲染目标
	md3dImmediateContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());
	// 清空颜色缓冲区
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	// 清除深度和模板缓冲
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);


	auto lightVPMatrix = lightView * lightProj;
	boxShader->SetRawValue("light",&light,sizeof(Light));
	boxShader->SetFloat3("viewPos",camera->pos);
	boxShader->SetShaderResource("shadowMap",shadowMapSRV.Get());
	boxShader->SetMatrix4x4("lightVPMatrix",lightVPMatrix);

	auto view = camera->GetViewMatrix(); auto proj = camera->GetProjMatrix();
	model = XMMatrixTranslation(0, 3, 0);
	auto transInvModel = XMMatrixTranspose(GetInverseMatrix(model));
	mvp = model * view * proj;
	boxShader->SetMatrix4x4("mvp", mvp);
	boxShader->SetMatrix4x4("model",model);
	boxShader->SetMatrix4x4("transInvModel",transInvModel);
	boxShader->SetTexture2D("mainTex",boxTexture);
	boxMesh->Draw(boxShader,md3dImmediateContext.Get());

	model = XMMatrixScaling(20, 0.5, 20);
	transInvModel = XMMatrixTranspose(GetInverseMatrix(model));
	mvp = model * view * proj;
	boxShader->SetMatrix4x4("mvp", mvp);
	boxShader->SetMatrix4x4("model", model);
	boxShader->SetMatrix4x4("transInvModel", transInvModel);
	boxShader->SetTexture2D("mainTex", planeTexture);
	boxMesh->Draw(boxShader, md3dImmediateContext.Get());

#pragma endregion

#pragma region 渲染光源
	auto lightModel = XMMatrixTranslation(light.pos.x, light.pos.y, light.pos.z);
	mvp = lightModel * view * proj;
	whiteShader->SetMatrix4x4("mvp",mvp);
	sphereMesh->Draw(whiteShader,md3dImmediateContext.Get());
#pragma endregion

#pragma region 渲染阴影图	
	spriteRender->DrawSprite(
		md3dImmediateContext.Get(),
		shadowMapSRV.Get(),
		float3(-mClientWidth/2,-mClientHeight/2,0),
		float2(300*AspectRatio(),300));
#pragma endregion

}
