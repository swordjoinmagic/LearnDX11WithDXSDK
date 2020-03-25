#include "Sample6_QuadTree.h"

#include "GeometryGenerator.h"
#include <iostream>

void Sample6::OnStart() {
	camera = std::make_shared<SJM::Camera>(float3(0, 0, -3), float3(0, 0, 0), AspectRatio());
	camera->Far = 100;

	// 初始化线型渲染模式
	CD3D11_RASTERIZER_DESC lineModeDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
	lineModeDesc.FillMode = D3D11_FILL_WIREFRAME;
	HR(md3dDevice->CreateRasterizerState(&lineModeDesc, lineMode.GetAddressOf()));

	boxMesh = GeometryGenerator::CreateBox(1,1,1);	
	boxMesh->SetUpBuffer(md3dDevice.Get());
	colliderShader = std::make_shared<Shader>(L"Shader/Common/Compiled/WhiteObject.fxo", md3dDevice.Get());
	
	sphereMesh = GeometryGenerator::CreateSphere(0.2,32,32);
	sphereMesh->SetUpBuffer(md3dDevice.Get());
	XNA::ComputeBoundingOrientedBoxFromPoints(&gameObjectOrientedBox,sphereMesh->vertics.size(),&sphereMesh->vertics[0].pos,sizeof(DefaultVertex));

	// 初始化四叉树
	quadTree = std::make_shared<QuadTree>(float3(0,0,0),float3(15,1,15),float4(0,0,0,1));
	quadTree->maxDepth = 8;

#pragma region 初始化游戏对象
	gameObjects.resize(25);
	// 初始化5x5个立方体
	for (uint i = 0; i < 5; i++) {
		for (uint j = 0; j < 5; j++) {

			// 每个立方体间隔为2
			float3 pos = float3(i * 2+1, 0, j * 2+1);
			// 每个立方体(x,y)多旋转10度
			float3 rotation = float3(0,0, 0);
			// 每个立方体均无缩放
			float3 scale = float3(1, 1, 1);

			auto gameObject = std::make_shared<GameObject>();
			gameObject->pos = pos;
			gameObject->scale = scale;
			gameObject->rotation = rotation;
			//gameObject->mesh = sphereMesh;
			//gameObject->shader = colliderShader;
			gameObject->colliderBox = gameObjectOrientedBox;

			gameObjects[i * 5 + j] = gameObject;

			quadTree->AddGameObject(gameObject);
		}
	}

#pragma endregion

}

void Sample6::UpdateScene(float deltaTime) {

}

void Sample6::Render() {
	
	RenderQuadTree(quadTree);
	
	auto view = camera->GetViewMatrix(); auto proj = camera->GetProjMatrix();
	for (auto gameObject : gameObjects) {
		auto model =
			XMMatrixScaling(gameObject->scale.x, gameObject->scale.y, gameObject->scale.z) *
			XMMatrixRotationX(MathF::Radians(gameObject->rotation.x)) *
			XMMatrixRotationY(MathF::Radians(gameObject->rotation.y)) *
			XMMatrixRotationZ(MathF::Radians(gameObject->rotation.z)) * 
			XMMatrixTranslation(gameObject->pos.x, gameObject->pos.y, gameObject->pos.z);
		auto mvp = model * view * proj;
		colliderShader->SetMatrix4x4("mvp",mvp);
		sphereMesh->Draw(colliderShader,md3dImmediateContext.Get());
		
	}

}

void Sample6::RenderQuadTree(std::shared_ptr<QuadTree> quadTree) {
	md3dImmediateContext->RSSetState(lineMode.Get());
	RenderQuadTreeNode(quadTree->treeRoot);
	md3dImmediateContext->RSSetState(NULL);
}

void Sample6::RenderQuadTreeNode(std::shared_ptr<QuadTreeNode> quadTreeNode) {

	auto view = camera->GetViewMatrix(); auto proj = camera->GetProjMatrix();

	float3 scale = float3(
		quadTreeNode->nodeColliderBox.Extents.x * 2,
		quadTreeNode->nodeColliderBox.Extents.y * 2,
		quadTreeNode->nodeColliderBox.Extents.z * 2
	);
	float3 pos = quadTreeNode->nodeColliderBox.Center;

	auto model = XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixTranslation(pos.x,pos.y,pos.z);
	auto mvp = model * view * proj;
	colliderShader->SetMatrix4x4("mvp",mvp);
	boxMesh->Draw(colliderShader,md3dImmediateContext.Get());

	for (uint i = 0; i < 4;i++) {
		if (quadTreeNode->childNodes[i]) {
			RenderQuadTreeNode(quadTreeNode->childNodes[i]);
		}
	}
}

