#pragma once

#include "d3dApp.h"
#include "Shader.h"
#include "Mesh.h"

class Sample10 : public D3DApp{
private:
	std::shared_ptr<Shader> streamOutShader;
	std::shared_ptr<Shader> colorShader;

	std::shared_ptr<Mesh> boxMesh;
public:
	Sample10(HINSTANCE hInstance) : D3DApp(hInstance){}

	void OnStart() override;
	void Render() override;

};