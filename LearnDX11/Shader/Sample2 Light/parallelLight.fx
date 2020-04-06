#include "../Common/Light.hlsl"

cbuffer cbPerObject{
    // 移除平移操作后的vp矩阵
    matrix mvp;
    matrix model;
    matrix transInvModel;
    Light light;
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

    float3 diff = ProcessLightDiffuseWithLambert(light,i.worldNormal,i.worldPos);
    float3 specu = ProcessLightSpecWithLambert(light,i.worldNormal,i.worldPos,viewPos,255);

    float3 texColor = mainTex.Sample(state1,i.uv).rgb;
    float3 albedo = diff * texColor;
    float3 ambient = texColor * 0.1;

    return float4( albedo + specu + ambient,1.0);
}

technique11{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vert()));
        SetPixelShader(CompileShader(ps_5_0,pixel()));
    }
}