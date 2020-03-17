#include "Sample3_Light.h"

#include "GeometryGenerator.h"

void Sample3::OnStart() {
	camera = std::make_shared<SJM::Camera>(float3(0, 0, -3), float3(0, 0, 0), AspectRatio());

	// ��ʼ��mesh��shader
	lightMesh = GeometryGenerator::CreateSphere(0.2,32,32);
	lightMesh->SetUpBuffer(md3dDevice.Get());
	lightShader = std::make_shared<Shader>(L"Shader/Common/Compiled/WhiteObject.fxo",md3dDevice.Get());
	
	shader = std::make_shared<Shader>(L"Shader/Sample2 Light/Compiled/pointLight.fxo",md3dDevice.Get());
	boxMesh = GeometryGenerator::CreateBox(1,1,1);
	boxMesh->SetUpBuffer(md3dDevice.Get());

	spotLightShader = std::make_shared<Shader>(L"Shader/Sample2 Light/Compiled/spotLight.fxo",md3dDevice.Get());

	texture = std::make_shared<Texture2D>(md3dDevice.Get(),L"Textures/Brick_Diffuse.JPG",D3D11_BIND_SHADER_RESOURCE);

	// ��ʼ�����Դ
	pointLight.pos = float3(2,2,0);
	pointLight.lightColor = float3(1,1,1);
	pointLight.Constant = 1.0;
	pointLight.Linear = 0.09f;
	pointLight.Quadratic = 0.032f;

	// ��ʼ���۹��
	spotLight.pos = camera->pos;
	spotLight.dir = camera->forward;
	spotLight.lightColor = float3(1,1,1);
	spotLight.cutOff = cos(MathF::Radians(7.5));
	spotLight.outerCutOff = cos(MathF::Radians(15));
	spotLight.Constant = 1.0;
	spotLight.Linear = 0.09f;
	spotLight.Quadratic = 0.032f;	
}

void Sample3::UpdateScene(float deltaTime) {

	// ���¾۹�Ƶ������λ��
	spotLight.pos = camera->pos;
	spotLight.dir = camera->forward;

	if (GetAsyncKeyState(VK_LEFT) < 0) {
		pointLight.pos.x -= deltaTime * 5;
	}
	if (GetAsyncKeyState(VK_RIGHT) < 0) {
		pointLight.pos.x += deltaTime * 5;
	}
	if (GetAsyncKeyState(VK_DOWN) < 0) {
		pointLight.pos.y -= deltaTime * 5;
	}
	if (GetAsyncKeyState(VK_UP) < 0) {
		pointLight.pos.y += deltaTime * 5;
	}
}

void Sample3::Render() {
	auto view = camera->GetViewMatrix();
	auto proj = camera->GetProjMatrix();

	// ��Ⱦ��Դ
	auto lightModel = XMMatrixTranslation(pointLight.pos.x, pointLight.pos.y, pointLight.pos.z);
	auto mvp = lightModel * view * proj;
	lightShader->SetMatrix4x4("mvp",mvp);
	lightMesh->Draw(lightShader,md3dImmediateContext.Get());

	// ��Ⱦ����
	XMMATRIX boxModel = XMMatrixScaling(1,1,1);
	mvp = boxModel * view * proj;
	XMMATRIX transInvModel = XMMatrixTranspose(GetInverseMatrix(boxModel));
	spotLightShader->SetMatrix4x4("mvp",mvp);
	spotLightShader->SetMatrix4x4("model",boxModel);
	spotLightShader->SetMatrix4x4("transInvModel",transInvModel);
	spotLightShader->SetRawValue("light",&spotLight,sizeof(SpotLight));
	spotLightShader->SetFloat3("viewPos",camera->pos);
	spotLightShader->SetShaderResource("mainTex",texture->GetSRV().Get());
	boxMesh->Draw(spotLightShader,md3dImmediateContext.Get());
}