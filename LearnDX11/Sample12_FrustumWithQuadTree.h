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
	// �Ĳ����ڵ�,ÿ���ڵ㶼��һ���ڵ��Χ��(OBB),�Լ�һ����������(�а�Χ��/λ��/����/��ת/Mesh/Shader)�б�
	class QuadTreeNode {
	public:
		// �ڵ��Χ��
		XNA::OrientedBox nodeColliderBox;
		// �ýڵ��������Ϸ�����б�
		std::vector<std::shared_ptr<GameObject>> gameObjects;
		// �ýڵ�ĺ��ӽڵ�,���ĸ�,�ֱ���
		// ����/����/����/�����ĸ��ڵ�
		std::shared_ptr<QuadTreeNode> childNodes[4];
	};

	class QuadTree {
	public:
		// �Ĳ������ڵ�
		std::shared_ptr<QuadTreeNode> treeRoot;

		// �Ĳ���������
		uint maxDepth;

		// �����Ĳ�����Χ������
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

		// �����Ϸ����
		void AddGameObject(std::shared_ptr<GameObject> gameObject) {

			std::shared_ptr<QuadTreeNode> treeNode = treeRoot;
			auto gameObjectCollider = gameObject->colliderBox;

			// �Ӹ��ڵ㿪ʼ������Ϸ����͸��ڵ��ĸ��ӽڵ������ײ��⣬�������������
			// 1�� ֻ��ĳ���ӽڵ���ײ����ô������ӽڵ㣬�ظ��������̣�ֱ���ﵽ������
			// 2�� ������1���ӽڵ���ײ����ô��ǰ�ڵ㱣�����Ϸ���󣬽�����ӹ���
			for (uint i = 0; i < maxDepth; i++) {

				// ��¼��Ϸ������ĸ��ڵ㷢����ײ
				uint colliderIndex = -1;

				// �����ǰ�ڵ���ӽڵ�Ϊ�գ���ô�Զ���ʼ�������ӽڵ�
				if (treeNode->childNodes[0] == nullptr)
					InitChildNode(treeNode);

				// �͵�ǰ�ڵ���ĸ��ӽڵ������ײ���
				for (uint j = 0; j < 4; j++) {

					auto childNode = treeNode->childNodes[j];
					// ��ײ����

					// ����ռ����Ϸ������ײ��
					XNA::OrientedBox worldSpaceGCollider;

					// ����Ϸ�������ײ�ж��任������ռ�(����ڵ���ײ�еľֲ��ռ䱻����Ϊ������ռ�)
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
							// ������ӽڵ���ײ�з�����ײ����ǰ��Ϸ���󱣴�����ǰ�ڵ�
							treeNode->gameObjects.push_back(gameObject);
							return;
						} else {
							colliderIndex = j;
						}
					}
				}

				// �����Ĳ�������һ��ڵ�
				treeNode = treeNode->childNodes[colliderIndex];
			}

			// ��������İ˲�ڵ�Ĺ��ˣ�����Ϸ������ӵ���ǰ�ڵ���
			treeNode->gameObjects.push_back(gameObject);
		}

		// ��ʼ���ĸ��ӽڵ�
		void InitChildNode(std::shared_ptr<QuadTreeNode> node) {
			float3 center = node->nodeColliderBox.Center;
			float3 extends = node->nodeColliderBox.Extents;
			float4 quat = node->nodeColliderBox.Orientation;

			// ����
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

			// ����
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

			// ����
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

			// ����
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

			// �Ӹ��ڵ㿪ʼ������Ϸ����͸��ڵ��ĸ��ӽڵ������ײ��⣬�������������
			// 1�� ֻ��ĳ���ӽڵ���ײ����ô������ӽڵ㣬�ظ��������̣�ֱ���ﵽ������
			// 2�� ������1���ӽڵ���ײ����ô��ǰ�ڵ㱣�����Ϸ���󣬽�����ӹ���
			for (uint i = 0; i < maxDepth; i++) {

				// ��¼��Ϸ������ĸ��ڵ㷢����ײ
				uint colliderIndex = -1;

				// �����ǰ�ڵ���ӽڵ�Ϊ�գ���ô�Զ���ʼ�������ӽڵ�
				if (treeNode->childNodes[0] == nullptr)
					InitChildNode(treeNode);

				// �͵�ǰ�ڵ���ĸ��ӽڵ������ײ���
				for (uint j = 0; j < 4; j++) {

					auto childNode = treeNode->childNodes[j];
					// ��ײ����

					// ����ռ����Ϸ������ײ��
					XNA::OrientedBox worldSpaceGCollider;

					// ����Ϸ�������ײ�ж��任������ռ�(����ڵ���ײ�еľֲ��ռ䱻����Ϊ������ռ�)
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
							// ������ӽڵ���ײ�з�����ײ��˵��Ҫ���ҵ���Ϸ�����ڵ�ǰ�ڵ�
							return treeNode;							
						} else {
							colliderIndex = j;
						}
					}
				}

				// �����Ĳ�������һ��ڵ�
				treeNode = treeNode->childNodes[colliderIndex];
			}
			return treeNode;
		}

		void UpdateGameObjct(std::shared_ptr<GameObject> gameObject, float3 oldPos, float4 oldQuat, float3 oldScale){
			// �����ҵ�Ŀ��GameObject,�������ӽڵ���б���ɾȥ
			auto treeNode = FindGameObjectRegion(gameObject,oldPos,oldQuat,oldScale);
			auto iter = treeNode->gameObjects.begin();
			while (iter!=treeNode->gameObjects.end()) {
				if ((*iter) == gameObject) {
					break;
				}
				iter++;
			}
			treeNode->gameObjects.erase(iter);

			// �ڶ���,���°������GameObject
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