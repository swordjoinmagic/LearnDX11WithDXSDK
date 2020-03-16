#pragma once

#include "d3dApp.h"

class InitDirect3DApp : public D3DApp{
public:
	InitDirect3DApp(HINSTANCE hInstance);
	virtual ~InitDirect3DApp();

	void OnResize() override;
	void UpdateScene(float deltaTime) override;
	void Render() override;
	void OnStart() override;
};

