#include "../Common/Light.hlsl"

cbuffer cbPerObject{
    // 移除平移操作后的vp矩阵
    matrix mvp;
    matrix model;
    matrix transInvModel;
    Light light;
    // 用于将物体从世界空间转换到光源空间,是一个VP矩阵
    matrix lightVPMatrix;
    float3 viewPos;
};

Texture2D mainTex;

// 点光源阴影贴图
Texture2D shadowMap;

SamplerState state1{
    Filter =  MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};


struct vertexIn{
    float3 vertex : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD0;
};
struct vertexOut{
    float4 pos : SV_Position;
    float3 worldPos : POSITION;
    float3 worldNormal : NORMAL;
    float2 uv : TEXCOORD0;
    float4 shadowUV : TEXCOORD1;
};

vertexOut vert(vertexIn v){
    vertexOut o;
    
    o.pos = mul(float4(v.vertex,1.0),mvp);
    o.worldNormal = mul(float4(v.normal,1.0),transInvModel).xyz;
    o.worldPos = mul(float4(v.vertex,1.0),model).xyz;
    o.uv = v.texcoord;
    o.shadowUV = mul(float4(o.worldPos,1.0),lightVPMatrix);

    return o;
}

float4 pixel(vertexOut i) : SV_Target{    

    float3 diff = ProcessLightDiffuseWithLambert(light,i.worldNormal,i.worldPos);
    float3 specu = ProcessLightSpecWithLambert(light,i.worldNormal,i.worldPos,viewPos,32);

    float3 texColor = mainTex.Sample(state1,i.uv).rgb;
    float3 albedo = diff * texColor;
    float3 ambient = texColor * 0.1;

    // 阴影计算
    float4 shadowUV = i.shadowUV;
    // 进行齐次除法
    shadowUV.xyz /= shadowUV.w;

    float2 uv = float2(
        0.5*shadowUV.x+0.5,
        -0.5*shadowUV.y+0.5
    );

    // 获得当前像素坐标在光源空间下的实际深度
    float depth = shadowUV.z;

    float shadowBias = 0.005;

    // 采样获得当前片元坐标在光源空间下的最近深度
    float nearestDepth = shadowMap.Sample(state1,uv).r + shadowBias;
    // 如果当前片元深度小于等于最近深度,那么当前片元可见,否则处于阴影下
    float shadowFactor = depth > nearestDepth ? 1 : 0;

    return float4( (albedo + specu) * (1-shadowFactor) + ambient,1.0);
}

technique11{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vert()));
        SetPixelShader(CompileShader(ps_5_0,pixel()));
    }
}