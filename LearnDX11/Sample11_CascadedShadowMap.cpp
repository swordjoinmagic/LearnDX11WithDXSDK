#include "Sample11_CascadedShadowMap.h"

#include "GeometryGenerator.h"

void Sample11::OnStart() {

	camera = std::make_shared<SJM::Camera>(float3(0, 3, -5), float3(0, 0, 0), AspectRatio());
	camera->Near = 1;
	camera->Far = 100;

	// 初始化平行光
	light.pos = float3(0, 0, 0);
	light.dir = float3(1, 1, 0);
	light.lightColor = float3(1, 1, 1);


	// 初始化级联阴影分割数组
	m_cascadeEnd.resize(cascadedSize+1);
	m_cascadeEnd[0] = camera->Near;
	m_cascadeEnd[1] = 20;
	m_cascadeEnd[2] = 50;
	m_cascadeEnd[3] = camera->Far;

#pragma region 级联阴影贴图初始化

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = textureSize;
	texDesc.Height = textureSize;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = cascadedSize;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	HR(md3dDevice->CreateTexture2D(&texDesc,0,cascadedShadowTextureArray.GetAddressOf()));

	// 生成级联阴影贴图的SRV和DSV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = cascadedSize;
	HR(md3dDevice->CreateShaderResourceView(cascadedShadowTextureArray.Get(),&srvDesc,cascadedShadowSRV.GetAddressOf()));

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Texture2DArray.MipSlice = 0;
	dsvDesc.Texture2DArray.ArraySize = 1;
	// 逐个生成DSV
	cascadedShadowDSV.resize(cascadedSize);
	for (uint i = 0; i < cascadedSize;i++) {
		dsvDesc.Texture2DArray.FirstArraySlice = i;
		HR(md3dDevice->CreateDepthStencilView(cascadedShadowTextureArray.Get(),&dsvDesc,cascadedShadowDSV[i].GetAddressOf()));
	}


	srvs.resize(cascadedSize);
	srvDesc.Texture2DArray.ArraySize = 1;
	for (uint i = 0; i < cascadedSize;i++) {
		srvDesc.Texture2DArray.FirstArraySlice = i;
		HR(md3dDevice->CreateShaderResourceView(cascadedShadowTextureArray.Get(),&srvDesc,srvs[i].GetAddressOf()));
	}

	

#pragma endregion

#pragma region 初始化物体Shader和Mesh
	boxShader = std::make_shared<Shader>(L"Shader/Sample5 CSM/Compiled/CSM.fxo", md3dDevice.Get());
	boxMesh = GeometryGenerator::CreateSphere(1,32,32);
	boxMesh->SetUpBuffer(md3dDevice.Get());
	boxTexture = std::make_shared<Texture2D>(md3dDevice.Get(), L"Textures/Crate_Diffuse.jpg");
	planeTexture = std::make_shared<Texture2D>(md3dDevice.Get(), L"Textures/Brick_Diffuse.JPG");
	oldBoxShader = std::make_shared<Shader>(L"Shader/Sample5 CSM/Compiled/shadow.fxo", md3dDevice.Get());
#pragma endregion

	// 初始化SpriteRender
	spriteRender = std::make_shared<SpriteRender>(md3dDevice, mClientWidth, mClientHeight);

	// 初始化渲染深度图的Shader
	renderShadowMapShader = std::make_shared<Shader>(L"Shader/Sample5 CSM/Compiled/renderShadowMap.fxo", md3dDevice.Get());

	// 用于渲染阴影贴图的视口
	renderShadowMapViewPort.TopLeftX = 0;
	renderShadowMapViewPort.TopLeftY = 0;
	renderShadowMapViewPort.Width = static_cast<float>(textureSize);
	renderShadowMapViewPort.Height = static_cast<float>(textureSize);
	renderShadowMapViewPort.MinDepth = 0.0f;
	renderShadowMapViewPort.MaxDepth = 1.0f;
	
	// 初始化5*5个箱子的位置
	boxPositions.resize(25);
	for (uint i = 0; i < 5; i++) {
		for (uint j = 0; j < 5; j++) {
			boxPositions[i * 5 + j] = float3(i * 10, 3, j * 10);
		}
	}
	// 正向剔除的渲染状态
	CD3D11_RASTERIZER_DESC cullFrontFaceStateDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
	cullFrontFaceStateDesc.CullMode = D3D11_CULL_FRONT;
	HR(md3dDevice->CreateRasterizerState(&cullFrontFaceStateDesc, cullFrontFaceState.GetAddressOf()));

	cameraPoes.resize(cascadedSize);
	lightVPMatrixs.resize(cascadedSize);
}

