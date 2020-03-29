#include "Sample7_CSM.h"
#include "GeometryGenerator.h"
#include "MathF.h"

void Sample7::OnStart() {
	// 初始化摄像机
	camera = std::make_shared<SJM::Camera>(float3(0, 3, -5), float3(0, 0, 0), AspectRatio());
	camera->Near = 1;

	// 初始化平行光
	light.pos = float3(0,0,0);
	light.dir = float3(1,1,0);
	light.lightColor = float3(1,1,1);

	//light.pos = float3(2,4,0);
#pragma region 初始化深度贴图
	// 初始化深度贴图
	D3D11_TEXTURE2D_DESC shadowMapDesc;
	shadowMapDesc.Width = textureSize;
	shadowMapDesc.Height = textureSize;
	shadowMapDesc.MipLevels = 1;
	shadowMapDesc.ArraySize = 1;
	shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	shadowMapDesc.SampleDesc.Count = 1;
	shadowMapDesc.SampleDesc.Quality = 0;
	shadowMapDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowMapDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowMapDesc.CPUAccessFlags = 0;
	shadowMapDesc.MiscFlags = 0;

	HR(md3dDevice->CreateTexture2D(&shadowMapDesc,NULL,shadowMap.GetAddressOf()));

	// 初始化深度贴图的SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = shadowMapDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	HR(md3dDevice->CreateShaderResourceView(shadowMap.Get(),&srvDesc,shadowMapSRV.GetAddressOf()));

	// 初始化深度贴图的DSV
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	HR(md3dDevice->CreateDepthStencilView(shadowMap.Get(),&dsvDesc,shadowMapDSV.GetAddressOf()));
#pragma endregion

#pragma region 初始化场景的Mesh和Shader和Texture2D

	boxMesh = GeometryGenerator::CreateBox(1,1,1);
	boxMesh->SetUpBuffer(md3dDevice.Get());
	boxShader = std::make_shared<Shader>(L"Shader/Sample5 CSM/Compiled/shadow.fxo",md3dDevice.Get());
	boxTexture = std::make_shared<Texture2D>(md3dDevice.Get(),L"Textures/Crate_Diffuse.jpg");
	planeTexture = std::make_shared<Texture2D>(md3dDevice.Get(), L"Textures/Brick_Diffuse.JPG");

#pragma endregion

	// 获得世界空间光源摄像机的坐标
	// 获得光源摄像机各参数
	CalculateCameraCorners();

	// 初始化SpriteRender
	spriteRender = std::make_shared<SpriteRender>(md3dDevice,mClientWidth,mClientHeight);

	// 初始化渲染深度图的Shader
	renderShadowMapShader = std::make_shared<Shader>(L"Shader/Sample5 CSM/Compiled/renderShadowMap.fxo",md3dDevice.Get());


	CD3D11_RASTERIZER_DESC cullFrontFaceStateDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
	cullFrontFaceStateDesc.CullMode = D3D11_CULL_FRONT;
	HR(md3dDevice->CreateRasterizerState(&cullFrontFaceStateDesc,cullFrontFaceState.GetAddressOf()));

	renderShadowMapViewPort.TopLeftX = 0;
	renderShadowMapViewPort.TopLeftY = 0;
	renderShadowMapViewPort.Width = static_cast<float>(textureSize);
	renderShadowMapViewPort.Height = static_cast<float>(textureSize);
	renderShadowMapViewPort.MinDepth = 0.0f;
	renderShadowMapViewPort.MaxDepth = 1.0f;


}

