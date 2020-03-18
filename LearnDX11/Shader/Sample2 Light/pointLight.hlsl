#include "../Common/Light.hlsl"

cbuffer cbPerObject{
    // 移除平移操作后的vp矩阵
    matrix mvp;
    matrix model;
    matrix transInvModel;
    PointLight pointLight;
    float3 viewPos;
};

Texture2D mainTex;

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
};

vertexOut vert(vertexIn v){
    vertexOut o;
    
    o.pos = mul(float4(v.vertex,1.0),mvp);
    o.worldNormal = mul(float4(v.normal,1.0),transInvModel).xyz;
    o.worldPos = mul(float4(v.vertex,1.0),model).xyz;
    o.uv = v.texcoord;

    return o;
}

float4 pixel(vertexOut i) : SV_Target{    

    float diff = ProcessPointLightDiffuseWithLambert(pointLight,i.worldNormal,i.worldPos);
    float3 specu = ProcessPointLightSpecWithLambert(pointLight,i.worldNormal,viewPos,i.worldPos,32) * float3(1,1,1);    

    float3 albedo = diff * mainTex.Sample(state1,i.uv).rgb;
    float3 ambient = mainTex.Sample(state1,i.uv).rgb * 0.1;

    float3 finalColor = albedo + specu + ambient;

    return float4(finalColor,1.0);
}

technique11{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vert()));
        SetPixelShader(CompileShader(ps_5_0,pixel()));
    }
}