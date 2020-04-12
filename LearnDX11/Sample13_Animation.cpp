#include "Sample13_Animation.h"

#include "MeshLoader.h"
#include "MathF.h"
#include <iostream>

void Sample13::OnStart() {
	camera = std::make_shared<SJM::Camera>(float3(0, 0, -3), float3(0, 0, 0), AspectRatio());
	camera->Far = 50;

	characterMesh = MeshLoader::LoadMesh("Models/skeleton_animated.FBX");
	characterMesh->SetUpBuffer(md3dDevice.Get());
	characterSkinMesh = MeshLoader::LoadSkinMesh("Models/skeleton_animated.FBX");
	characterSkinMesh->SetUpBuffer(md3dDevice.Get());

	characterTexture = std::make_shared<Texture2D>(md3dDevice.Get(), L"Textures/Crate_Diffuse.jpg");
	normalShader = std::make_shared<Shader>(L"Shader/Sample2 Light/Compiled/parallelLight.fxo", md3dDevice.Get());
	shader = std::make_shared<Shader>(L"Shader/Sample7 Animation/Compiled/anime.fxo",md3dDevice.Get(),false);

	// 初始化平行光
	light.pos = float3(0, 0, 0);
	light.dir = float3(1, 1, 0);
	light.lightColor = float3(1, 1, 1);

	// 初始化蒙皮网格的顶点输入布局
	D3D11_INPUT_ELEMENT_DESC inputElementsDesc[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT,0,24,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"BONEID",0,DXGI_FORMAT_R32_SINT,0,36,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"BONEID",1,DXGI_FORMAT_R32_SINT,0,40,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"BONEID",2,DXGI_FORMAT_R32_SINT,0,44,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"BONEID",3,DXGI_FORMAT_R32_SINT,0,48,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"WEIGHTS",0,DXGI_FORMAT_R32_FLOAT,0,52,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"WEIGHTS",1,DXGI_FORMAT_R32_FLOAT,0,56,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"WEIGHTS",2,DXGI_FORMAT_R32_FLOAT,0,60,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"WEIGHTS",3,DXGI_FORMAT_R32_FLOAT,0,64,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,68,D3D11_INPUT_PER_VERTEX_DATA,0}
	};

	Shader::ShaderInputSign inputSign = shader->GetInputSign();

	HR(md3dDevice->CreateInputLayout(
		inputElementsDesc,
		12,
		inputSign.pIAInputSignature,
		inputSign.IAInputSignatureSize,
		skinVertexInputLayout.GetAddressOf()));

}

void Sample13::UpdateScene(float deltaTime) {

}

void Sample13::Render() {

	auto view = camera->GetViewMatrix(); auto proj = camera->GetProjMatrix();

	auto model = XMMatrixScaling(0.03, 0.03, 0.03) * XMMatrixRotationY(MathF::Radians(180));
	auto mvp = model * view * proj;
	auto transInvModel = GetInverseMatrix(model);

	//normalShader->SetMatrix4x4("mvp", mvp);
	//normalShader->SetMatrix4x4("model",model);
	//normalShader->SetMatrix4x4("transInvModel",transInvModel);
	//normalShader->SetTexture2D("mainTex",characterTexture);
	//normalShader->SetFloat3("viewPos",camera->pos);
	//normalShader->SetRawValue("light",&light,sizeof(Light));
	//characterMesh->Draw(normalShader, md3dImmediateContext.Get());

	shader->SetMatrix4x4("mvp", mvp);
	shader->SetMatrix4x4("model", model);
	shader->SetMatrix4x4("transInvModel", transInvModel);
	shader->SetTexture2D("mainTex", characterTexture);
	shader->SetFloat3("viewPos", camera->pos);
	shader->SetRawValue("light", &light, sizeof(Light));

	std::vector<aiMatrix4x4> transforms;		
	characterSkinMesh->BoneTransform(0, timer.TotalTime(), transforms);
	//characterSkinMesh->BoneTransform(0, 0.2, transforms);

	shader->SetRawValue("gBones", &transforms[0], sizeof(aiMatrix4x4)*transforms.size());


	// 设置绘制模式
	md3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// 设置顶点和索引缓冲
	characterSkinMesh->Use(md3dImmediateContext.Get());
	// 设置着色器
	shader->UsePass(0,skinVertexInputLayout.Get(),md3dImmediateContext.Get());
	
	md3dImmediateContext->DrawIndexed(characterSkinMesh->indexCount,0,0);
}