#include "Sample1_Cube.h"
#include "Camera.h"
#include "GeometryGenerator.h"

Sample1::~Sample1() {
	
}
void Sample1::OnStart() {
	camera = std::make_shared<SJM::Camera>(float3(0, 0, -3), float3(0, 0, 0), AspectRatio());

	// 初始化Mesh
	boxMesh = GeometryGenerator::CreateBox(1,1,1);
	boxMesh->SetUpBuffer(md3dDevice.Get());

	// 初始化Shader
	shader = std::make_shared<Shader>(L"Shader/Common/Compiled/WhiteObject.fxo",md3dDevice.Get());
}

void Sample1::Render() {
	XMMATRIX view = camera->GetViewMatrix();
	XMMATRIX proj = camera->GetProjMatrix();
	XMMATRIX mvp = view * proj;
	shader->SetMatrix4x4("mvp",mvp);

	boxMesh->Draw(shader,md3dImmediateContext.Get());

}