void Sample7::CalculateCameraCorners() {
	float aspect = camera->aspect;
	
	// 横向的FOV角的tan值(用于计算近平面和远平面的x)
	float tanHalfHFov = tanf(MathF::Radians( (camera->fovAngle * aspect) / 2.0f ));
	// 纵向的fov角的tan值
	float tanHalfVFov = tanf(MathF::Radians(camera->fovAngle / 2.0f));

	// 计算主摄像机八个顶点的观察空间坐标(即以主摄像机为原点的空间)
	float xn = tanHalfHFov * camera->Near;
	float xf = tanHalfHFov * camera->Far;
	float yn = tanHalfVFov * camera->Near;
	float yf = tanHalfVFov * camera->Far;

	// 计算近平面的四个点

	// 右上角
	cameraConrners[0] = float4(xn,yn,camera->Near,1.0);
	// 左上角
	cameraConrners[1] = float4(-xn,yn,camera->Near,1.0);
	// 右下角
	cameraConrners[2] = float4(xn,-yn,camera->Near,1.0);
	// 左下角
	cameraConrners[3] = float4(-xn,-yn,camera->Near,1.0);

	// 计算远平面的四个点

	// 右上角
	cameraConrners[4] = float4(xf,yf,camera->Far,1.0);
	// 左上角
	cameraConrners[5] = float4(-xf,yf,camera->Far,1.0);
	// 右下角
	cameraConrners[6] = float4(xf,-yf,camera->Far,1.0);
	// 左下角
	cameraConrners[7] = float4(-xf,-yf,camera->Far,1.0);


	auto viewMatrix = camera->GetViewMatrix();
	// 计算view的逆矩阵,用于将主摄像机的八个角从观察空间转换到世界空间
	auto invViewMatrix = GetInverseMatrix(viewMatrix);
	
	auto eyePos = XMVectorSet(0,0,0,1);
	auto lightDir = -XMLoadFloat3(&light.dir);
	auto focusPos = eyePos + lightDir;
	auto upDir = XMVectorSet(0,1,0,0);
	// 光源矩阵,用于将主摄像机的八个顶点从世界空间转换至光源的观察空间
	auto lightMatrix = XMMatrixLookAtLH(eyePos,focusPos,upDir);
	// 从光源空间转回世界空间的矩阵
	auto invLightMatrix = GetInverseMatrix(lightMatrix);

	// 包围盒的min/max点		
	float3 minPoint = float3(FLT_MAX, FLT_MAX, FLT_MAX);
	float3 maxPoint = float3(FLT_MIN, FLT_MIN, FLT_MIN);	

	for (uint i = 0; i < 8; i++) {
		XMVECTOR camearConrnerViewPos = XMLoadFloat4(&cameraConrners[i]);

		// 将观察空间的摄像机顶点转换到世界空间
		XMVECTOR worldPos = XMVector4Transform(camearConrnerViewPos,invViewMatrix);

		// 将世界空间的摄像机顶点转换到光源的观察空间
		XMVECTOR lightViewPos = XMVector4Transform(worldPos,lightMatrix);

		float3 viewPos;
		XMStoreFloat3(&viewPos,lightViewPos);

		// 计算八个顶点在光源空间下的包围盒
		minPoint.x = min(minPoint.x,viewPos.x);
		minPoint.y = min(minPoint.y, viewPos.y);
		minPoint.z = min(minPoint.z, viewPos.z);
		maxPoint.x = max(maxPoint.x, viewPos.x);
		maxPoint.y = max(maxPoint.y, viewPos.y);
		maxPoint.z = max(maxPoint.z, viewPos.z);
	}

#pragma region 取余算法阴影抖动问题
	
	// 约束光源以Texel为增量进行移动
	XMVECTOR minValue = XMLoadFloat3(&minPoint);
	XMVECTOR maxValue = XMLoadFloat3(&maxPoint);
	XMVECTOR vFrustumPoints0 = XMLoadFloat4(&cameraConrners[0]);
	XMVECTOR vFrustumPoints6 = XMLoadFloat4(&cameraConrners[7]);

	float minZ = XMVectorGetZ(minValue);
	float maxZ = XMVectorGetZ(maxValue);

	XMVECTOR vDiagonal = XMVector3Length(vFrustumPoints0 - vFrustumPoints6);

	float fCascadeBoung = XMVectorGetX(vDiagonal);

	XMVECTOR vBorderOffset = (vDiagonal - (maxValue - minValue)) * XMVectorSet(0.5,0.5,0.5,0.5);
	vBorderOffset *= XMVectorSet(1,1,0,0);

	maxValue += vBorderOffset;
	minValue -= vBorderOffset;

	// 计算阴影贴图的每个纹素
	XMVECTOR fWorldUnitPerTexel = XMVectorSet(fCascadeBoung / textureSize, fCascadeBoung  / textureSize, 1, 1);

	// 让包围盒的minValue和maxValue为纹素的倍数
	minValue /= fWorldUnitPerTexel;
	minValue = XMVectorFloor(minValue);
	minValue *= fWorldUnitPerTexel;

	maxValue /= fWorldUnitPerTexel;
	maxValue = XMVectorFloor(maxValue);
	maxValue *= fWorldUnitPerTexel;

	XMStoreFloat3(&minPoint,minValue);
	minPoint.z = minZ;
	XMStoreFloat3(&maxPoint,maxValue);
	maxPoint.z = maxZ;

#pragma endregion

	// 获得光源摄像机正交投影的各个参数
	// (或者说获得主摄像机在光源方向的包围盒信息)
	shadowOrthProjInfo.right = maxPoint.x;
	shadowOrthProjInfo.left = minPoint.x;
	shadowOrthProjInfo.bottom = minPoint.y;
	shadowOrthProjInfo.top = maxPoint.y;
	shadowOrthProjInfo.Far = maxPoint.z;
	shadowOrthProjInfo.Near = minPoint.z;

	// 计算光源摄像机视椎体八个顶点

	// 近平面

	// 左下角
	lightCorner[0] = float3(minPoint.x,minPoint.y,minPoint.z);
	// 右下角
	lightCorner[1] = float3(maxPoint.x,minPoint.y,minPoint.z);
	// 右上角
	lightCorner[2] = float3(maxPoint.x,maxPoint.y,minPoint.z);
	// 左上角
	lightCorner[3] = float3(minPoint.x,maxPoint.y,minPoint.z);

	// 远平面
	// 左下角
	lightCorner[4] = float3(minPoint.x, minPoint.y, maxPoint.z);
	// 右下角
	lightCorner[5] = float3(maxPoint.x, minPoint.y, maxPoint.z);
	// 右上角
	lightCorner[6] = float3(maxPoint.x, maxPoint.y, maxPoint.z);
	// 左上角
	lightCorner[7] = float3(minPoint.x, maxPoint.y, maxPoint.z);
	
	// 光源摄像机的位置 = 近平面左下角向量 + (近平面左下角指向右上角向量) / 2
	// 即 lightCameraPos = lightCorner[0] + (lightcorner[2] - lightcorner[0]) / 2;

	XMVECTOR nearLeftDownVector = XMLoadFloat3(&lightCorner[0]);
	XMVECTOR nearRightUpVector = XMLoadFloat3(&lightCorner[2]);

	// 光源摄像机的位置(光源视图空间)
	XMVECTOR lightCameraPos = nearLeftDownVector + (nearRightUpVector - nearLeftDownVector) / 2;
	
	// 将光源摄像机的位置转回世界空间
	lightCameraPos = XMVector3Transform(lightCameraPos,invLightMatrix);

	// 获得光源摄像机在世界空间的坐标
	XMStoreFloat3(&lcPos,lightCameraPos);



	
}

