#include "MeshLoader.h"

#include <iostream>


std::shared_ptr<Mesh> MeshLoader::LoadMesh(const std::string& path) {
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path,
		aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		throw;
	}

	if (!scene->HasMeshes()) throw;

	aiMesh* mesh = scene->mMeshes[0];

	std::vector<DefaultVertex> vertices(mesh->mNumVertices);
	std::vector<uint> indices;

	for (uint i = 0; i < mesh->mNumVertices; i++) {
		DefaultVertex vertex;
		vertex.pos = float3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.normal = float3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		vertex.tangent = float3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
		vertex.texcoord = mesh->mTextureCoords[0] ? float2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : float2(0, 0);
		vertices[i] = vertex;
	}
	for (uint i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (uint j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<SkinMesh> MeshLoader::LoadSkinMesh(const std::string& path) {
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path,
		aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		throw;
	}
	if (!scene->HasMeshes()) throw;

	aiMesh* mesh = scene->mMeshes[0];

	// ��ʼ�����񶥵������
	std::vector<SkinVertex> vertices(mesh->mNumVertices);
	std::vector<uint> indices;

	for (uint i = 0; i < mesh->mNumVertices; i++) {
		SkinVertex vertex;
		vertex.pos = float3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.normal = float3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		vertex.tangent = float3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
		vertex.texcoord = mesh->mTextureCoords[0] ? float2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : float2(0, 0);
		vertices[i] = vertex;
	}
	for (uint i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (uint j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	auto skinMesh = std::make_shared<SkinMesh>(vertices,indices);

	skinMesh->boneOffsetMatrixs.resize(mesh->mNumBones);

	// ��ʼ������Ĺ����Լ�����Ȩ��
	for (int i = 0; i < mesh->mNumBones; i++) {
		aiBone* bone = mesh->mBones[i];

		auto boneName = std::string(bone->mName.data);
		
		// ���ù�����ƫ�ƾ���,��������Ӿֲ��ռ�任�������ռ�ı任����
		skinMesh->boneOffsetMatrixs[i] = bone->mOffsetMatrix;
		
		// ���ù������ƺ͹���ID��ӳ��
		skinMesh->boneMapping[boneName] = i;

		// �����ܸù���Ӱ��Ķ���
		for (uint j = 0; j < bone->mNumWeights; j++) {
			aiVertexWeight vertexWeight = bone->mWeights[j];

			int vID = vertexWeight.mVertexId;
			float vWeight = vertexWeight.mWeight;

			for (int k = 0; k < 4;k++) {
				if (skinMesh->vertics[vID].weights[k] == 0) {
					skinMesh->vertics[vID].weights[k] = vWeight;
					skinMesh->vertics[vID].boneID[k] = i;
					break;
				}
			}
		}
	}

	for (int i = 0; i < skinMesh->vertics.size();i++) {
		auto v = skinMesh->vertics[i];
		if (v.weights[0] == 0)
			std::cout << "��" << i << "�Ŷ���û�а󶨹���" << std::endl;
	}


	// ���ù������ڵ�(���ڲ��Ŷ���ʱ�ı任����)
	aiNode* rootNode = scene->mRootNode;
	skinMesh->booneRootNode = CopyNode(rootNode,nullptr);

	// ���ö���
	skinMesh->animations.resize(scene->mNumAnimations);
	for (uint i = 0; i < scene->mNumAnimations;i++) {

		auto anime = std::make_shared<SkinMesh::Animation>();

		auto aiAnimation = scene->mAnimations[i];
		
		anime->duration = aiAnimation->mDuration;
		anime->ticksPersecond = aiAnimation->mTicksPerSecond;

		anime->channels.resize(aiAnimation->mNumChannels);
		for (uint j = 0; j < aiAnimation->mNumChannels; j++) {
			auto nodeAnime = std::make_shared<SkinMesh::NodeAnime>();
			
			auto aiNodeAnim = aiAnimation->mChannels[j];

			nodeAnime->name = std::string(aiNodeAnim->mNodeName.data);
			
			nodeAnime->positions.resize(aiNodeAnim->mNumPositionKeys);
			for (uint k = 0; k < aiNodeAnim->mNumPositionKeys; k++) {
				nodeAnime->positions[k] = aiNodeAnim->mPositionKeys[k];				
			}
			nodeAnime->rotations.resize(aiNodeAnim->mNumRotationKeys);
			for (uint k = 0; k < aiNodeAnim->mNumRotationKeys; k++) {
				nodeAnime->rotations[k] = aiNodeAnim->mRotationKeys[k];
			}
			nodeAnime->scalings.resize(aiNodeAnim->mNumScalingKeys);
			for (uint k = 0; k < aiNodeAnim->mNumScalingKeys; k++) {
				nodeAnime->scalings[k] = aiNodeAnim->mScalingKeys[k];
			}

			anime->channels[j] = nodeAnime;
		}
		
		skinMesh->animations[i] = anime;
	}

	return skinMesh;
}

std::shared_ptr<SkinMesh::SkeletonNode> MeshLoader::CopyNode(const aiNode* node, std::shared_ptr<SkinMesh::SkeletonNode> parent) {
	auto skeletonNode = std::make_shared<SkinMesh::SkeletonNode>();

	skeletonNode->nodeName = std::string(node->mName.data);
	skeletonNode->parent = parent;
	skeletonNode->transformation = node->mTransformation;

	skeletonNode->children.resize(node->mNumChildren);
	for (int i = 0; i < node->mNumChildren;i++) {
		skeletonNode->children[i] = CopyNode(node->mChildren[i],skeletonNode);
	}

	return skeletonNode;
}
