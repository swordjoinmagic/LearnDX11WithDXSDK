#pragma once

#include "D3DUtils.h"
#include <vector>
#include "MathF.h"
#include "DefaultVertex.h"
#include "Mesh.h"
class GeometryGenerator {
public:
	/*
		创建一个width*height的网格,共分为m*n个quad,
	*/
	static std::shared_ptr<Mesh> CreateGrid(float width,float height,uint m,uint n) {		
		std::vector<DefaultVertex> vertices;
		std::vector<uint> indices;

		// 顶点数
		uint vertexCount = m * n;
		// 三角面数
		uint faceCount = (m - 1)*(n - 1) * 2;

		float halfWidth = 0.5f*width;
		float halfHeight = 0.5f*height;

		// 每个四边形间隔距离
		float dx = width / (n - 1);
		float dy = height / (m - 1);
		float du = 1.0f / (n - 1);
		float dv = 1.0f / (m - 1);

		// 初始化容量
		vertices.resize(vertexCount);

		// 设置顶点属性
		for (uint i = 0; i < m;i++) {
			float y = halfHeight - i * dy;
			for (uint j = 0; j < n;j++) {			
				// 设置第i行j列的网格
				float x = -halfWidth + j * dx;

				vertices[i*n + j].pos = float3(x,0,y);
				vertices[i*n + j].normal = float3(0,1,0);
				vertices[i*n + j].tangent = float3(1,0,0);

				// uv
				vertices[i*n + j].texcoord = float2(j*du,i*dv);

			}
		}

		indices.resize(faceCount*3);
		// 设置顶点索引
		uint k = 0;	
		for (uint i = 0; i < m - 1;i++) {
			for (uint j = 0; j < n - 1;j++) {
				indices[k] = i * n + j;
				indices[k + 1] = i * n + j + 1;
				indices[k + 2] = (i + 1)*n + j;
				indices[k + 3] = (i + 1)*n + j;
				indices[k + 4] = i * n + j + 1;
				indices[k + 5] = (i + 1)*n + j + 1;
				k += 6;
			} 
		}

		return std::make_shared<Mesh>(vertices,indices);
	}
	
