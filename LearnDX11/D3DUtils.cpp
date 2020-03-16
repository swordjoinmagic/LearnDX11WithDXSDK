#include "D3DUtils.h"
#include <D3Dcompiler.h>
#include "MathF.h"

ID3DBlob* LoadCompiledShaderFromFile(const std::wstring& fileName) {
	std::ifstream fin(fileName, std::ios::binary);
	fin.seekg(0, std::ios_base::end);
	std::ifstream::pos_type size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);

	ID3DBlob* blob;
	HR(D3DCreateBlob(size, &blob));
	
	fin.read((char*)blob->GetBufferPointer(), size);
	fin.close();

	return blob;
}

std::vector<char> LoadCompiledEffectFile(const std::wstring& fileName,int& size) {
	std::ifstream fin(fileName,std::ios::binary);
	fin.seekg(0,std::ios_base::end);
	size = (int)fin.tellg();
	fin.seekg(0,std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0],size);
	fin.close();

	return compiledShader;
}

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
) {
	D3D11_BUFFER_DESC bufferDesc = {
		byteWidth,
		usage,		
		bindFlag,
		cpuAccessFlags,
		miscFlags,
		structureByteStride
	};

	D3D11_SUBRESOURCE_DATA bufferData;
	bufferData.pSysMem = data;
	
	if(data)
		return d3dDevice->CreateBuffer(&bufferDesc,&bufferData,buffer);
	else {
		return d3dDevice->CreateBuffer(&bufferDesc, 0, buffer);
	}
}


XMMATRIX GetInverseMatrix(XMMATRIX A) {
	XMVECTOR det = XMMatrixDeterminant(A);
	return XMMatrixInverse(&det,A);	
}