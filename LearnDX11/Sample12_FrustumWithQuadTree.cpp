#include "Sample12_FrustumWithQuadTree.h"

#include "MathF.h"
#include "GeometryGenerator.h"
#include <iostream>

void Sample12::OnStart() {
	camera = std::make_shared<SJM::Camera>(float3(0, 0, -3), float3(0, 0, 0), AspectRatio());
	camera->Far = 300;

	// ��ʼ��Shader/Texture��Mesh
	boxMesh = GeometryGenerator::CreateBox(1, 1, 1);
	boxMesh->SetUpBuffer(md3dDevice.Get());
	boxShader = std::make_shared<Shader>(L"Shader/Sample2 Light/Compiled/parallelLight.fxo", md3dDevice.Get());
	boxTexture = std::make_shared<Texture2D>(md3dDevice.Get(), L"Textures/Crate_Diffuse.jpg");
	planeTexture = std::make_shared<Texture2D>(md3dDevice.Get(), L"Textures/Brick_Diffuse.JPG");

	// ��ʼ���Ĳ���
	quadTree = std::make_shared<QuadTree>(float3(0, 0, 0), float3(200, 1, 200), float4(0, 0, 0, 1));
	quadTree->maxDepth = 8;

	XNA::OrientedBox colliderBox;
	XNA::ComputeBoundingOrientedBoxFromPoints(&colliderBox, boxMesh->vertics.size(), &boxMesh->vertics[0].pos, sizeof(DefaultVertex));

	gameObjectList.resize(1000);
	// ��ʼ��40*25������
	for (uint i = 0; i < 40;i++) {
		for (uint j = 0; j < 25;j++) {
			auto gameObject = std::make_shared<GameObject>();
			// ÿ�����Ӽ��3����λ
			gameObject->pos = float3(i*3+5,0,j*7+5);
			// ��ת��(0-30)֮��
			gameObject->rotation = float3(MathF::RandF(0,30), MathF::RandF(0, 30), MathF::RandF(0, 30));
			// ������0.5-1.5֮��
			gameObject->scale = float3(MathF::RandF(0.5,1.5), MathF::RandF(0.5, 1.5), MathF::RandF(0.5, 1.5));
			gameObject->colliderBox = colliderBox;

			gameObjectList[i * 25 + j] = gameObject;

			quadTree->AddGameObject(gameObject);
		}
	}

	// ��ʼ��ƽ�й�
	light.pos = float3(0, 0, 0);
	light.dir = float3(1, 1, 0);
	light.lightColor = float3(1, 1, 1);


	// ��ʼ���������׵��
	XNA::ComputeFrustumFromProjection(&cameraFrustum,&camera->GetProjMatrix());

	needRenderGameObjects.resize(1000);


	CD3D11_RASTERIZER_DESC rasterizerStateDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
	rasterizerStateDesc.FillMode = D3D11_FILL_WIREFRAME;
	HR(md3dDevice->CreateRasterizerState(&rasterizerStateDesc,lineMode.GetAddressOf()));
	colliderShader = std::make_shared<Shader>(L"Shader/Common/Compiled/WhiteObject.fxo", md3dDevice.Get());

}

void Sample12::QuadTreeCulling(std::shared_ptr<Sample12::QuadTreeNode> treeNode,XMMATRIX view,uint depth) {

	// �͵�ǰ�ڵ����ײ����,����ɹ�,��ô��Ⱦ��ǰ�ڵ���������GameObejct
	// ͬʱ��ǰ�ڵ���ĸ��ӽڵ���м����޳�
	// ���ʧ��,��ôֱ�ӷ���

	// ���Ĳ����ĸ��ӽڵ��Χ�б任���۲�ռ�
	XNA::OrientedBox viewSpaceCollider;

	XMVECTOR translation, quat, scale;
	XMMatrixDecompose(&scale, &quat, &translation, view);
	quat = XMQuaternionNormalize(quat);
	XNA::TransformOrientedBox(&viewSpaceCollider, &treeNode->nodeColliderBox, XMVectorGetX(scale), quat, translation);

	bool intersectResult = XNA::IntersectOrientedBoxFrustum(&viewSpaceCollider, &cameraFrustum);
	if (intersectResult) {

		// ��Ⱦ��ǰ�ڵ����������
		for (auto gameObject : treeNode->gameObjects) {
			needRenderGameObjects[renderSize++] = gameObject;
		}

		if (depth < quadTree->maxDepth) {
			// �������ĸ��ӽڵ�����޳�
			for (uint i = 0; i < 4; i++) {
				auto childNode = treeNode->childNodes[i];
				if(childNode!=nullptr)
					QuadTreeCulling(childNode,view,depth+1);
			}
		}
	}
}

void Sample12::UpdateScene(float deltaTime) {

#pragma region ����GameObject0���˶��߼�
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

#pragma region ������
	//// ���������������׵���޳�
	//for (auto gameObject : gameObjectList) {
	//	auto model =
	//		XMMatrixScaling(gameObject->scale.x, gameObject->scale.y, gameObject->scale.z) *
	//		XMMatrixRotationX(MathF::Radians(gameObject->rotation.x)) *
	//		XMMatrixRotationY(MathF::Radians(gameObject->rotation.y)) *
	//		XMMatrixRotationZ(MathF::Radians(gameObject->rotation.z)) *
	//		XMMatrixTranslation(gameObject->pos.x, gameObject->pos.y, gameObject->pos.z);

	//	// �������ģ�Ϳռ�ת�����۲�ռ�ı任����
	//	auto mvMatrix = model * view;

	//	// �ֽ����
	//	XMVECTOR scale;
	//	XMVECTOR rotQuat;
	//	XMVECTOR translation;
	//	XMMatrixDecompose(&scale, &rotQuat, &translation, mvMatrix);
	//	rotQuat = XMQuaternionNormalize(rotQuat);

	//	// ������İ�Χ�б任���۲�ռ�
	//	XNA::OrientedBox gameObjectColliderBox;
	//	XNA::TransformOrientedBox(&gameObjectColliderBox, &gameObject->colliderBox, XMVectorGetX(scale), rotQuat, translation);

	//	if (XNA::IntersectOrientedBoxFrustum(&gameObjectColliderBox, &cameraFrustum)) {
	//		needRenderGameObjects[renderSize++] = (gameObject);
	//	}
	//}

#pragma endregion	

	QuadTreeCulling(quadTree->treeRoot,view,0);

	std::cout << "������Ⱦ��������Ϊ:" << renderSize << std::endl;
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