void Sample7::UpdateScene(float deltaTime) {
	
}

void Sample7::Render() {
	ID3D11ShaderResourceView* none[] = { nullptr,nullptr };
	md3dImmediateContext->PSSetShaderResources(0, 2, none);

	RenderShadowMap();

	md3dImmediateContext->PSSetShaderResources(0, 2, none);
	RenderScene();

	md3dImmediateContext->PSSetShaderResources(0, 2, none);

	spriteRender->DrawSprite(
		md3dImmediateContext.Get(),
		shadowMapSRV.Get(),
		float3(-mClientWidth/2,-mClientHeight/2,0),
		float2(300*AspectRatio(),300));
}

void Sample7::RenderScene() {
	// 设置渲染目标
	md3dImmediateContext->OMSetRenderTargets(1,mRenderTargetView.GetAddressOf(),mDepthStencilView.Get());
	// 清空颜色缓冲区
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	// 清除深度和模板缓冲
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);

#pragma region 计算光源的VP矩阵

	auto eyePos = XMLoadFloat3(&light.pos);
	auto lightDir = -XMLoadFloat3(&light.dir);
	auto focusPos = eyePos + lightDir;
	auto upDir = XMVectorSet(0, 1, 0, 0);

	auto lightView = XMMatrixLookAtLH(eyePos, focusPos, upDir);
	auto lightProj = XMMatrixOrthographicLH(
		shadowOrthProjInfo.right - shadowOrthProjInfo.left,
		shadowOrthProjInfo.top - shadowOrthProjInfo.bottom,
		0,
		shadowOrthProjInfo.Far - shadowOrthProjInfo.Near
	);

	auto lightVPMatrix = lightView * lightProj;
