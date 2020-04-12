#pragma once

#include "Mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "D3DUtils.h"
#include "DefaultVertex.h"
#include "SkinMesh.h"

class MeshLoader {
public:
	static std::shared_ptr<Mesh> LoadMesh(const std::string& path);

	static std::shared_ptr<SkinMesh> LoadSkinMesh(const std::string& path);

	static std::shared_ptr<SkinMesh::SkeletonNode> CopyNode(const aiNode* node,std::shared_ptr<SkinMesh::SkeletonNode> parent);
};