#pragma once

#include "d3dApp.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture2D.h"
#include "xnacollision.h"
#include "Light.h"


class Sample12 : public D3DApp{
public:
	class GameObject {
	public:
		float3 pos;
		float3 rotation;
		float3 scale;
		XNA::OrientedBox colliderBox;
	};
	// 四叉树节点,每个节点都有一个节点包围盒(OBB),以及一个包含物体(有包围盒/位置/缩放/旋转/Mesh/Shader)列表
	class QuadTreeNode {
	public:
		// 节点包围盒
		XNA::OrientedBox nodeColliderBox;
		// 该节点包含的游戏对象列表
		std::vector<std::shared_ptr<GameObject>> gameObjects;
		// 该节点的孩子节点,共四个,分别是
		// 左上/右上/左下/右下四个节点
		std::shared_ptr<QuadTreeNode> childNodes[4];
	};

	class QuadTree {
	public:
		// 四叉树根节点
		std::shared_ptr<QuadTreeNode> treeRoot;

		// 四叉树最大层数
		uint maxDepth;

		// 整个四叉树包围的中心
		float3 center;
		float3 extends;
		float4 quat;

		QuadTree(float3 center, float3 extends, float4 quat) {
			treeRoot = std::make_shared<QuadTreeNode>();
			treeRoot->nodeColliderBox.Center = center;
			treeRoot->nodeColliderBox.Extents = extends;
			treeRoot->nodeColliderBox.Orientation = quat;

			this->center = center;
			this->extends = extends;
			this->quat = quat;
		}

		// 添加游戏物体
		void AddGameObject(std::shared_ptr<GameObject> gameObject) {

			std::shared_ptr<QuadTreeNode> treeNode = treeRoot;
			auto gameObjectCollider = gameObject->colliderBox;

			// 从根节点开始，让游戏对象和根节点四个子节点进行碰撞检测，会有以下情况：
			// 1： 只跟某个子节点碰撞，那么进入该子节点，重复上述过程，直到达到最大层数
			// 2： 跟超过1个子节点碰撞，那么当前节点保存该游戏对象，结束添加过程
			for (uint i = 0; i < maxDepth; i++) {

				// 记录游戏对象和哪个节点发生碰撞
				uint colliderIndex = -1;

				// 如果当前节点的子节点为空，那么自动初始化它的子节点
				if (treeNode->childNodes[0] == nullptr)
					InitChildNode(treeNode);

				// 和当前节点的四个子节点进行碰撞检测
				for (uint j = 0; j < 4; j++) {

					auto childNode = treeNode->childNodes[j];
					// 碰撞测试

					// 世界空间的游戏对象碰撞盒
					XNA::OrientedBox worldSpaceGCollider;

					// 将游戏对象的碰撞盒都变换至世界空间(这里节点碰撞盒的局部空间被我设为了世界空间)
					auto model =
						XMMatrixScaling(gameObject->scale.x, gameObject->scale.y, gameObject->scale.z) *
						XMMatrixRotationX(MathF::Radians(gameObject->rotation.x)) * XMMatrixRotationY(MathF::Radians(gameObject->rotation.y)) * XMMatrixRotationZ(MathF::Radians(gameObject->rotation.z)) *
						XMMatrixTranslation(gameObject->pos.x, gameObject->pos.y, gameObject->pos.z);

					XMVECTOR translation, quat, scale;
					XMMatrixDecompose(&scale, &quat, &translation, model);
					quat = XMQuaternionNormalize(quat);
					XNA::TransformOrientedBox(&worldSpaceGCollider, &gameObjectCollider, XMVectorGetX(scale), quat, translation);

					bool intersectResult = XNA::IntersectOrientedBoxOrientedBox(&worldSpaceGCollider, &childNode->nodeColliderBox);

					if (intersectResult) {
						if (colliderIndex != -1) {
							// 跟多个子节点碰撞盒发生碰撞，当前游戏对象保存至当前节点
							treeNode->gameObjects.push_back(gameObject);
							return;
						} else {
							colliderIndex = j;
						}
					}
				}

				// 进入四叉树的下一层节点
				treeNode = treeNode->childNodes[colliderIndex];
			}

			// 经过上面的八层节点的过滤，将游戏对象添加到当前节点上
			treeNode->gameObjects.push_back(gameObject);
		}

		// 初始化四个子节点
		void InitChildNode(std::shared_ptr<QuadTreeNode> node) {
			float3 center = node->nodeColliderBox.Center;
			float3 extends = node->nodeColliderBox.Extents;
			float4 quat = node->nodeColliderBox.Orientation;

			// 左上
			auto leftUp = std::make_shared<QuadTreeNode>();
			leftUp->nodeColliderBox.Center = float3(
				center.x - extends.x / 2,
				center.y,
				center.z + extends.z / 2
			);
			leftUp->nodeColliderBox.Extents = float3(
				extends.x / 2,
				extends.y,
				extends.z / 2
			);
			leftUp->nodeColliderBox.Orientation = quat;

			// 右上
			auto rightUp = std::make_shared<QuadTreeNode>();
			rightUp->nodeColliderBox.Center = float3(
				center.x + extends.x / 2,
				center.y,
				center.z + extends.z / 2
			);
			rightUp->nodeColliderBox.Extents = float3(
				extends.x / 2,
				extends.y,
				extends.z / 2
			);
			rightUp->nodeColliderBox.Orientation = quat;

			// 左下
			auto leftDown = std::make_shared<QuadTreeNode>();
			leftDown->nodeColliderBox.Center = float3(
				center.x - extends.x / 2,
				center.y,
				center.z - extends.z / 2
			);
			leftDown->nodeColliderBox.Extents = float3(
				extends.x / 2,
				extends.y,
				extends.z / 2
			);
			leftDown->nodeColliderBox.Orientation = quat;

			// 右下
			auto rightDown = std::make_shared<QuadTreeNode>();
			rightDown->nodeColliderBox.Center = float3(
				center.x + extends.x / 2,
				center.y,
				center.z - extends.z / 2
			);
			rightDown->nodeColliderBox.Extents = float3(
				extends.x / 2,
				extends.y,
				extends.z / 2
			);
			rightDown->nodeColliderBox.Orientation = quat;


			node->childNodes[0] = leftUp;
			node->childNodes[1] = rightUp;
			node->childNodes[2] = leftDown;
			node->childNodes[3] = rightDown;
		}

