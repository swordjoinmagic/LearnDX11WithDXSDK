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
	
	// 每秒中的tick数
	float ticksPerSecond = (float)anime->ticksPersecond;
	// 当前时间经过的tick数
	float timeInTicks = TimeInSeconds * ticksPerSecond;
	// 当前时间经过的tick数对动画片段总tick数进行取模
	float animationTime = fmod(timeInTicks,(float)anime->duration);

	if(transforms.size() == 0)
		transforms.resize(anime->channels.size());

	XMMATRIX identity = XMMatrixIdentity();

	// 从跟节点依次向下计算骨骼变换矩阵
	ReadNodeHeirarchy(
		transforms,animationIndedx,animationTime,
		booneRootNode,
		MathF::LoadXMMatrix(identity),
		booneRootNode->transformation.Inverse()
	);
}

/*
	依次从骨骼根节点开始,向下递归计算每个骨骼的变换矩阵,步骤如下:

	1. 确定要播放的动画片段
	2. 通过骨骼名称找到该骨骼在当前动画片段下的序列帧(也就是一串pos/quat/scal数据的数组)
	3. 根据当前时间,判断到达序列帧的哪两帧之间
	4. 在这两帧之间,对pos/quat/scal进行插值,计算变换矩阵
	5. 当前骨骼的最终变换矩阵(以列向量为标准) =
		根节点至模型局部空间的变换矩阵 *
		在该骨骼空间变换到跟节点骨骼的变换矩阵(简单来说 = 变换到跟节点矩阵 * 插值得到的变换矩阵) *
		骨骼偏移矩阵(即将顶点从局部空间变换至骨骼空间的变换矩阵)
	6. 向下继续递归计算其他骨骼的最终变换矩阵
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
		// 插值Pos
		if (animeNode->positions.size() == 1)
			pos = animeNode->positions[0].mValue;
		else {
			// 两个关键帧之间时间插值
			float deltaTime = (float)(animeNode->positions[pIndex+1].mTime - animeNode->positions[pIndex].mTime);
			// 计算当前时间在这两个关键帧之间的比值
			float factor = (animationTime - (float)animeNode->positions[pIndex].mTime) / deltaTime;
			pos = MathF::LerpVector3D(animeNode->positions[pIndex].mValue,animeNode->positions[pIndex+1].mValue,factor);
		}
		
		aiVector3D scal;
		// 插值scal
		if (animeNode->scalings.size() == 1)
			scal = animeNode->scalings[0].mValue;
		else {
			// 两个关键帧之间时间插值
			float deltaTime = (float)(animeNode->scalings[sIndex+1].mTime - animeNode->scalings[sIndex].mTime);
			// 计算当前时间在这两个关键帧之间的比值
			float factor = (animationTime - (float)animeNode->scalings[sIndex].mTime) / deltaTime;
			scal = MathF::LerpVector3D(animeNode->scalings[sIndex].mValue, animeNode->scalings[sIndex+1].mValue, factor);
		}

		aiQuaternion quat;
		// 插值quat
		if (animeNode->rotations.size() == 1)
			quat = animeNode->rotations[0].mValue;
		else {
			// 两个关键帧之间时间插值
			float deltaTime = (float)(animeNode->rotations[qIndex+1].mTime - animeNode->rotations[qIndex].mTime);
			// 计算当前时间在这两个关键帧之间的比值
			float factor = (animationTime - (float)animeNode->rotations[qIndex].mTime) / deltaTime;
			aiQuaternion::Interpolate(quat,animeNode->rotations[qIndex].mValue,animeNode->rotations[qIndex+1].mValue,factor);
		}

		aiMatrix4x4 rotationMatrix = aiMatrix4x4(quat.GetMatrix());

		nodeTransformation = XMMatrixScaling(scal.x, scal.y, scal.z) * MathF::LoadAssimpMatrix(rotationMatrix) * XMMatrixTranslation(pos.x,pos.y,pos.z);
	}

	// 至骨骼根节点的变换矩阵
	aiMatrix4x4 toRootNodeTransfrom = parentTransform * MathF::LoadXMMatrix(nodeTransformation);

	// 根据骨骼名称找到骨骼ID
	if (boneMapping.find(boneNode->nodeName) != boneMapping.end()) {
		int boneIndex = boneMapping[boneNode->nodeName];
		// 设置骨骼的最终变换矩阵(从右往左看) = 从根节点变换至顶点局部空间 * 从某个骨骼的骨骼空间变换至骨骼根节点空间 * 骨骼的偏移矩阵,将顶点从局部空间变换至骨骼空间
		transforms[boneIndex] = rootToLocalMatrix * toRootNodeTransfrom * boneOffsetMatrixs[boneIndex];
	}

	// 继续向下递归计算子节点的变换矩阵
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
	// 找到当前所处的pos数组的下标
	for (uint i = 0; i < nodeAnime->positions.size() - 1;i++) {
		if (animationTime < (float)nodeAnime->positions[i + 1].mTime) {
			*outPosIndexint = i;
			break;
		}
	}

	*outScalIndex = 0;
	// 找到当前所处的scal数组的下标
	for (uint i = 0; i < nodeAnime->scalings.size() - 1; i++) {
		if (animationTime < (float)nodeAnime->scalings[i + 1].mTime) {
			*outScalIndex = i;
			break;
		}
	}

	*outQuatIndex = 0;
	// 找到当前所处的quat数组的下标
	for (uint i = 0; i < nodeAnime->rotations.size() - 1; i++) {
		if (animationTime < (float)nodeAnime->rotations[i + 1].mTime) {
			*outQuatIndex = i;
			break;
		}
	}
}



void SkinMesh::Draw(std::shared_ptr<Shader> shader, ID3D11DeviceContext* context, D3D_PRIMITIVE_TOPOLOGY primitiveTopology, int usePass) {
	// 设置绘制模式
	context->IASetPrimitiveTopology(primitiveTopology);
	// 设置顶点和索引缓冲
	uint stride = sizeof(DefaultVertex);
	uint offset = 0;
	context->IASetVertexBuffers(0, 1, verticesBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(indicesBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	if (usePass != -1) {
		// 使用某个具体的Pass进行绘制
		shader->UsePass(usePass, context);
		context->DrawIndexed(indexCount, 0, 0);
	} else {
		// 使用全部Pass逐一绘制该Mesh(举例: 进行描边时,pass1绘制黑边,pass2绘制物体)
		for (uint i = 0; i < shader->GetPassCount(); i++) {
			shader->UsePass(i, context);
			context->DrawIndexed(indexCount, 0, 0);
		}
	}
}

void SkinMesh::Use(ID3D11DeviceContext* context) {
	// 设置顶点和索引缓冲
	uint stride = sizeof(SkinVertex);
	uint offset = 0;
	context->IASetVertexBuffers(0, 1, verticesBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(indicesBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}