#pragma once

#include "d3dApp.h"
#include "Mesh.h"
#include "Shader.h"
#include "Light.h"
#include "Texture2D.h"

class Sample3 : public D3DApp{
private:
	std::shared_ptr<Mesh> lightMesh;
	std::shared_ptr<Shader> lightShader;

	std::shared_ptr<Shader> shader;
	std::shared_ptr<Mesh> boxMesh;

	std::shared_ptr<Shader> spotLightShader;

	std::shared_ptr<Texture2D> texture;

	PointLight pointLight;

	SpotLight spotLight;
public:
	Sample3(HINSTANCE hInstance) :D3DApp(hInstance) {}
	void OnStart() override;
	void UpdateScene(float deltaTime) override;
	void Render() override;
};