		std::shared_ptr<QuadTreeNode> FindGameObjectRegion(std::shared_ptr<GameObject> gameObject,float3 oldPos,float4 oldQuat,float3 oldScale) {
			std::shared_ptr<QuadTreeNode> treeNode = treeRoot;
			auto gameObjectCollider = gameObject->colliderBox;

			// 从根节点开始，让游戏对象和根节点四个子节点进行碰撞检测，会有以下情况：
			// 1： 只跟某个子节点碰撞，那么进入该子节点，重复上述过程，直到达到最大层数
			// 2： 跟超过1个子节点碰撞，那么当前节点保存该游戏对象，结束添加过程
			for (uint i = 0; i < maxDepth; i++) {

				// 记录游戏对象和哪个节点发生碰撞
				uint colliderIndex = -1;

				// 如果当前节点的子节点为空，那么自动初始化它的子节点
				if (treeNode->childNodes[0] == nullptr)
					InitChildNode(treeNode);

				// 和当前节点的四个子节点进行碰撞检测
				for (uint j = 0; j < 4; j++) {

					auto childNode = treeNode->childNodes[j];
					// 碰撞测试

					// 世界空间的游戏对象碰撞盒
					XNA::OrientedBox worldSpaceGCollider;

					// 将游戏对象的碰撞盒都变换至世界空间(这里节点碰撞盒的局部空间被我设为了世界空间)
					auto model =
						XMMatrixScaling(gameObject->scale.x, gameObject->scale.y, gameObject->scale.z) *
						XMMatrixRotationX(MathF::Radians(gameObject->rotation.x)) * XMMatrixRotationY(MathF::Radians(gameObject->rotation.y)) * XMMatrixRotationZ(MathF::Radians(gameObject->rotation.z)) *
						XMMatrixTranslation(gameObject->pos.x, gameObject->pos.y, gameObject->pos.z);

					XMVECTOR translation, quat, scale;
					XMMatrixDecompose(&scale, &quat, &translation, model);
					quat = XMQuaternionNormalize(quat);
					XNA::TransformOrientedBox(&worldSpaceGCollider, &gameObjectCollider, XMVectorGetX(scale), quat, translation);

					bool intersectResult = XNA::IntersectOrientedBoxOrientedBox(&worldSpaceGCollider, &childNode->nodeColliderBox);

					if (intersectResult) {
						if (colliderIndex != -1) {
							// 跟多个子节点碰撞盒发生碰撞，说明要查找的游戏对象在当前节点
							return treeNode;							
						} else {
							colliderIndex = j;
						}
					}
				}

				// 进入四叉树的下一层节点
				treeNode = treeNode->childNodes[colliderIndex];
			}
			return treeNode;
		}

		void UpdateGameObjct(std::shared_ptr<GameObject> gameObject, float3 oldPos, float4 oldQuat, float3 oldScale){
			// 首先找到目标GameObject,并将它从节点的列表中删去
			auto treeNode = FindGameObjectRegion(gameObject,oldPos,oldQuat,oldScale);
			auto iter = treeNode->gameObjects.begin();
			while (iter!=treeNode->gameObjects.end()) {
				if ((*iter) == gameObject) {
					break;
				}
				iter++;
			}
			treeNode->gameObjects.erase(iter);

			// 第二步,重新安插这个GameObject
			AddGameObject(gameObject);
		}
	};
private:
	std::vector<std::shared_ptr<GameObject>> gameObjectList;
	
	std::shared_ptr<Shader> boxShader;
	std::shared_ptr<Mesh> boxMesh;
	std::shared_ptr<Texture2D> boxTexture;
	std::shared_ptr<Texture2D> planeTexture;

	Light light;

	XNA::Frustum cameraFrustum;

	std::shared_ptr<QuadTree> quadTree;

	std::vector<std::shared_ptr<GameObject>> needRenderGameObjects;
	uint renderSize;

	ComPtr<ID3D11RasterizerState> lineMode;
	std::shared_ptr<Shader> colliderShader;
public:
	Sample12(HINSTANCE hInstance) : D3DApp(hInstance) {}
	void OnStart() override;
	void UpdateScene(float deltaTime) override;
	void Render();
	void QuadTreeCulling(std::shared_ptr<Sample12::QuadTreeNode> treeNode, XMMATRIX viewMatrix,  uint depth);
	void RenderQuadTree(std::shared_ptr<QuadTree> quadTree);
	void RenderQuadTreeNode(std::shared_ptr<QuadTreeNode> quadTreeNode);
};