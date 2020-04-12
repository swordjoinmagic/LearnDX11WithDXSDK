#pragma once

#include "D3DUtils.h"
#include "DefaultVertex.h"
#include <map>
#include "assimp/scene.h"
#include "Shader.h"


/*
	蒙皮网格类,与一般的网格类不同,
	SkinMesh类中有骨骼
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
	// 动画数组
	std::vector<std::shared_ptr<Animation>> animations;
public:	
	struct SkeletonNode {
		std::string nodeName;
		aiMatrix4x4 transformation;
		std::shared_ptr<SkeletonNode> parent;
		std::vector<std::shared_ptr<SkeletonNode>> children;
	};
	// 骨骼根节点
	std::shared_ptr<SkeletonNode> booneRootNode;
private:
	ComPtr<ID3D11Buffer> verticesBuffer;
	ComPtr<ID3D11Buffer> indicesBuffer;	
public:
	std::vector<SkinVertex> vertics;
	std::vector<uint> indices;
	uint indexCount;

	// 骨骼名称 - 骨骼ID 映射表
	std::map<std::string, int> boneMapping;	

	// 骨骼偏移数组,对应顶点变换到每个骨骼的骨骼空间的变换矩阵
	std::vector<aiMatrix4x4> boneOffsetMatrixs;

	SkinMesh(const std::vector<SkinVertex>& vertices, const std::vector<uint>& indices) :vertics(vertices), indices(indices) { indexCount = indices.size(); }
	void SetUpBuffer(ID3D11Device* device);

	/*
		根据当前时间获得当前动画下,各个骨骼的最终变换矩阵
		animationIndedx - 本次播放的动画ID
		TimeInSeconds - 游戏开始到目前为止经过的时间(单位:秒)
		transform - 输出的变换矩阵列表,下标为骨骼的ID
	*/
	void BoneTransform(uint animationIndedx, float TimeInSeconds,std::vector<aiMatrix4x4>& transforms);

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
	void ReadNodeHeirarchy(
		std::vector<aiMatrix4x4>& transforms,
		uint animationIndex,
		float animationTime,
		std::shared_ptr<SkeletonNode> boneNode,
		const aiMatrix4x4& parentTransform,
		const aiMatrix4x4& rootToLocalMatrix);

	/*
		根据骨骼名称和当前播放的动画片段,找到该骨骼对应的序列帧
	*/
	std::shared_ptr<NodeAnime> FindNode(std::shared_ptr<Animation> animation,std::string boneName);

	/*
		根据当前动画时间(单位:秒)和当前动画某个骨骼的序列,
		找到此时时间下,骨骼运动在哪个下标下
	*/
	void FindAnimeFramesIndex(float animationTime,std::shared_ptr<NodeAnime> nodeAnime,uint* outPosIndexint,uint* outScalIndex,uint* outQuatIndex);

	/*
		应用对应的Shader对该Mesh进行绘制
		shader - 本次绘制使用的Shader
		context - 绘制操作需要用到context对象设置环境变量
		primitiveTopology - 本次绘制使用的绘制模式(三角形模式/线形模式等)
		usePass - 本次绘制使用的Pass,为-1时,使用全部Pass进行绘制,当指定时,表示使用具体某个Pass进行绘制
				  举例: 使用高斯模糊时,要分别指定pass1垂直模糊和pass2竖直模糊来进行两次模糊
	*/
	void Draw(std::shared_ptr<Shader> shader, ID3D11DeviceContext* context, D3D_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, int usePass = -1);

	void Use(ID3D11DeviceContext* context);
};