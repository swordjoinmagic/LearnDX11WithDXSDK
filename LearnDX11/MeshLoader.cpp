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

	// 初始化网格顶点和索引
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

	// 初始化网格的骨骼以及顶点权重
	for (int i = 0; i < mesh->mNumBones; i++) {
		aiBone* bone = mesh->mBones[i];

		auto boneName = std::string(bone->mName.data);
		
		// 设置骨骼的偏移矩阵,即将顶点从局部空间变换至骨骼空间的变换矩阵
		skinMesh->boneOffsetMatrixs[i] = bone->mOffsetMatrix;
		
		// 设置骨骼名称和骨骼ID的映射
		skinMesh->boneMapping[boneName] = i;

		// 设置受该骨骼影响的顶点
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
			std::cout << "第" << i << "号顶点没有绑定骨骼" << std::endl;
	}


	// 设置骨骼根节点(用于播放动画时的变换操作)
	aiNode* rootNode = scene->mRootNode;
	skinMesh->booneRootNode = CopyNode(rootNode,nullptr);

	// 设置动画
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
