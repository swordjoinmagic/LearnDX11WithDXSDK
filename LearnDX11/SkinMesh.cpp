#include "SkinMesh.h"

#include "MathF.h"

void SkinMesh::SetUpBuffer(ID3D11Device* device) {
	HR(CreateBuffer(
		D3D11_USAGE_IMMUTABLE,
		sizeof(SkinVertex) * vertics.size(),
		D3D11_BIND_VERTEX_BUFFER,
		0,0,
		sizeof(SkinVertex),
		&vertics[0],
		device,
		verticesBuffer.GetAddressOf()
	));

	HR(CreateBuffer(
		D3D11_USAGE_IMMUTABLE,
		sizeof(uint)*indices.size(),
		D3D11_BIND_INDEX_BUFFER,
		0, 0,
		sizeof(uint),
		&indices[0],
		device,
		indicesBuffer.GetAddressOf()
	));

	indexCount = indices.size();
}

void SkinMesh::BoneTransform(uint animationIndedx,float TimeInSeconds, std::vector<aiMatrix4x4>& transforms) {
	auto anime = animations[animationIndedx];
	
	// ÿ���е�tick��
	float ticksPerSecond = (float)anime->ticksPersecond;
	// ��ǰʱ�侭����tick��
	float timeInTicks = TimeInSeconds * ticksPerSecond;
	// ��ǰʱ�侭����tick���Զ���Ƭ����tick������ȡģ
	float animationTime = fmod(timeInTicks,(float)anime->duration);

	if(transforms.size() == 0)
		transforms.resize(anime->channels.size());

	XMMATRIX identity = XMMatrixIdentity();

	// �Ӹ��ڵ��������¼�������任����
	ReadNodeHeirarchy(
		transforms,animationIndedx,animationTime,
		booneRootNode,
		MathF::LoadXMMatrix(identity),
		booneRootNode->transformation.Inverse()
	);
}

/*
	���δӹ������ڵ㿪ʼ,���µݹ����ÿ�������ı任����,��������:

	1. ȷ��Ҫ���ŵĶ���Ƭ��
	2. ͨ�����������ҵ��ù����ڵ�ǰ����Ƭ���µ�����֡(Ҳ����һ��pos/quat/scal���ݵ�����)
	3. ���ݵ�ǰʱ��,�жϵ�������֡������֮֡��
	4. ������֮֡��,��pos/quat/scal���в�ֵ,����任����
	5. ��ǰ���������ձ任����(��������Ϊ��׼) =
		���ڵ���ģ�;ֲ��ռ�ı任���� *
		�ڸù����ռ�任�����ڵ�����ı任����(����˵ = �任�����ڵ���� * ��ֵ�õ��ı任����) *
		����ƫ�ƾ���(��������Ӿֲ��ռ�任�������ռ�ı任����)
	6. ���¼����ݹ�����������������ձ任����
*/
void SkinMesh::ReadNodeHeirarchy(
	std::vector<aiMatrix4x4>& transforms,
	uint animationIndex,
	float animationTime, 
	std::shared_ptr<SkeletonNode> boneNode, 
	const aiMatrix4x4& parentTransform,
	const aiMatrix4x4& rootToLocalMatrix
) {

	auto anime = animations[animationIndex];
	auto animeNode = FindNode(anime,boneNode->nodeName);

	XMMATRIX nodeTransformation = MathF::LoadAssimpMatrix(boneNode->transformation);

	if(animeNode) {
		
		uint pIndex, sIndex, qIndex;
		FindAnimeFramesIndex(animationTime,animeNode,&pIndex,&sIndex,&qIndex);

		aiVector3D pos;
		// ��ֵPos
		if (animeNode->positions.size() == 1)
			pos = animeNode->positions[0].mValue;
		else {
			// �����ؼ�֮֡��ʱ���ֵ
			float deltaTime = (float)(animeNode->positions[pIndex+1].mTime - animeNode->positions[pIndex].mTime);
			// ���㵱ǰʱ�����������ؼ�֮֡��ı�ֵ
			float factor = (animationTime - (float)animeNode->positions[pIndex].mTime) / deltaTime;
			pos = MathF::LerpVector3D(animeNode->positions[pIndex].mValue,animeNode->positions[pIndex+1].mValue,factor);
		}
		
		aiVector3D scal;
		// ��ֵscal
		if (animeNode->scalings.size() == 1)
			scal = animeNode->scalings[0].mValue;
		else {
			// �����ؼ�֮֡��ʱ���ֵ
			float deltaTime = (float)(animeNode->scalings[sIndex+1].mTime - animeNode->scalings[sIndex].mTime);
			// ���㵱ǰʱ�����������ؼ�֮֡��ı�ֵ
			float factor = (animationTime - (float)animeNode->scalings[sIndex].mTime) / deltaTime;
			scal = MathF::LerpVector3D(animeNode->scalings[sIndex].mValue, animeNode->scalings[sIndex+1].mValue, factor);
		}

		aiQuaternion quat;
		// ��ֵquat
		if (animeNode->rotations.size() == 1)
			quat = animeNode->rotations[0].mValue;
		else {
			// �����ؼ�֮֡��ʱ���ֵ
			float deltaTime = (float)(animeNode->rotations[qIndex+1].mTime - animeNode->rotations[qIndex].mTime);
			// ���㵱ǰʱ�����������ؼ�֮֡��ı�ֵ
			float factor = (animationTime - (float)animeNode->rotations[qIndex].mTime) / deltaTime;
			aiQuaternion::Interpolate(quat,animeNode->rotations[qIndex].mValue,animeNode->rotations[qIndex+1].mValue,factor);
		}

		aiMatrix4x4 rotationMatrix = aiMatrix4x4(quat.GetMatrix());

		nodeTransformation = XMMatrixScaling(scal.x, scal.y, scal.z) * MathF::LoadAssimpMatrix(rotationMatrix) * XMMatrixTranslation(pos.x,pos.y,pos.z);
	}

	// ���������ڵ�ı任����
	aiMatrix4x4 toRootNodeTransfrom = parentTransform * MathF::LoadXMMatrix(nodeTransformation);

	// ���ݹ��������ҵ�����ID
	if (boneMapping.find(boneNode->nodeName) != boneMapping.end()) {
		int boneIndex = boneMapping[boneNode->nodeName];
		// ���ù��������ձ任����(��������) = �Ӹ��ڵ�任������ֲ��ռ� * ��ĳ�������Ĺ����ռ�任���������ڵ�ռ� * ������ƫ�ƾ���,������Ӿֲ��ռ�任�������ռ�
		transforms[boneIndex] = rootToLocalMatrix * toRootNodeTransfrom * boneOffsetMatrixs[boneIndex];
	}

	// �������µݹ�����ӽڵ�ı任����
	for (uint i = 0; i < boneNode->children.size();i++) {
		ReadNodeHeirarchy(
			transforms,
			animationIndex,
			animationTime,
			boneNode->children[i],
			toRootNodeTransfrom,
			rootToLocalMatrix
		);
	}
}

