/*
    包含常用的顶点输入和输出结构,
    参考自UnityCG.cginc文件
*/

// 最常用的顶点输入结构,包含顶点位置/法线/uv
struct appdata_basic{
    float3 vertex : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD0; 
};

// 含切线的顶点输入结构
struct appdata_tangent{
    float3 vertex : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;    
    float2 texcoord : TEXCOORD0; 
};
// 一般用于后处理操作的顶点输入结构
struct appdata_img{
    float3 vertex : POSITION;
    float2 texcoord : TEXCOORD0;
};
// 一般用于渲染阴影时的顶点输入结构,仅有顶点位置属性
struct appdata_vertex{
    float3 vertex : POSITION;
};

// 一般用于后处理的顶点输出结构
struct vertexOut_img{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0; 
};
// 只含pos和法线的顶点输出结构,一般用于前向渲染中，渲染深度图和法线图
struct vertexOut_normal{
    float4 pos : SV_Position;
    float3 worldNormal : NORMAL;
};
// 只含pos的顶点输出结构,一般用于前向渲染,仅渲染阴影图的情况
struct vertexOut_pos{
    float4 pos : SV_POSITION;
};