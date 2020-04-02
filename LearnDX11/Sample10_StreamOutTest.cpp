#include "Sample10_StreamOutTest.h"

#include "GeometryGenerator.h"

void Sample10::OnStart() {

	camera = std::make_shared<SJM::Camera>(float3(0, 0, -5), float3(0, 0, 0), AspectRatio());

	boxMesh = GeometryGenerator::CreateBox(1,1,1);
	boxMesh->SetUpBuffer(md3dDevice.Get());

	colorShader = std::make_shared<Shader>(L"Shader/Common/Compiled/WhiteObject.fxo", md3dDevice.Get());
	
}

void Sample10::Render() {
	auto view = camera->GetViewMatrix(); auto proj = camera->GetProjMatrix();
	
	auto mvp = view * proj;
	colorShader->SetMatrix4x4("mvp",mvp);
	boxMesh->Draw(colorShader,md3dImmediateContext.Get());
}