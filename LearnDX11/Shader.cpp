#include "Shader.h"

Shader::Shader(const std::wstring &filePath, ID3D11Device* d3dDevice, bool createInputLayout) {
	

	ID3DBlob* csBuffer = LoadCompiledShaderFromFile(filePath);
	D3DX11CreateEffectFromMemory(csBuffer->GetBufferPointer(),csBuffer->GetBufferSize(),0,d3dDevice,effect.GetAddressOf());

	// 仅使用第一个Technique
	technique = effect->GetTechniqueByIndex(0);

	// 获得第一个Technique的pass数
	D3DX11_TECHNIQUE_DESC techniqueDesc;
	technique->GetDesc(&techniqueDesc);
	passCount = techniqueDesc.Passes;	

	if (createInputLayout) {
		inputLayouts.resize(passCount);
		// 初始化输入布局
		D3D11_INPUT_ELEMENT_DESC inputElementsDesc[] = {
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT,0,24,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,36,D3D11_INPUT_PER_VERTEX_DATA,0}
		};
		for (uint i = 0; i < passCount; i++) {

			D3DX11_PASS_DESC passDesc;
			technique->GetPassByIndex(i)->GetDesc(&passDesc);

			d3dDevice->CreateInputLayout(
				inputElementsDesc,
				4,
				passDesc.pIAInputSignature,
				passDesc.IAInputSignatureSize,
				inputLayouts[i].GetAddressOf());
		}
	}
}

void Shader::UsePass(int passIndex, ID3D11DeviceContext* deviceContext) const{
	// 应用目标pass
	technique->GetPassByIndex(passIndex)->Apply(0,deviceContext);

	// 设置顶点输入布局
	deviceContext->IASetInputLayout(inputLayouts[passIndex].Get());
}

void Shader::UsePass(int passIndex, ID3D11InputLayout* inputLayout, ID3D11DeviceContext* deviceContext) const {
	// 应用目标pass
	technique->GetPassByIndex(passIndex)->Apply(0, deviceContext);

	// 设置顶点输入布局
	deviceContext->IASetInputLayout(inputLayout);
}


void Shader::SetFloat(const std::string &paramName, float value) {
	HR(effect->GetVariableByName(paramName.c_str())->SetRawValue(&value, 0, sizeof(value)));
}
void Shader::SetFloat2(const std::string &paramName, const float2 &value) {
	XMVECTOR vector = XMLoadFloat2(&value);
	HR(effect->GetVariableByName(paramName.c_str())->AsVector()->SetFloatVector(reinterpret_cast<float*>(&vector)));
}
void Shader::SetFloat3(const std::string &paramName, const float3 &value) {
	XMVECTOR vector = XMLoadFloat3(&value);
	HR(effect->GetVariableByName(paramName.c_str())->AsVector()->SetFloatVector(reinterpret_cast<float*>(&vector)));
}
void Shader::SetFloat4(const std::string &paramName, const float4 &value) {
	XMVECTOR vector = XMLoadFloat4(&value);
	HR(effect->GetVariableByName(paramName.c_str())->AsVector()->SetFloatVector(reinterpret_cast<float*>(&vector)));
}
void Shader::SetMatrix4x4(const std::string& paramName, const float4x4 &value) {
	XMMATRIX marix = XMLoadFloat4x4(&value);
	HR(effect->GetVariableByName(paramName.c_str())->AsMatrix()->SetMatrix(reinterpret_cast<float*>(&marix)));
}
void Shader::SetRawValue(const std::string& paramName, const void *pData, uint size) {
	HR(effect->GetVariableByName(paramName.c_str())->SetRawValue(pData, 0, size));	
}
void Shader::SetMatrix4x4(const std::string& paramName, XMMATRIX &value) {
	HR(effect->GetVariableByName(paramName.c_str())->AsMatrix()->SetMatrix(reinterpret_cast<float*>(&value)));
}
void Shader::SetVector(const std::string& paramName, XMVECTOR &value) {
	HR(effect->GetVariableByName(paramName.c_str())->AsVector()->SetFloatVector(reinterpret_cast<float*>(&value)));
}
void Shader::SetShaderResource(const std::string& paramName, ID3D11ShaderResourceView* value) {
	HR(effect->GetVariableByName(paramName.c_str())->AsShaderResource()->SetResource(value));
}

void Shader::SetTexture2D(const std::string& paramName,std::shared_ptr<Texture2D> texture2D) {
	SetShaderResource(paramName,texture2D->GetSRV().Get());
}
void Shader::SetCubeMap(const std::string& paramName, std::shared_ptr<CubeMap> cubeMap) {
	SetShaderResource(paramName, cubeMap->GetSRV().Get());
}



uint Shader::GetPassCount() const{
	return passCount;
}

void Shader::Release() {
	inputLayouts.clear();	
}

Shader::ShaderInputSign Shader::GetInputSign() {

	D3DX11_PASS_DESC passDesc;
	technique->GetPassByIndex(0)->GetDesc(&passDesc);


	ShaderInputSign inputSign;
	inputSign.pIAInputSignature = passDesc.pIAInputSignature;
	inputSign.IAInputSignatureSize = passDesc.IAInputSignatureSize;

	return inputSign;
}