#pragma endregion


	auto view = camera->GetViewMatrix(); auto proj = camera->GetProjMatrix();

	boxShader->SetRawValue("light", &light, sizeof(Light));
	boxShader->SetFloat3("viewPos", camera->pos);
	boxShader->SetShaderResource("shadowMap", shadowMapSRV.Get());
	boxShader->SetMatrix4x4("lightVPMatrix",lightVPMatrix);
	auto model = XMMatrixScaling(2, 2, 2) * XMMatrixTranslation(0,3,0);
	auto transInvModel = XMMatrixTranspose(GetInverseMatrix(model));
	auto mvp = model * view * proj;
	boxShader->SetMatrix4x4("mvp",mvp);
	boxShader->SetMatrix4x4("model",model);
	boxShader->SetMatrix4x4("transInvModel",transInvModel);
	boxShader->SetTexture2D("mainTex",boxTexture);
	boxMesh->Draw(boxShader,md3dImmediateContext.Get());

	model = XMMatrixScaling(20,0.2,20);
	transInvModel = XMMatrixTranspose(GetInverseMatrix(model));
	mvp = model * view * proj;
	boxShader->SetMatrix4x4("mvp",mvp);
	boxShader->SetMatrix4x4("model", model);
	boxShader->SetMatrix4x4("transInvModel", transInvModel);
	boxShader->SetTexture2D("mainTex", planeTexture);
	boxMesh->Draw(boxShader,md3dImmediateContext.Get());

}

void Sample7::RenderShadowMap() {
	// 计算光源摄像机的位置
	CalculateCameraCorners();
	light.pos = lcPos;


	ID3D11RenderTargetView* none[] = { nullptr };
	// 重设渲染目标
	md3dImmediateContext->OMSetRenderTargets(1,none,shadowMapDSV.Get());
	// 清除深度缓冲区
	md3dImmediateContext->ClearDepthStencilView(shadowMapDSV.Get(),D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,1.0,0);
	// 剔除正面
	md3dImmediateContext->RSSetState(cullFrontFaceState.Get());
	// 设置视口
	md3dImmediateContext->RSSetViewports(1,&renderShadowMapViewPort);
	
	auto eyePos = XMLoadFloat3(&light.pos);
	auto lightDir = -XMLoadFloat3(&light.dir);
	auto focusPos = eyePos + lightDir;
	auto upDir = XMVectorSet(0, 1, 0, 0);

	auto view = XMMatrixLookAtLH(eyePos,focusPos,upDir);
	auto proj = XMMatrixOrthographicLH(
		shadowOrthProjInfo.right - shadowOrthProjInfo.left,
		shadowOrthProjInfo.top - shadowOrthProjInfo.bottom,
		0,
		shadowOrthProjInfo.Far - shadowOrthProjInfo.Near
	);

	auto model = XMMatrixScaling(2,2,2) * XMMatrixTranslation(0, 3, 0);
	auto mvp = model * view * proj;
	renderShadowMapShader->SetMatrix4x4("mvp", mvp);
	boxMesh->Draw(renderShadowMapShader, md3dImmediateContext.Get());

	model = XMMatrixScaling(20, 0.2, 20);
	mvp = model * view * proj;
	renderShadowMapShader->SetMatrix4x4("mvp", mvp);
	boxMesh->Draw(renderShadowMapShader, md3dImmediateContext.Get());

	// 恢复默认光栅化设置
	md3dImmediateContext->RSSetState(NULL);
	// 恢复默认视口
	md3dImmediateContext->RSSetViewports(1,&mScreenViewport);
	
}