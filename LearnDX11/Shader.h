#pragma once
#include "D3DUtils.h"
#include "Texture2D.h"
#include "CubeMap.h"

/*
	��ɫ����,���ڷ�װ��ɫ������,��װ�����в���:
		1. ���ø��ೣ������������
		2. �ڻ���ǰ�ĸ��໷������,�����벼��/����ģʽ��

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

	// ��Ӧ����Pass�����벼��
	std::vector<ComPtr<ID3D11InputLayout>> inputLayouts;

	// pass����
	uint passCount;
public:
	// �����ѱ������ɫ���ļ�������ɫ������
	Shader(const std::wstring &filePath, ID3D11Device* d3dDevice,bool createInputLayout = true);
	

	// ��ʹ�ø���ɫ������ǰ������õĺ���,��ʾӦ��ĳ��Pass�Ļ��ƻ�����������
	void UsePass(int passIndex, ID3D11DeviceContext* deviceContext) const;

	void UsePass(int passIndex, ID3D11InputLayout* inputLayout ,ID3D11DeviceContext* deviceContext) const;

#pragma region ΪShader���ø�������ķ���
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

	// ��õ�ǰShader��pass����
	uint GetPassCount() const;

	void Release();
};