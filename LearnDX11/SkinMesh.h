#pragma once

#include "D3DUtils.h"
#include "DefaultVertex.h"
#include <map>
#include "assimp/scene.h"
#include "Shader.h"


/*
	��Ƥ������,��һ��������಻ͬ,
	SkinMesh�����й���
*/

class SkinMesh {
public:
	struct NodeAnime {
		std::string name;
		std::vector<aiVectorKey> positions;
		std::vector<aiQuatKey> rotations;
		std::vector<aiVectorKey> scalings;
	};
	struct Animation {
		double duration;
		double ticksPersecond;
		std::vector<std::shared_ptr<NodeAnime>> channels;
	};
	// ��������
	std::vector<std::shared_ptr<Animation>> animations;
public:	
	struct SkeletonNode {
		std::string nodeName;
		aiMatrix4x4 transformation;
		std::shared_ptr<SkeletonNode> parent;
		std::vector<std::shared_ptr<SkeletonNode>> children;
	};
	// �������ڵ�
	std::shared_ptr<SkeletonNode> booneRootNode;
private:
	ComPtr<ID3D11Buffer> verticesBuffer;
	ComPtr<ID3D11Buffer> indicesBuffer;	
public:
	std::vector<SkinVertex> vertics;
	std::vector<uint> indices;
	uint indexCount;

	// �������� - ����ID ӳ���
	std::map<std::string, int> boneMapping;	

	// ����ƫ������,��Ӧ����任��ÿ�������Ĺ����ռ�ı任����
	std::vector<aiMatrix4x4> boneOffsetMatrixs;

	SkinMesh(const std::vector<SkinVertex>& vertices, const std::vector<uint>& indices) :vertics(vertices), indices(indices) { indexCount = indices.size(); }
	void SetUpBuffer(ID3D11Device* device);

	/*
		���ݵ�ǰʱ���õ�ǰ������,�������������ձ任����
		animationIndedx - ���β��ŵĶ���ID
		TimeInSeconds - ��Ϸ��ʼ��ĿǰΪֹ������ʱ��(��λ:��)
		transform - ����ı任�����б�,�±�Ϊ������ID
	*/
	void BoneTransform(uint animationIndedx, float TimeInSeconds,std::vector<aiMatrix4x4>& transforms);

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
	void ReadNodeHeirarchy(
		std::vector<aiMatrix4x4>& transforms,
		uint animationIndex,
		float animationTime,
		std::shared_ptr<SkeletonNode> boneNode,
		const aiMatrix4x4& parentTransform,
		const aiMatrix4x4& rootToLocalMatrix);

	/*
		���ݹ������ƺ͵�ǰ���ŵĶ���Ƭ��,�ҵ��ù�����Ӧ������֡
	*/
	std::shared_ptr<NodeAnime> FindNode(std::shared_ptr<Animation> animation,std::string boneName);

	/*
		���ݵ�ǰ����ʱ��(��λ:��)�͵�ǰ����ĳ������������,
		�ҵ���ʱʱ����,�����˶����ĸ��±���
	*/
	void FindAnimeFramesIndex(float animationTime,std::shared_ptr<NodeAnime> nodeAnime,uint* outPosIndexint,uint* outScalIndex,uint* outQuatIndex);

	/*
		Ӧ�ö�Ӧ��Shader�Ը�Mesh���л���
		shader - ���λ���ʹ�õ�Shader
		context - ���Ʋ�����Ҫ�õ�context�������û�������
		primitiveTopology - ���λ���ʹ�õĻ���ģʽ(������ģʽ/����ģʽ��)
		usePass - ���λ���ʹ�õ�Pass,Ϊ-1ʱ,ʹ��ȫ��Pass���л���,��ָ��ʱ,��ʾʹ�þ���ĳ��Pass���л���
				  ����: ʹ�ø�˹ģ��ʱ,Ҫ�ֱ�ָ��pass1��ֱģ����pass2��ֱģ������������ģ��
	*/
	void Draw(std::shared_ptr<Shader> shader, ID3D11DeviceContext* context, D3D_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, int usePass = -1);

	void Use(ID3D11DeviceContext* context);
};