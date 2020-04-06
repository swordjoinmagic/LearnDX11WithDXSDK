#include "Sample12_FrustumWithQuadTree.h"

#include "MathF.h"
#include "GeometryGenerator.h"
#include <iostream>

void Sample12::OnStart() {
	camera = std::make_shared<SJM::Camera>(float3(0, 0, -3), float3(0, 0, 0), AspectRatio());
	camera->Far = 300;

	// 初始化Shader/Texture和Mesh
	boxMesh = GeometryGenerator::CreateBox(1, 1, 1);
	boxMesh->SetUpBuffer(md3dDevice.Get());
	boxShader = std::make_shared<Shader>(L"Shader/Sample2 Light/Compiled/parallelLight.fxo", md3dDevice.Get());
	boxTexture = std::make_shared<Texture2D>(md3dDevice.Get(), L"Textures/Crate_Diffuse.jpg");
	planeTexture = std::make_shared<Texture2D>(md3dDevice.Get(), L"Textures/Brick_Diffuse.JPG");

	// 初始化四叉树
	quadTree = std::make_shared<QuadTree>(float3(0, 0, 0), float3(200, 1, 200), float4(0, 0, 0, 1));
	quadTree->maxDepth = 8;

	XNA::OrientedBox colliderBox;
	XNA::ComputeBoundingOrientedBoxFromPoints(&colliderBox, boxMesh->vertics.size(), &boxMesh->vertics[0].pos, sizeof(DefaultVertex));

	gameObjectList.resize(1000);
	// 初始化40*25个箱子
	for (uint i = 0; i < 40;i++) {
		for (uint j = 0; j < 25;j++) {
			auto gameObject = std::make_shared<GameObject>();
			// 每个箱子间隔3个单位
			gameObject->pos = float3(i*3+5,0,j*7+5);
			// 旋转在(0-30)之间
			gameObject->rotation = float3(MathF::RandF(0,30), MathF::RandF(0, 30), MathF::RandF(0, 30));
			// 缩放在0.5-1.5之间
			gameObject->scale = float3(MathF::RandF(0.5,1.5), MathF::RandF(0.5, 1.5), MathF::RandF(0.5, 1.5));
			gameObject->colliderBox = colliderBox;

			gameObjectList[i * 25 + j] = gameObject;

			quadTree->AddGameObject(gameObject);
		}
	}

	// 初始化平行光
	light.pos = float3(0, 0, 0);
	light.dir = float3(1, 1, 0);
	light.lightColor = float3(1, 1, 1);


	// 初始化摄像机视椎体
	XNA::ComputeFrustumFromProjection(&cameraFrustum,&camera->GetProjMatrix());

	needRenderGameObjects.resize(1000);


	CD3D11_RASTERIZER_DESC rasterizerStateDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
	rasterizerStateDesc.FillMode = D3D11_FILL_WIREFRAME;
	HR(md3dDevice->CreateRasterizerState(&rasterizerStateDesc,lineMode.GetAddressOf()));
	colliderShader = std::make_shared<Shader>(L"Shader/Common/Compiled/WhiteObject.fxo", md3dDevice.Get());

}

void Sample12::QuadTreeCulling(std::shared_ptr<Sample12::QuadTreeNode> treeNode,XMMATRIX view,uint depth) {

	// 和当前节点进碰撞测试,如果成功,那么渲染当前节点所包含的GameObejct
	// 同时向当前节点的四个子节点进行继续剔除
	// 如果失败,那么直接返回

	// 将四叉树四个子节点包围盒变换至观察空间
	XNA::OrientedBox viewSpaceCollider;

	XMVECTOR translation, quat, scale;
	XMMatrixDecompose(&scale, &quat, &translation, view);
	quat = XMQuaternionNormalize(quat);
	XNA::TransformOrientedBox(&viewSpaceCollider, &treeNode->nodeColliderBox, XMVectorGetX(scale), quat, translation);

	bool intersectResult = XNA::IntersectOrientedBoxFrustum(&viewSpaceCollider, &cameraFrustum);
	if (intersectResult) {

		// 渲染当前节点包含的物体
		for (auto gameObject : treeNode->gameObjects) {
			needRenderGameObjects[renderSize++] = gameObject;
		}

		if (depth < quadTree->maxDepth) {
			// 继续向四个子节点进行剔除
			for (uint i = 0; i < 4; i++) {
				auto childNode = treeNode->childNodes[i];
				if(childNode!=nullptr)
					QuadTreeCulling(childNode,view,depth+1);
			}
		}
	}
}

