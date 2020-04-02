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

ComPtr<ID3D11ShaderResourceView> d3dHelper::CreateTexture2DArraySRV(
	ID3D11Device* device, ID3D11DeviceContext* context,
	std::vector<std::wstring>& filenames,
	DXGI_FORMAT format,
	UINT filter,
	UINT mipFilter) {

	UINT size = filenames.size();

	std::vector<ComPtr<ID3D11Texture2D>> srcTex(size);
	for (UINT i = 0; i < size; ++i) {
		D3DX11_IMAGE_LOAD_INFO loadInfo;

		loadInfo.Width = D3DX11_FROM_FILE;
		loadInfo.Height = D3DX11_FROM_FILE;
		loadInfo.Depth = D3DX11_FROM_FILE;
		loadInfo.FirstMipLevel = 0;
		loadInfo.MipLevels = D3DX11_FROM_FILE;
		loadInfo.Usage = D3D11_USAGE_STAGING;
		loadInfo.BindFlags = 0;
		loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		loadInfo.MiscFlags = 0;
		loadInfo.Format = format;
		loadInfo.Filter = filter;
		loadInfo.MipFilter = mipFilter;
		loadInfo.pSrcInfo = 0;

		HR(D3DX11CreateTextureFromFile(device, filenames[i].c_str(),
			&loadInfo, 0, (ID3D11Resource**)srcTex[i].GetAddressOf(), 0));
	}

	D3D11_TEXTURE2D_DESC texElementDesc;
	srcTex[0]->GetDesc(&texElementDesc);

	D3D11_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width = texElementDesc.Width;
	texArrayDesc.Height = texElementDesc.Height;
	texArrayDesc.MipLevels = texElementDesc.MipLevels;
	texArrayDesc.ArraySize = size;
	texArrayDesc.Format = texElementDesc.Format;
	texArrayDesc.SampleDesc.Count = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags = 0;
	texArrayDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> texArray = 0;
	HR(device->CreateTexture2D(&texArrayDesc, 0, texArray.GetAddressOf()));

	for (UINT texElement = 0; texElement < size; ++texElement) {
		// for each mipmap level...
		for (UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel) {
			D3D11_MAPPED_SUBRESOURCE mappedTex2D;
			HR(context->Map(srcTex[texElement].Get(), mipLevel, D3D11_MAP_READ, 0, &mappedTex2D));

			context->UpdateSubresource(texArray.Get(),
				D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels),
				0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);

			context->Unmap(srcTex[texElement].Get(), mipLevel);
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = size;

	ComPtr<ID3D11ShaderResourceView> texArraySRV;
	HR(device->CreateShaderResourceView(texArray.Get(), &viewDesc, texArraySRV.GetAddressOf()));

	return texArraySRV;
}

ComPtr<ID3D11ShaderResourceView> d3dHelper::CreateRandomTexture1DSRV(ID3D11Device* device) {
	// 
// Create the random data.
//
	float4 randomValues[1024];

	for (int i = 0; i < 1024; ++i) {
		randomValues[i].x = MathF::RandF(-1.0f, 1.0f);
		randomValues[i].y = MathF::RandF(-1.0f, 1.0f);
		randomValues[i].z = MathF::RandF(-1.0f, 1.0f);
		randomValues[i].w = MathF::RandF(-1.0f, 1.0f);
	}

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = randomValues;
	initData.SysMemPitch = 1024 * sizeof(float4);
	initData.SysMemSlicePitch = 0;

	//
	// Create the texture.
	//
	D3D11_TEXTURE1D_DESC texDesc;
	texDesc.Width = 1024;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	texDesc.ArraySize = 1;

	ComPtr<ID3D11Texture1D> randomTex;
	HR(device->CreateTexture1D(&texDesc, &initData, randomTex.GetAddressOf()));

	//
	// Create the resource view.
	//
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
	viewDesc.Texture1D.MostDetailedMip = 0;

	ComPtr<ID3D11ShaderResourceView> randomTexSRV;
	HR(device->CreateShaderResourceView(randomTex.Get(), &viewDesc, randomTexSRV.GetAddressOf()));

	return randomTexSRV;
}