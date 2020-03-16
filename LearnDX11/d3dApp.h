#pragma once
#ifndef D3DAPP_H
#define D3DAPP_H

#include "D3DUtils.h"
#include "GameTimer.h"
#include <Windows.h>
#include <windowsx.h>
#include <string>
#include "Camera.h"

class D3DApp {
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();

	// 获得这个d3d程序的句柄
	HINSTANCE AppInst() const;
	// 获得窗口句柄
	HWND MainWnd() const;
	// 获得窗口比例
	float AspectRatio() const;

	int Run();

	// 初始化窗口和D3D上下文
	virtual bool Init();

	// 当窗口改变时,自动引用分辨率改变的模板/深度缓冲区
	virtual void OnResize();

	// 用于更新场景逻辑(位移/碰撞检测等)
	virtual void UpdateScene(float deltaTime = 0) {}

	// 初始化该d3dApp的方法,一般用于初始化模型,顶点结构,着色器等
	virtual void OnStart() = 0;

	// 绘制场景(即用于渲染的函数)
	virtual void Render() = 0;

	// 窗口过程函数,用于处理各类消息
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// 用于处理鼠标事件
	virtual void OnMouseDown(WPARAM btnState, int x, int y) {}
	virtual void OnMouseUp(WPARAM btnState, int x, int y) {}
	virtual void OnMouseMove(WPARAM btnState, int x, int y);
	// 处理任意键盘事件
	virtual void ProcessKeyBoardInput();
protected:
	// 初始化Windows窗口
	bool InitMainWindow();
	// 初始化D3D程序
	bool InitDirect3D();
	void CalculateFrameState();
	virtual void DrawScene();

	// 清除控制台信息
	void ClearConsoleDebugInformation();
protected:
	// 应用程序实例句柄
	HINSTANCE mhAppInst;
	// 主窗口句柄
	HWND mhMainWnd;	
	// 程序是否处于暂停状态
	bool mAppPaused;
	// 程序是否最小化最大化
	bool mMinimized;
	bool mMaximized;
	// 程序是否处于在改变大小的状态
	bool mResizing;

	// 4x msaa的质量等级
	UINT m4xMessQuality;

	// 用于记录deltaTime的计时器类
	GameTimer timer;

protected:
	// d3d设备
	ComPtr<ID3D11Device> md3dDevice;
	ComPtr<ID3D11DeviceContext> md3dImmediateContext;
	ComPtr<IDXGISwapChain> mSwapChain;
	ComPtr<ID3D11Texture2D> mDepthStencilBuffer;
	ComPtr<ID3D11RenderTargetView> mRenderTargetView;
	ComPtr<ID3D11DepthStencilView> mDepthStencilView;
	D3D11_VIEWPORT mScreenViewport;

	D3D_DRIVER_TYPE md3dDriverType;

	int mClientWidth;
	int mClientHeight;

	bool mEnable4xMsaa;

	// 窗口名
	std::wstring mMainWndCaption = L"Learning D3D";

protected:
	// 处理摄像机与鼠标移动
	int lastX=0, lastY=0;
	bool firstMouse = true;
	std::shared_ptr<SJM::Camera> camera;

};

#endif // !D3DAPP_H