void Sample12::UpdateScene(float deltaTime) {

#pragma region 处理GameObject0的运动逻辑
	auto gameObject = gameObjectList[0];
	XMVECTOR gameObjectPos = XMLoadFloat3(&gameObject->pos);
	XMVECTOR right = XMLoadFloat3(&camera->right);
	XMVECTOR up = XMLoadFloat3(&camera->up);

	if (GetAsyncKeyState(VK_LEFT) < 0) {
		gameObjectPos += right * deltaTime * 5;
	}
	if (GetAsyncKeyState(VK_RIGHT) < 0) {
		gameObjectPos -= right * deltaTime * 5;
	}
	if (GetAsyncKeyState(VK_DOWN) < 0) {
		gameObjectPos -= up * deltaTime * 5;
	}
	if (GetAsyncKeyState(VK_UP) < 0) {
		gameObjectPos += up * deltaTime * 5;
	}

	XMStoreFloat3(&gameObject->pos, gameObjectPos);
#pragma endregion

	auto view = camera->GetViewMatrix(); auto proj = camera->GetProjMatrix();
	renderSize = 0;

#pragma region 蛮力法
	//// 对所有物体进行视椎体剔除
	//for (auto gameObject : gameObjectList) {
	//	auto model =
	//		XMMatrixScaling(gameObject->scale.x, gameObject->scale.y, gameObject->scale.z) *
	//		XMMatrixRotationX(MathF::Radians(gameObject->rotation.x)) *
	//		XMMatrixRotationY(MathF::Radians(gameObject->rotation.y)) *
	//		XMMatrixRotationZ(MathF::Radians(gameObject->rotation.z)) *
	//		XMMatrixTranslation(gameObject->pos.x, gameObject->pos.y, gameObject->pos.z);

	//	// 将物体从模型空间转换至观察空间的变换矩阵
	//	auto mvMatrix = model * view;

	//	// 分解矩阵
	//	XMVECTOR scale;
	//	XMVECTOR rotQuat;
	//	XMVECTOR translation;
	//	XMMatrixDecompose(&scale, &rotQuat, &translation, mvMatrix);
	//	rotQuat = XMQuaternionNormalize(rotQuat);

	//	// 将物体的包围盒变换至观察空间
	//	XNA::OrientedBox gameObjectColliderBox;
	//	XNA::TransformOrientedBox(&gameObjectColliderBox, &gameObject->colliderBox, XMVectorGetX(scale), rotQuat, translation);

	//	if (XNA::IntersectOrientedBoxFrustum(&gameObjectColliderBox, &cameraFrustum)) {
	//		needRenderGameObjects[renderSize++] = (gameObject);
	//	}
	//}

#pragma endregion	

	QuadTreeCulling(quadTree->treeRoot,view,0);

	std::cout << "本次渲染物体数量为:" << renderSize << std::endl;
}

void Sample12::Render() {
	auto view = camera->GetViewMatrix(); auto proj = camera->GetProjMatrix();

	boxShader->SetFloat3("viewPos",camera->pos);
	boxShader->SetRawValue("light",&light,sizeof(Light));
	boxShader->SetTexture2D("mainTex",boxTexture);
	for (uint i = 0; i < renderSize;i++) {
		auto gameObject = needRenderGameObjects[i];
		auto model =
			XMMatrixScaling(gameObject->scale.x, gameObject->scale.y, gameObject->scale.z) *
			XMMatrixRotationX(MathF::Radians(gameObject->rotation.x)) *
			XMMatrixRotationY(MathF::Radians(gameObject->rotation.y)) *
			XMMatrixRotationZ(MathF::Radians(gameObject->rotation.z)) *
			XMMatrixTranslation(gameObject->pos.x, gameObject->pos.y, gameObject->pos.z);
		auto transInvModel = XMMatrixTranspose(GetInverseMatrix(model));
		auto mvp = model * view	* proj;
		boxShader->SetMatrix4x4("mvp",mvp);
		boxShader->SetMatrix4x4("transInvModel",transInvModel);
		boxShader->SetMatrix4x4("model",model);

		boxMesh->Draw(boxShader,md3dImmediateContext.Get());
		
	}

	//RenderQuadTree(quadTree);
}

void Sample12::RenderQuadTree(std::shared_ptr<QuadTree> quadTree) {
	md3dImmediateContext->RSSetState(lineMode.Get());
	RenderQuadTreeNode(quadTree->treeRoot);
	md3dImmediateContext->RSSetState(NULL);
}

void Sample12::RenderQuadTreeNode(std::shared_ptr<QuadTreeNode> quadTreeNode) {

	auto view = camera->GetViewMatrix(); auto proj = camera->GetProjMatrix();

	float3 scale = float3(
		quadTreeNode->nodeColliderBox.Extents.x * 2,
		quadTreeNode->nodeColliderBox.Extents.y * 2,
		quadTreeNode->nodeColliderBox.Extents.z * 2
	);
	float3 pos = quadTreeNode->nodeColliderBox.Center;

	auto model = XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixTranslation(pos.x, pos.y, pos.z);
	auto mvp = model * view * proj;
	colliderShader->SetMatrix4x4("mvp", mvp);
	boxMesh->Draw(colliderShader, md3dImmediateContext.Get());

	for (uint i = 0; i < 4; i++) {
		if (quadTreeNode->childNodes[i]) {
			RenderQuadTreeNode(quadTreeNode->childNodes[i]);
		}
	}
}
