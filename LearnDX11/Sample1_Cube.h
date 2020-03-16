#pragma once

#include "d3dApp.h"
#include "Mesh.h"

class Sample1 : public D3DApp {
private:
	std::shared_ptr<Mesh> boxMesh;
	std::shared_ptr<Shader> shader;
public:
	Sample1(HINSTANCE hInstance) : D3DApp(hInstance) {}
	~Sample1();
	void OnStart() override;
	void Render() override;
};