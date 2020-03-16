#include "d3dApp.h"
#include <string>
#include <assert.h>
#include<conio.h>

//#define DEBUG

// 为了设置窗口的回调函数,消息处理函数
// 用到的全局变量
namespace {
	D3DApp* gd3dApp = 0;
}
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return gd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

// 构造方法,初始化所有变量,同时处理一下外部这个MainWndProc
D3DApp::D3DApp(HINSTANCE hInstance) : 
	mhAppInst(hInstance),
	mMainWndCaption(L"Learn D3D11"),
	md3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
	mClientWidth(800),
	mClientHeight(600),
	mEnable4xMsaa(false),
	mhMainWnd(0),
	mAppPaused(false),
	mMinimized(false),
	mMaximized(false),
	mResizing(false),
	m4xMessQuality(0),
	md3dDevice(NULL),
	md3dImmediateContext(NULL),
	mSwapChain(NULL),
	mDepthStencilBuffer(NULL),
	mRenderTargetView(NULL),
	mDepthStencilView(NULL)
{
	ZeroMemory(&mScreenViewport,sizeof(D3D11_VIEWPORT));

	gd3dApp = this;
}

// 析构,用于释放所有COM接口
D3DApp::~D3DApp() {
	if (md3dImmediateContext) md3dImmediateContext->ClearState();
}

HINSTANCE D3DApp::AppInst() const {
	return mhAppInst;
}

HWND D3DApp::MainWnd() const {
	return mhMainWnd;
}

// 屏幕宽高比例
float D3DApp::AspectRatio() const {
	return static_cast<float>(mClientWidth)/mClientHeight;
}

// 计算帧率
void D3DApp::CalculateFrameState() {
	static int frameCnt = 0;
	static float timeElpsed = 0.0f;

	frameCnt++;
	

	if (timer.TotalTime() - timeElpsed >= 1.0f) {
		float fps = (float)frameCnt;
		float mspf = 1000.0 / fps;
		
		
		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);

		std::wstring windowText = L"   fps: " + fpsStr +
			L"    mspf: " + mspfStr+L"(ms)";
		SetWindowText(mhMainWnd,windowText.c_str());

		frameCnt = 0;
		timeElpsed += 1.0f;
	}
}

// 消息处理函数
LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_SIZE:
		mClientWidth = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);
		if (md3dDevice) {
			if (wParam == SIZE_MINIMIZED) {
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			} else if(wParam == SIZE_MAXIMIZED){
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			} else if(wParam == SIZE_RESTORED){
				if (mMinimized) {
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				} else if(mMaximized){
					mAppPaused = false;
					mMaximized = false;
					OnResize();
				} else if (mResizing) {} else {
					OnResize();
				}
			}
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_MENUCHAR:
		return MAKELRESULT(0,MNC_CLOSE);
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam,GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}

	return DefWindowProc(hwnd,msg,wParam,lParam);
}

// 初始化窗口
bool D3DApp::InitMainWindow() {
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mhAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"D3DWndClassName";

	

	if (!RegisterClass(&wc)) {
		MessageBox(0,L"注册窗口失败",0,0);
		return false;
	}

	RECT R = { 0, 0, mClientWidth, mClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	

	mhMainWnd = CreateWindow(L"D3DWndClassName",mMainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0);
	if (!mhMainWnd) {
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return true;
}

// 初始化d3d程序
bool D3DApp::InitDirect3D() {
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;

	HRESULT hr = D3D11CreateDevice(
		0,	// 默认显卡
		md3dDriverType,
		0,	// 不使用软渲染
		createDeviceFlags,
		0,0,// 默认特征
		D3D11_SDK_VERSION,
		md3dDevice.GetAddressOf(),
		&featureLevel,
		md3dImmediateContext.GetAddressOf()
	);
	// 判断是否创建d3d设备失败
	if (FAILED(hr)) {
		MessageBox(0,L"创建d3d11Device失败!",0,0);
		return false;
	}

	// 判断目前能支持的最高特征能不能支持d3d11
	if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
		MessageBox(0,L"d3d11的特征不被支持!",0,0);
		return false;
	}

	// 检查是否支持msaa抗锯齿
	HR(md3dDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM,4,&m4xMessQuality
	));
	if (m4xMessQuality <= 0) {
		MessageBox(0, L"msaa不被支持!", 0, 0);
	}


	// 填充并创建交换链
	DXGI_SWAP_CHAIN_DESC swapDesc;
	swapDesc.BufferDesc.Width = mClientWidth;
	swapDesc.BufferDesc.Height = mClientHeight;
	// 显示刷新率
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	// 后台缓冲区像素格式
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// 显示扫描线模式
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// 是否使用4xmsaa
	if (mEnable4xMsaa) {
		swapDesc.SampleDesc.Count = 4;
		swapDesc.SampleDesc.Quality = m4xMessQuality - 1;
	} else {
		swapDesc.SampleDesc.Count = 1;
		swapDesc.SampleDesc.Quality = 0;
	}

	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.BufferCount = 1;
	swapDesc.OutputWindow = mhMainWnd;
	swapDesc.Windowed = true;
	// 让显卡驱动程序选择最高效 的显示模式
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Flags = 0;

	// 创建交换链
	ComPtr<IDXGIDevice> dxgiDevice = 0;
	HR(md3dDevice->QueryInterface(__uuidof(IDXGIDevice),(void**)dxgiDevice.GetAddressOf()));
	
	ComPtr<IDXGIAdapter> dxgiAdapter = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter),(void**)dxgiAdapter.GetAddressOf()));

	ComPtr<IDXGIFactory> dxgiFactory = 0;
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory),(void**)dxgiFactory.GetAddressOf()));

	// 根据factory创建交换链
	HR(dxgiFactory->CreateSwapChain(md3dDevice.Get(),&swapDesc,mSwapChain.GetAddressOf()));

	OnResize();
	return true;
}