	/*
		bottomRadius: 底部圆形半径,
		topRadius:	  顶部圆形半径,
		height:		  圆柱体高度
		sliceCount:   切片数量,即将一个圆分割的三角形数量
		stackCount:	  圆柱体层数,即横向切割的层数
	*/
	static std::shared_ptr<Mesh> CreateCylinder(float bottomRadius,float topRadius,float height,uint sliceCount, uint stackCount) {

		std::vector<DefaultVertex> vertices;
		std::vector<uint> indices;

		// 每层的高度,即△H
		float stackHegiht = height / stackCount;
		// 从底部到顶部每层的半径增量,即为△r
		float radiusStep = (topRadius - bottomRadius) / stackCount;
		// 环得数量
		uint ringCount = stackCount + 1;

		//mesh.vertices.resize(ringCount*sliceCount);
		// 从底部开始,由下至上计算每个环上顶点的坐标
		for (uint i = 0; i < ringCount;i++) {
			// 顶点高度(圆柱中心为原点,故1/2下方处高度为负)
			float y = -0.5*height + i * stackHegiht;
			// 当前堆叠层的半径
			float r = bottomRadius + i * radiusStep;

			// 角度增量
			float dTheta = 2.0f * MathF::PI / sliceCount;
			// 遍历环上各个顶点
			for (uint j = 0; j <= sliceCount;j++) {
				DefaultVertex vertex;
				
				float c = cosf(j*dTheta);
				float s = sinf(j*dTheta);

				vertex.pos = XMFLOAT3(r*c,y,r*s);
				vertex.texcoord = XMFLOAT2((float)j/sliceCount,1.0f-(float)i/stackCount);
				vertex.tangent = XMFLOAT3(-s,0.0f,c);
				
				float dr = bottomRadius - topRadius;
				XMFLOAT3 bitangent(dr*c,-height,dr*s);

				XMVECTOR T = XMLoadFloat3(&vertex.tangent);
				XMVECTOR B = XMLoadFloat3(&bitangent);
				XMVECTOR N = XMVector3Normalize(XMVector3Cross(T,B));
				XMStoreFloat3(&vertex.normal,N);

				vertices.push_back(vertex);
			}
		}
			
		uint ringVertexCount = sliceCount + 1;
		for (uint i = 0; i < stackCount;i++) {
			for (uint j = 0; j < sliceCount;j++) {
				indices.push_back(i*ringVertexCount+j);
				indices.push_back((i+1)*ringVertexCount+j);
				indices.push_back((i+1)*ringVertexCount+j+1);

				indices.push_back(i*ringVertexCount+j);
				indices.push_back((i+1)*ringVertexCount+j+1);
				indices.push_back(i*ringVertexCount+j+1);
			}
		}

		
		// 构建顶部的圆形网格		
		uint baseIndex = (uint)vertices.size();
		float dTheta = 2.0f * MathF::PI / sliceCount;
		float y = 0.5f*height;
		
		for (uint i = 0; i <= sliceCount;i++) {
			float x = topRadius * cosf(i*dTheta);
			float z = topRadius * sinf(i*dTheta);

			float u = x / height + 0.5f;
			float v = z / height + 0.5f;

			vertices.push_back(
				DefaultVertex(float3(x, y, z),float3(0, 1, 0),float3(1, 0, 0),float2(u,v))
			);
		}
		// 顶面的中心顶点
		vertices.push_back(DefaultVertex(float3(0,y,0),float3(0,1,0),float3(1,0,0),float2(0.5f,0.5f)));
		// 重心顶点的索引值
		uint centerindex = (uint)vertices.size() - 1;
		for (uint i = 0; i < sliceCount;i++) {
			indices.push_back(centerindex);
			indices.push_back(baseIndex + i + 1);
			indices.push_back(baseIndex + i);
		}


		// 构建底部的圆形网格
		baseIndex = (uint)vertices.size();
		y = -0.5f*height;
		for (uint i = 0; i <= sliceCount; i++) {
			float x = bottomRadius * cosf(i*dTheta);
			float z = bottomRadius * sinf(i*dTheta);
	
			float u = x / height + 0.5f;
			float v = z / height + 0.5f;

			vertices.push_back(
				DefaultVertex(float3(x, y, z), float3(0, -1, 0), float3(1, 0, 0), float2(u, v))
			);
		}
		vertices.push_back(DefaultVertex(float3(0,y,0),float3(0,-1,0),float3(1,0,0),float2(0.5f,0.5f)));
		centerindex = (uint)vertices.size() - 1;

		for (uint i = 0; i < sliceCount;i++) {
			indices.push_back(centerindex);
			indices.push_back(baseIndex+i);
			indices.push_back(baseIndex+i+1);
		}

		return std::make_shared<Mesh>(vertices,indices);
	}

	static std::shared_ptr<Mesh> CreateBox(float width,float height,float depth) {				
		std::vector<DefaultVertex> v;
		v.resize(24);

		float w2 = 0.5f*width;
		float h2 = 0.5f*height;
		float d2 = 0.5f*depth;

		// Fill in the front face vertex data.
		v[0] = DefaultVertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		v[1] = DefaultVertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		v[2] = DefaultVertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		v[3] = DefaultVertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		// Fill in the back face vertex data.
		v[4] = DefaultVertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
		v[5] = DefaultVertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		v[6] = DefaultVertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		v[7] = DefaultVertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

		// Fill in the top face vertex data.
		v[8] = DefaultVertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		v[9] = DefaultVertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		v[10] = DefaultVertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		v[11] = DefaultVertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		// Fill in the bottom face vertex data.
		v[12] = DefaultVertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
		v[13] = DefaultVertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		v[14] = DefaultVertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		v[15] = DefaultVertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

		// Fill in the left face vertex data.
		v[16] = DefaultVertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
		v[17] = DefaultVertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
		v[18] = DefaultVertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
		v[19] = DefaultVertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

		// Fill in the right face vertex data.
		v[20] = DefaultVertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
		v[21] = DefaultVertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
		v[22] = DefaultVertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
		v[23] = DefaultVertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

		//
		// Create the indices.
		//

		std::vector<uint> i;
		i.resize(36);

		// Fill in the front face index data
		i[0] = 0; i[1] = 1; i[2] = 2;
		i[3] = 0; i[4] = 2; i[5] = 3;

		// Fill in the back face index data
		i[6] = 4; i[7] = 5; i[8] = 6;
		i[9] = 4; i[10] = 6; i[11] = 7;

		// Fill in the top face index data
		i[12] = 8; i[13] = 9; i[14] = 10;
		i[15] = 8; i[16] = 10; i[17] = 11;

		// Fill in the bottom face index data
		i[18] = 12; i[19] = 13; i[20] = 14;
		i[21] = 12; i[22] = 14; i[23] = 15;

		// Fill in the left face index data
		i[24] = 16; i[25] = 17; i[26] = 18;
		i[27] = 16; i[28] = 18; i[29] = 19;

		// Fill in the right face index data
		i[30] = 20; i[31] = 21; i[32] = 22;
		i[33] = 20; i[34] = 22; i[35] = 23;

		return std::make_shared<Mesh>(v, i);
	}