void Sample11::UpdateScene(float deltaTime) {

}

void Sample11::Render() {
	ID3D11RenderTargetView* noneRTV[] = { nullptr };
	ID3D11ShaderResourceView* noneSRV[] = {nullptr,nullptr,nullptr};
	md3dImmediateContext->PSSetShaderResources(0,3,noneSRV);

	// 获得三个级联的光源摄像机世界坐标以及他们的VP矩阵
	for (uint i = 0; i < cascadedSize;i++) {
		float nearPlane = m_cascadeEnd[i];
		float farPlane = m_cascadeEnd[i + 1];
		lightVPMatrixs[i] = CalculateCameraCorners(nearPlane,farPlane,&cameraPoes[i]);

		// 设置渲染目标并对阴影进行渲染
		md3dImmediateContext->OMSetRenderTargets(1, noneRTV, cascadedShadowDSV[i].Get());
		// 清除深度缓冲区
		md3dImmediateContext->ClearDepthStencilView(cascadedShadowDSV[i].Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
		// 剔除正面
		md3dImmediateContext->RSSetState(cullFrontFaceState.Get());
		// 设置视口
		md3dImmediateContext->RSSetViewports(1, &renderShadowMapViewPort);

		RenderShadowMap(lightVPMatrixs[i]);
	}

	RenderScene();

	//spriteRender->DrawSprite(
	//	md3dImmediateContext.Get(),
	//	srvs[0].Get(),
	//	float3(-mClientWidth / 2, -mClientHeight / 2, 0),
	//	float2(300 * AspectRatio(), 300));

	//spriteRender->DrawSprite(
	//	md3dImmediateContext.Get(),
	//	srvs[1].Get(),
	//	float3(-mClientWidth / 2 + 400, -mClientHeight / 2, 0),
	//	float2(300 * AspectRatio(), 300));

	//spriteRender->DrawSprite(
	//	md3dImmediateContext.Get(),
	//	srvs[2].Get(),
	//	float3(-mClientWidth / 2, -mClientHeight / 2 + 300, 0),
	//	float2(300 * AspectRatio(), 300));

}

void Sample11::RenderScene() {

	// 设置渲染目标
	md3dImmediateContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());
	// 清空颜色缓冲区
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	// 清除深度和模板缓冲
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);

	auto view = camera->GetViewMatrix(); auto proj = camera->GetProjMatrix();

	boxShader->SetRawValue("light", &light, sizeof(Light));
	boxShader->SetFloat3("viewPos", camera->pos);
	boxShader->SetShaderResource("cascadedShadowMap", cascadedShadowSRV.Get());
	for (uint i = 0; i < cascadedSize; i++) lightVPMatrixs[i] = XMMatrixTranspose(lightVPMatrixs[i]);
	boxShader->SetRawValue("lightVPMatrixs",&lightVPMatrixs[0],sizeof(float4x4)*lightVPMatrixs.size());
	//boxShader->SetMatrix4x4("lightVPMatrix",lightVPMatrixs[2]);

	// 绘制箱子和平面
	for (uint i = 0; i < boxPositions.size(); i++) {
		auto model = XMMatrixScaling(2, 2, 2) * XMMatrixTranslation(boxPositions[i].x, boxPositions[i].y, boxPositions[i].z);
		auto transInvModel = XMMatrixTranspose(GetInverseMatrix(model));
		auto mvp = model * view * proj;
		boxShader->SetMatrix4x4("mvp", mvp);
		boxShader->SetMatrix4x4("model", model);
		boxShader->SetMatrix4x4("transInvModel", transInvModel);
		boxShader->SetTexture2D("mainTex", boxTexture);
		boxMesh->Draw(boxShader, md3dImmediateContext.Get());

	}

	auto model = XMMatrixScaling(200, 0.2, 200);
	auto transInvModel = XMMatrixTranspose(GetInverseMatrix(model));
	auto mvp = model * view * proj;
	boxShader->SetMatrix4x4("mvp", mvp);
	boxShader->SetMatrix4x4("model", model);
	boxShader->SetMatrix4x4("transInvModel", transInvModel);
	boxShader->SetTexture2D("mainTex", planeTexture);
	boxMesh->Draw(boxShader, md3dImmediateContext.Get());

}

