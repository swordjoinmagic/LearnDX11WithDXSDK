#include "../Common/basic.hlsl"


cbuffer cbPerObject{
    // 移除平移操作后的vp矩阵
    matrix mvp;
    matrix model;
    matrix transInvModel;

    // 高光反射光滑度
    float gloss;
};

Texture2D mainTex;

SamplerState state1{
    Filter =  MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

struct vertexOut{
    float4 pos : SV_Position;
    float3 worldPos : POSITION;
    float3 worldNormal : NORMAL;
    float2 uv : TEXCOORD0;
};

// 多渲染目标
struct pixelOut{
    float4 worldPos;
    float4 worldNormal;
    float4 albedoGloss;
};

vertexOut vert(appdata_basic v){
    vertexOut o;
    o.pos = mul(float4(v.vertex,1.0),mvp);
    o.worldPos = mul(float4(v.vertex,1.0),model).xyz;
    o.worldNormal = mul(float4(v.normal,1.0),transInvModel).xyz;
    o.uv = v.texcoord;
    return o;
}

pixelOut pixel(vertexOut i) : SV_Target{    
    pixelOut o;
    o.worldPos = float4(i.worldPos,1.0);
    o.worldNormal = float4(i.worldNormal,1.0);
    o.albedoGloss.xyz = mainTex.Sample(state1,i.uv);
    o.albedoGloss.w = gloss;
    return o;
}

technique11{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vert()));
        SetPixelShader(CompileShader(ps_5_0,pixel()));
    }
}