	static std::shared_ptr<Mesh> CreateSphere(float radius, UINT sliceCount, UINT stackCount) {
		std::vector<DefaultVertex> vertices;
		std::vector<uint> indices;		

		//
		// Compute the vertices stating at the top pole and moving down the stacks.
		//

		// Poles: note that there will be texture coordinate distortion as there is
		// not a unique point on the texture map to assign to the pole when mapping
		// a rectangular texture onto a sphere.
		DefaultVertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		DefaultVertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

		vertices.push_back(topVertex);

		float phiStep = XM_PI / stackCount;
		float thetaStep = 2.0f*XM_PI / sliceCount;

		// Compute vertices for each stack ring (do not count the poles as rings).
		for (UINT i = 1; i <= stackCount - 1; ++i) {
			float phi = i * phiStep;

			// Vertices of ring.
			for (UINT j = 0; j <= sliceCount; ++j) {
				float theta = j * thetaStep;

				DefaultVertex v;

				// spherical to cartesian
				v.pos.x = radius * sinf(phi)*cosf(theta);
				v.pos.y = radius * cosf(phi);
				v.pos.z = radius * sinf(phi)*sinf(theta);

				// Partial derivative of P with respect to theta
				v.tangent.x = -radius * sinf(phi)*sinf(theta);
				v.tangent.y = 0.0f;
				v.tangent.z = +radius * sinf(phi)*cosf(theta);

				XMVECTOR T = XMLoadFloat3(&v.tangent);
				XMStoreFloat3(&v.tangent, XMVector3Normalize(T));

				XMVECTOR p = XMLoadFloat3(&v.pos);
				XMStoreFloat3(&v.normal, XMVector3Normalize(p));

				v.texcoord.x = theta / XM_2PI;
				v.texcoord.y = phi / XM_PI;

				vertices.push_back(v);
			}
		}

		vertices.push_back(bottomVertex);

		//
		// Compute indices for top stack.  The top stack was written first to the vertex buffer
		// and connects the top pole to the first ring.
		//

		for (UINT i = 1; i <= sliceCount; ++i) {
			indices.push_back(0);
			indices.push_back(i + 1);
			indices.push_back(i);
		}

		//
		// Compute indices for inner stacks (not connected to poles).
		//

		// Offset the indices to the index of the first vertex in the first ring.
		// This is just skipping the top pole vertex.
		UINT baseIndex = 1;
		UINT ringVertexCount = sliceCount + 1;
		for (UINT i = 0; i < stackCount - 2; ++i) {
			for (UINT j = 0; j < sliceCount; ++j) {
				indices.push_back(baseIndex + i * ringVertexCount + j);
				indices.push_back(baseIndex + i * ringVertexCount + j + 1);
				indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);

				indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);
				indices.push_back(baseIndex + i * ringVertexCount + j + 1);
				indices.push_back(baseIndex + (i + 1)*ringVertexCount + j + 1);
			}
		}

		//
		// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
		// and connects the bottom pole to the bottom ring.
		//

		// South pole vertex was added last.
		UINT southPoleIndex = (UINT)vertices.size() - 1;

		// Offset the indices to the index of the first vertex in the last ring.
		baseIndex = southPoleIndex - ringVertexCount;

		for (UINT i = 0; i < sliceCount; ++i) {
			indices.push_back(southPoleIndex);
			indices.push_back(baseIndex + i);
			indices.push_back(baseIndex + i + 1);
		}

		return std::make_shared<Mesh>(vertices,indices);
	}
};