void Sample11::RenderShadowMap(XMMATRIX lightVPMatrix) {
	for (uint i = 0; i < boxPositions.size(); i++) {
		auto model = XMMatrixScaling(2, 2, 2) * XMMatrixTranslation(boxPositions[i].x, boxPositions[i].y, boxPositions[i].z);
		auto mvp = model * lightVPMatrix;
		renderShadowMapShader->SetMatrix4x4("mvp", mvp);
		boxMesh->Draw(renderShadowMapShader, md3dImmediateContext.Get());
	}

	auto model = XMMatrixScaling(200, 0.2, 200);
	auto mvp = model * lightVPMatrix;
	renderShadowMapShader->SetMatrix4x4("mvp", mvp);
	boxMesh->Draw(renderShadowMapShader, md3dImmediateContext.Get());

	// 恢复默认光栅化设置
	md3dImmediateContext->RSSetState(NULL);
	// 恢复默认视口
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
}

XMMATRIX Sample11::CalculateCameraCorners(
	float nearPlane, float farPlane,
	float3* camearWorldPos
){

	#pragma region 相似三角形求观察空间下主摄像机视椎体八个顶点位置
	float aspect = camera->aspect;

	// 横向的FOV角的tan值(用于计算近平面和远平面的x)
	float tanHalfHFov = tanf(MathF::Radians((camera->fovAngle * aspect) / 2.0f));
	// 纵向的fov角的tan值
	float tanHalfVFov = tanf(MathF::Radians(camera->fovAngle / 2.0f));

	// 计算主摄像机八个顶点的观察空间坐标(即以主摄像机为原点的空间)
	float xn = tanHalfHFov * nearPlane;
	float xf = tanHalfHFov * farPlane;
	float yn = tanHalfVFov * nearPlane;
	float yf = tanHalfVFov * farPlane;
	float4 cameraConrners[8];

	// 计算近平面的四个点
	// 右上角
	cameraConrners[0] = float4(xn, yn, nearPlane, 1.0);
	// 左上角
	cameraConrners[1] = float4(-xn, yn, nearPlane, 1.0);
	// 右下角
	cameraConrners[2] = float4(xn, -yn, nearPlane, 1.0);
	// 左下角
	cameraConrners[3] = float4(-xn, -yn, nearPlane, 1.0);

	// 计算远平面的四个点
	// 右上角
	cameraConrners[4] = float4(xf, yf, farPlane, 1.0);
	// 左上角
	cameraConrners[5] = float4(-xf, yf, farPlane, 1.0);
	// 右下角
	cameraConrners[6] = float4(xf, -yf, farPlane, 1.0);
	// 左下角
	cameraConrners[7] = float4(-xf, -yf, farPlane, 1.0);
	#pragma endregion

	auto viewMatrix = camera->GetViewMatrix();
	// 计算view的逆矩阵,用于将主摄像机的八个角从观察空间转换到世界空间
	auto invViewMatrix = GetInverseMatrix(viewMatrix);

	auto eyePos = XMVectorSet(0, 0, 0, 1);
	auto lightDir = -XMLoadFloat3(&light.dir);
	auto focusPos = eyePos + lightDir;
	auto upDir = XMVectorSet(0, 1, 0, 0);
	// 光源矩阵,用于将主摄像机的八个顶点从世界空间转换至光源的观察空间
	auto lightMatrix = XMMatrixLookAtLH(eyePos, focusPos, upDir);
	// 从光源空间转回世界空间的矩阵
	auto invLightMatrix = GetInverseMatrix(lightMatrix);

	// 包围盒的min/max点		
	float3 minPoint = float3(FLT_MAX, FLT_MAX, FLT_MAX);
	float3 maxPoint = float3(FLT_MIN, FLT_MIN, FLT_MIN);

	for (uint i = 0; i < 8; i++) {
		XMVECTOR camearConrnerViewPos = XMLoadFloat4(&cameraConrners[i]);

		// 将观察空间的摄像机顶点转换到世界空间
		XMVECTOR worldPos = XMVector4Transform(camearConrnerViewPos, invViewMatrix);

		// 将世界空间的摄像机顶点转换到光源的观察空间
		XMVECTOR lightViewPos = XMVector4Transform(worldPos, lightMatrix);

		float3 viewPos;
		XMStoreFloat3(&viewPos, lightViewPos);

		// 计算八个顶点在光源空间下的包围盒
		minPoint.x = min(minPoint.x, viewPos.x);
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
	XMVECTOR vFrustumNearMaxPoint = XMLoadFloat4(&cameraConrners[0]);
	XMVECTOR vFrustumNearMinPoint = XMLoadFloat4(&cameraConrners[3]);
	XMVECTOR vFrustumFarMinPoint = XMLoadFloat4(&cameraConrners[7]);
	XMVECTOR vFrustumFarMaxPoint = XMLoadFloat4(&cameraConrners[4]);

	float minZ = XMVectorGetZ(minValue);
	float maxZ = XMVectorGetZ(maxValue);	
	
	#pragma region DX SDK 阴影防抖的做法
	XMVECTOR vDiagonal = XMVector3Length(vFrustumNearMaxPoint - vFrustumFarMinPoint);	

	float fCascadeBoung = XMVectorGetX(vDiagonal);

	XMVECTOR vBorderOffset = (vDiagonal - (maxValue - minValue)) * XMVectorSet(0.5, 0.5, 0.5, 0.5);
	vBorderOffset *= XMVectorSet(1, 1, 0, 0);

	maxValue += vBorderOffset;
	minValue -= vBorderOffset;

	// 计算阴影贴图的每个纹素
	XMVECTOR fWorldUnitPerTexel = XMVectorSet(fCascadeBoung / textureSize, fCascadeBoung / textureSize, 1, 1);

	// 让包围盒的minValue和maxValue为纹素的倍数
	minValue /= fWorldUnitPerTexel;
	minValue = XMVectorFloor(minValue);
	minValue *= fWorldUnitPerTexel;

	maxValue /= fWorldUnitPerTexel;
	maxValue = XMVectorFloor(maxValue);
	maxValue *= fWorldUnitPerTexel;

	XMStoreFloat3(&minPoint, minValue);
	minPoint.z = minZ;
	XMStoreFloat3(&maxPoint, maxValue);
	maxPoint.z = maxZ;
	#pragma endregion

	#pragma endregion

	// 计算光源摄像机视椎体八个顶点
	float3 lightCorner[8];

	// 近平面
	// 左下角
	lightCorner[0] = float3(minPoint.x, minPoint.y, minPoint.z);
	// 右下角
	lightCorner[1] = float3(maxPoint.x, minPoint.y, minPoint.z);
	// 右上角
	lightCorner[2] = float3(maxPoint.x, maxPoint.y, minPoint.z);
	// 左上角
	lightCorner[3] = float3(minPoint.x, maxPoint.y, minPoint.z);

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
	lightCameraPos = XMVector3Transform(lightCameraPos, invLightMatrix);

	// 获得光源摄像机在世界空间的坐标
	XMStoreFloat3(camearWorldPos,lightCameraPos);

	eyePos = lightCameraPos;
	lightDir = -XMLoadFloat3(&light.dir);
	focusPos = eyePos + lightDir;
	upDir = XMVectorSet(0, 1, 0, 0);

	auto view = XMMatrixLookAtLH(eyePos, focusPos, upDir);
	auto proj = XMMatrixOrthographicLH(
		maxPoint.x - minPoint.x,
		maxPoint.y - minPoint.y,
		0,
		maxPoint.z - minPoint.z
	);

	return view * proj;
}