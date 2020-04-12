#pragma once

#include "d3dApp.h"
#include "Shader.h"
#include "Mesh.h"
#include "Light.h"
#include "Texture2D.h"
#include "SkinMesh.h"

class Sample13 : public D3DApp{
private:

	std::shared_ptr<Mesh> characterMesh;
	std::shared_ptr<SkinMesh> characterSkinMesh;
	std::shared_ptr<Shader> shader;
	
	std::shared_ptr<Shader> normalShader;
	std::shared_ptr<Texture2D> characterTexture;

	Light light;

	ComPtr<ID3D11InputLayout> skinVertexInputLayout;
public:
	Sample13(HINSTANCE hInstance) : D3DApp(hInstance) {}
	void OnStart() override;
	void UpdateScene(float deltaTime) override;
	void Render() override;

};