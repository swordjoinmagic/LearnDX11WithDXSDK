#pragma once

#include <D3D11.h>
#include <D3DX11.h>
#include <dxerr.h>
#include <xnamath.h>
#include <vector>
#include "effect/d3dx11effect.h"
#include <string>
#include <sstream>
#include <fstream>
#include <wrl/client.h>

typedef XMFLOAT2 float2;
typedef XMFLOAT3 float3;
typedef XMFLOAT4 float4;
typedef XMFLOAT4X4 float4x4;
typedef XMFLOAT3X3 float3x3;
typedef XMCOLOR Color;
typedef UINT uint;


#define VK_KEY_A 0x41
#define VK_KEY_B 0x42
#define VK_KEY_C 0x43
#define VK_KEY_D 0x44
#define VK_KEY_E 0x45
#define VK_KEY_F 0x46
#define VK_KEY_G 0x47
#define VK_KEY_H 0x48
#define VK_KEY_I 0x49
#define VK_KEY_J 0x4A
#define VK_KEY_K 0x4B
#define VK_KEY_L 0x4C
#define VK_KEY_M 0x4D
#define VK_KEY_N 0x4E
#define VK_KEY_O 0x4F
#define VK_KEY_P 0x50
#define VK_KEY_Q 0x51
#define VK_KEY_R 0x52
#define VK_KEY_S 0x53
#define VK_KEY_T 0x54
#define VK_KEY_U 0x55
#define VK_KEY_V 0x56
#define VK_KEY_W 0x57
#define VK_KEY_X 0x58
#define VK_KEY_Y 0x59
#define VK_KEY_Z 0x5A

template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

#ifndef HR
#define HR(x) {\
	\
	HRESULT hr = (x);\
	if(FAILED(hr)){\
		DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true);\
	}\
}
#endif // !HR

#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }


namespace Colors {
	XMGLOBALCONST XMVECTORF32 White = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };

	XMGLOBALCONST XMVECTORF32 Silver = { 0.75f, 0.75f, 0.75f, 1.0f };
	XMGLOBALCONST XMVECTORF32 LightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };
}

#pragma region Shader编译相关
ID3DBlob* LoadCompiledShaderFromFile(const std::wstring& fileName);
std::vector<char> LoadCompiledEffectFile(const std::wstring& fileName, int& size);
#pragma endregion


// 简化申请缓冲流程
HRESULT CreateBuffer(
	D3D11_USAGE usage,
	uint byteWidth,
	uint bindFlag,
	uint cpuAccessFlags,
	uint miscFlags,
	uint structureByteStride,
	const void *data,
	ID3D11Device* d3dDevice,
	ID3D11Buffer** buffer
);

XMMATRIX GetInverseMatrix(XMMATRIX A);

class d3dHelper {
public:
	///<summary>
	/// 
	/// Does not work with compressed formats.
	///</summary>
	static ComPtr<ID3D11ShaderResourceView> CreateTexture2DArraySRV(
		ID3D11Device* device, ID3D11DeviceContext* context,
		std::vector<std::wstring>& filenames,
		DXGI_FORMAT format = DXGI_FORMAT_FROM_FILE,
		UINT filter = D3DX11_FILTER_NONE,
		UINT mipFilter = D3DX11_FILTER_LINEAR);

	static ComPtr<ID3D11ShaderResourceView> CreateRandomTexture1DSRV(ID3D11Device* device);
};