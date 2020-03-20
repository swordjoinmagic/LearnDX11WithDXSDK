/*
    使用光体积优化的延迟渲染着色器
*/
#include "../Common/basic.hlsl"
#include "../Common/Light.hlsl"

cbuffer cbPerObject{
    matrix mvp;
    float3 viewPos;
    PointLight pointLight;
};

Texture2D gWorldPosTex;
Texture2D gWorldNormalTex;
Texture2D gAlbedoGloss;

SamplerState state{
    Filter =  MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

struct vertexOut{
    float4 pos : SV_POSITION;
    float4 scrPos : TEXCOORD0;
};

vertexOut vert(appdata_img v){
    vertexOut o;
    o.pos = mul(float4(v.vertex,1.0),mvp);
    o.scrPos = o.pos * 0.5;
    o.scrPos.xy = o.scrPos.xy + float2(o.scrPos.w,o.scrPos.w);
    o.scrPos.zw = o.pos.zw;
    return o;
}

float4 pixel(vertexOut i) : SV_Target{
    float2 uv = i.scrPos.xy / i.scrPos.w;
    uv = float2(uv.x,1-uv.y);

    float3 worldNormal = normalize(gWorldNormalTex.Sample(state,uv).xyz);
    float3 worldPos = gWorldPosTex.Sample(state,uv).xyz;
    float4 albedoGloss = gAlbedoGloss.Sample(state,uv);
    float gloss = albedoGloss.w;
    float3 texColor = albedoGloss.xyz;
    float3 ambient = texColor * 0.1;
    
    float3 finalColor = float3(0,0,0);

    float diff = ProcessPointLightDiffuseWithLambert(pointLight,worldNormal,worldPos);
    float specu = ProcessPointLightSpecWithLambert(pointLight,worldNormal,viewPos,worldPos,gloss);

    float3 diffuseColor = diff * texColor * pointLight.lightColor;
    float3 specularColor = specu * pointLight.lightColor;

    finalColor += diffuseColor + specularColor;

    return float4(finalColor,1.0);
}

technique11 DefferedTech{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vert()));
        SetPixelShader(CompileShader(ps_5_0,pixel()));
    }
}