void D3DApp::OnResize() {
	assert(md3dImmediateContext);
	assert(md3dDevice);
	assert(mSwapChain);

	// 将之前的视图和深度/模板缓冲区释放(如果有就释放),重新建一个	
	mRenderTargetView.Reset();
	mDepthStencilView.Reset();
	mDepthStencilBuffer.Reset();

	// 重置交换链后置缓冲区的宽高
	HR(mSwapChain->ResizeBuffers(1,mClientWidth,mClientHeight,DXGI_FORMAT_R8G8B8A8_UNORM,0));

	// 创建渲染目标视图
	ComPtr<ID3D11Texture2D> backBuffer;
	HR(mSwapChain->GetBuffer(0,__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(backBuffer.GetAddressOf())));
	HR(md3dDevice->CreateRenderTargetView(backBuffer.Get(),0,mRenderTargetView.GetAddressOf()));

	// 创建深度/模板缓冲区和它的视图
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = mClientWidth;
	depthStencilDesc.Height = mClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;	// 在纹理数组中的纹理数量
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 是否开启4xmsaa
	if (mEnable4xMsaa) {
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m4xMessQuality - 1;
	} else {
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;	

	// 创建深度/模板缓冲
	HR(md3dDevice->CreateTexture2D(&depthStencilDesc,0,mDepthStencilBuffer.GetAddressOf()));
	// 创建深度/模板视图
	HR(md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(),0,mDepthStencilView.GetAddressOf()));

	// 将视图绑定到输出合并阶段
	md3dImmediateContext->OMSetRenderTargets(1,mRenderTargetView.GetAddressOf(),mDepthStencilView.Get());

	// 设置视口变换
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(mClientWidth);
	mScreenViewport.Height = static_cast<float>(mClientHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	md3dImmediateContext->RSSetViewports(1,&mScreenViewport);
}

// 初始化窗口以及d3d程序
bool D3DApp::Init() {
	if (!InitMainWindow())
		return false;
	if (!InitDirect3D())
		return false;

	OnStart();

	return true;
}

void D3DApp::DrawScene() {
	// 清屏为蓝色
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	// 清除深度和模板缓冲
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);

	Render();

	// 交换双缓冲
	HR(mSwapChain->Present(0, 0));
}

// 执行消息循环和游戏循环
int D3DApp::Run() {
	MSG msg = {0};

	timer.Reset();

	while (msg.message != WM_QUIT) {
		// 有消息就处理消息,没消息就处理游戏循环
		if (PeekMessage(&msg,0,0,0,PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			// 计算deltaTime
			timer.Tick();
			if (!mAppPaused) {

				#ifdef DEBUG
				// 清除控制台调试信息
				ClearConsoleDebugInformation();
				#endif // DEBUG

				// 计算帧率
				CalculateFrameState();

				// 处理输入
				ProcessKeyBoardInput();

				// 更新场景逻辑
				UpdateScene(timer.DeltaTime());

				// 渲染场景
				DrawScene();

			} else {
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

// 处理摄像机以及他的移动事件
void D3DApp::ProcessKeyBoardInput() {

	if (camera) {
		short shiftState = GetAsyncKeyState(VK_SHIFT);
		if (shiftState<0)
			camera->MovementSpeed += 10.0f*timer.DeltaTime();
		else if(shiftState==0)
			camera->MovementSpeed = SJM::SPEED;

		if (GetAsyncKeyState(VK_KEY_A) < 0)
			camera->ProcessKeyBoard(SJM::CameraMovement::LEFT, timer.DeltaTime());
		if (GetAsyncKeyState(VK_KEY_W) < 0) {
			camera->ProcessKeyBoard(SJM::CameraMovement::FORWAD, timer.DeltaTime());		
		}
		if (GetAsyncKeyState(VK_KEY_D)<0)
			camera->ProcessKeyBoard(SJM::CameraMovement::RIGHT, timer.DeltaTime());
		if (GetAsyncKeyState(VK_KEY_S)<0)
			camera->ProcessKeyBoard(SJM::CameraMovement::BACKWARD, timer.DeltaTime());
	}
}

void D3DApp::OnMouseMove(WPARAM btnState, int x, int y) {
	if (GetKeyState(VK_RBUTTON) < 0) {
		if (firstMouse) {
			lastX = x;
			lastY = y;
			firstMouse = false;
		}
		float xOffset = lastX - x;
		float yOffset = lastY - y;

		lastX = x;
		lastY = y;

		camera->ProcessMouseMovement(xOffset,yOffset);
	} else {
		lastX = x;
		lastY = y;
		firstMouse = false;
	}
}

void D3DApp::ClearConsoleDebugInformation() {
	system("cls");
}