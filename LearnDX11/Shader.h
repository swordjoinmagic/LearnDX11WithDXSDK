#pragma once
#include "D3DUtils.h"
#include "Texture2D.h"
#include "CubeMap.h"

/*
	着色器类,用于封装着色器操作,封装有下列操作:
		1. 设置各类常量缓冲区变量
		2. 在绘制前的各类环境配置,如输入布局/绘制模式等

*/
class Shader {
public:
	struct ShaderInputSign {
		byte* pIAInputSignature;
		size_t IAInputSignatureSize;
	};
	ShaderInputSign GetInputSign();
private:
	ComPtr<ID3DX11Effect> effect;
	ID3DX11EffectTechnique* technique;

	// 对应各个Pass的输入布局
	std::vector<ComPtr<ID3D11InputLayout>> inputLayouts;

	// pass数量
	uint passCount;
public:
	// 根据已编译的着色器文件创建着色器对象
	Shader(const std::wstring &filePath, ID3D11Device* d3dDevice,bool createInputLayout = true);
	

	// 在使用该着色器绘制前必须调用的函数,表示应用某个Pass的绘制环境变量设置
	void UsePass(int passIndex, ID3D11DeviceContext* deviceContext) const;

	void UsePass(int passIndex, ID3D11InputLayout* inputLayout ,ID3D11DeviceContext* deviceContext) const;

#pragma region 为Shader设置各类变量的方法
	void SetMatrix4x4(const std::string& paramName, const float4x4 &value);
	void SetMatrix4x4(const std::string& paramName, XMMATRIX &value);
	void SetFloat4(const std::string& paramName, const float4 &value);
	void SetFloat3(const std::string& paramName, const float3 &value);
	void SetFloat2(const std::string& paramName, const float2 &value);
	void SetFloat(const std::string& paramName, float value);
	void SetUInt(const std::string& paramName, uint value);
	void SetVector(const std::string& paramName, XMVECTOR &value);
	void SetRawValue(const std::string& paramName, const void *pData, uint size);
	void SetShaderResource(const std::string& paramName, ID3D11ShaderResourceView* value);

	void SetTexture2D(const std::string& paramName,std::shared_ptr<Texture2D> texture2D);
	void SetCubeMap(const std::string& paramName, std::shared_ptr<CubeMap> cubeMap);

#pragma endregion

	// 获得当前Shader的pass数量
	uint GetPassCount() const;

	void Release();
};