std::shared_ptr<SkinMesh::NodeAnime> SkinMesh::FindNode(std::shared_ptr<Animation> animation, std::string boneName) {
	for (auto node : animation->channels) {
		if (node->name == boneName)
			return node;
	}
	return nullptr;
}

void SkinMesh::FindAnimeFramesIndex(float animationTime, std::shared_ptr<NodeAnime> nodeAnime, uint* outPosIndexint, uint* outScalIndex, uint* outQuatIndex) {

	*outPosIndexint = 0;
	// �ҵ���ǰ������pos������±�
	for (uint i = 0; i < nodeAnime->positions.size() - 1;i++) {
		if (animationTime < (float)nodeAnime->positions[i + 1].mTime) {
			*outPosIndexint = i;
			break;
		}
	}

	*outScalIndex = 0;
	// �ҵ���ǰ������scal������±�
	for (uint i = 0; i < nodeAnime->scalings.size() - 1; i++) {
		if (animationTime < (float)nodeAnime->scalings[i + 1].mTime) {
			*outScalIndex = i;
			break;
		}
	}

	*outQuatIndex = 0;
	// �ҵ���ǰ������quat������±�
	for (uint i = 0; i < nodeAnime->rotations.size() - 1; i++) {
		if (animationTime < (float)nodeAnime->rotations[i + 1].mTime) {
			*outQuatIndex = i;
			break;
		}
	}
}



void SkinMesh::Draw(std::shared_ptr<Shader> shader, ID3D11DeviceContext* context, D3D_PRIMITIVE_TOPOLOGY primitiveTopology, int usePass) {
	// ���û���ģʽ
	context->IASetPrimitiveTopology(primitiveTopology);
	// ���ö������������
	uint stride = sizeof(DefaultVertex);
	uint offset = 0;
	context->IASetVertexBuffers(0, 1, verticesBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(indicesBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	if (usePass != -1) {
		// ʹ��ĳ�������Pass���л���
		shader->UsePass(usePass, context);
		context->DrawIndexed(indexCount, 0, 0);
	} else {
		// ʹ��ȫ��Pass��һ���Ƹ�Mesh(����: �������ʱ,pass1���ƺڱ�,pass2��������)
		for (uint i = 0; i < shader->GetPassCount(); i++) {
			shader->UsePass(i, context);
			context->DrawIndexed(indexCount, 0, 0);
		}
	}
}

void SkinMesh::Use(ID3D11DeviceContext* context) {
	// ���ö������������
	uint stride = sizeof(SkinVertex);
	uint offset = 0;
	context->IASetVertexBuffers(0, 1, verticesBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(indicesBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}