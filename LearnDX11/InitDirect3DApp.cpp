#include "InitDirect3DApp.h"

InitDirect3DApp::InitDirect3DApp(HINSTANCE hInstance) : D3DApp(hInstance){}
InitDirect3DApp::~InitDirect3DApp() {	}

void InitDirect3DApp::OnStart() {}

void InitDirect3DApp::OnResize() {
	D3DApp::OnResize();
}

void InitDirect3DApp::UpdateScene(float deltaTime) {}

void InitDirect3DApp::Render() {

}