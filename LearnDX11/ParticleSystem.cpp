#include "ParticleSystem.h"
#include "DefaultVertex.h"

ParticleSystem::ParticleSystem():
	mInitVB(0),mDrawVB(0),mStreamOutVB(0),mTexArraySRV(0),mRandomTexSRV(0) {

	mFirstRun = true;
	mGameTime = 0.0f;
	mTimeStep = 0.0f;
	mAge = 0.0f;

	mEyePosW = float3(0,0,0);
	mEmitPosW = float3(0,0,0);
	mEmitDirW = float3(0,1,0);

}

float ParticleSystem::GetAge()const {
	return mAge;
}

void ParticleSystem::SetEyePos(const float3& eyePosW) {
	mEyePosW = eyePosW;
}

void ParticleSystem::SetEmitPos(const float3& emitPosW) {
	mEmitPosW = emitPosW;
}

void ParticleSystem::SetEmitDir(const float3& emitDirW) {
	mEmitDirW = emitDirW;
}

void ParticleSystem::Init(
	ID3D11Device* device,
	std::shared_ptr<Shader> particleControlShader,
	std::shared_ptr<Shader> effectShader,
	ComPtr<ID3D11ShaderResourceView> texArraySRV,
	ComPtr<ID3D11ShaderResourceView> randomTexSRV,
	uint maxParticles) {

	this->particleControlShader = particleControlShader;
	mMaxParticles = maxParticles;
	this->effectShader = effectShader;
	mTexArraySRV = texArraySRV;
	mRandomTexSRV = randomTexSRV;

	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Particle) * 1;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	Particle p;
	p.Age = 0.0f;
	p.Type = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &p;

	// ������ʼ��Ⱦ����Ķ��㻺����
	HR(device->CreateBuffer(&vertexBufferDesc,&vinitData,mInitVB.GetAddressOf()));

	// �����������ڽ�����stream-Out���㻺����
	vertexBufferDesc.ByteWidth = sizeof(Particle) * mMaxParticles;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

	HR(device->CreateBuffer(&vertexBufferDesc,0,mDrawVB.GetAddressOf()));
	HR(device->CreateBuffer(&vertexBufferDesc,0,mStreamOutVB.GetAddressOf()));



	D3D11_INPUT_ELEMENT_DESC inputElementsDesc[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"VELOCITY",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"SIZE",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"AGE",0,DXGI_FORMAT_R32_FLOAT,0,32,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TYPE",0,DXGI_FORMAT_R32_UINT,0,36,D3D11_INPUT_PER_VERTEX_DATA,0}
	};

	Shader::ShaderInputSign inputSign = particleControlShader->GetInputSign();

	HR(device->CreateInputLayout(inputElementsDesc,5,inputSign.pIAInputSignature,inputSign.IAInputSignatureSize,particleIL.GetAddressOf()));
}

void ParticleSystem::Reset() {
	mFirstRun = true;
	mAge = 0.0f;
}

void ParticleSystem::Update(float deltaTime,float gameTime) {
	mTimeStep = deltaTime;
	mGameTime = gameTime;

	mAge += deltaTime;
}

void ParticleSystem::Draw(ID3D11DeviceContext* deviceContext,std::shared_ptr<SJM::Camera> camera) {
	auto view = camera->GetViewMatrix();
	auto proj = camera->GetProjMatrix();

	auto vpMatrix = view * proj;

#pragma region �������ӵ�����������
	
	particleControlShader->SetFloat3("emitPosW",mEmitPosW);
	particleControlShader->SetFloat3("emitDirW",mEmitDirW);
	particleControlShader->SetShaderResource("randomTex",mRandomTexSRV.Get());
	particleControlShader->SetFloat("gameTime",mGameTime);
	particleControlShader->SetFloat("timeStep",mTimeStep);

	deviceContext->IASetInputLayout(particleIL.Get());
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	uint stride = sizeof(Particle);
	uint offset = 0;

	// ����ǵ�һ�λ���,��ôʹ�ó�ʼ�Ķ��㻺��,����ʹ��stream-out����Ķ��㻺��
	if (mFirstRun) {
		deviceContext->IASetVertexBuffers(0,1,mInitVB.GetAddressOf(),&stride,&offset);
	} else {
		deviceContext->IASetVertexBuffers(0,1,mDrawVB.GetAddressOf(),&stride,&offset);
	}

	// �����λ��ƽ����������㻺������
	deviceContext->SOSetTargets(1,mStreamOutVB.GetAddressOf(),&offset);

	// ���л���,������������������׶�
	particleControlShader->UsePass(0,particleIL.Get(),deviceContext);

	if (mFirstRun) {
		deviceContext->Draw(1,0);
		mFirstRun = false;
	} else {
		deviceContext->DrawAuto();
	}

	// ���������Ŀ��
	ID3D11Buffer* noneBuffer[] = {nullptr};
	deviceContext->SOSetTargets(1,noneBuffer,&offset);

	// ��������Stream-Out Buffer
	mDrawVB.Swap(mStreamOutVB);

#pragma endregion

#pragma region �������ӵĻ������˶��켣

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->IASetInputLayout(particleIL.Get());

	// ���ö��㻺��
	deviceContext->IASetVertexBuffers(0,1,mDrawVB.GetAddressOf(),&stride,&offset);

	effectShader->SetFloat3("eyePosW",camera->pos);
	effectShader->SetMatrix4x4("vpMatrix",vpMatrix);
	effectShader->SetShaderResource("gTexArray",mTexArraySRV.Get());

	effectShader->UsePass(0,particleIL.Get(),deviceContext);

	deviceContext->DrawAuto();

#pragma endregion

}