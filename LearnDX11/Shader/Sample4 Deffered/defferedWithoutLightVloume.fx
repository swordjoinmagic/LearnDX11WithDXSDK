/*
    无光体积优化的延迟渲染着色器,
    本质上就是对一个平铺在屏幕上的四边形进行着色,
    通过GBuffer来得到当前片元位置的世界坐标/法线/纹理颜色等
*/
#include "../Common/basic.hlsl"
#include "../Common/Light.hlsl"

cbuffer cbPerObject{
    PointLight pointLight;
    float3 viewPos;
};

Texture2D gWorldPosTex;
Texture2D gWorldNormalTex;
Texture2D gAlbedoGloss;

SamplerState state{
    Filter =  MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

vertexOut_img vert(appdata_img v){
    vertexOut_img o;
    o.pos = float4(v.vertex,1.0);
    o.uv = float2(v.texcoord.x,1-v.texcoord.y);
    return o;
}

float4 pixel(vertexOut_img i) : SV_Target{
    float3 worldNormal = normalize(gWorldNormalTex.Sample(state,i.uv).xyz);
    float3 worldPos = gWorldPosTex.Sample(state,i.uv).xyz;
    float4 albedoGloss = gAlbedoGloss.Sample(state,i.uv);
    float gloss = albedoGloss.w;
    float3 texColor = albedoGloss.xyz;

    float diff = ProcessPointLightDiffuseWithLambert(pointLight,worldNormal,worldPos);
    float specu = ProcessPointLightSpecWithLambert(pointLight,worldNormal,viewPos,worldPos,gloss);

    float3 diffuseColor = diff * texColor * pointLight.lightColor;
    float3 specularColor = specu * pointLight.lightColor;
    float3 ambient = texColor * 0.1;

    return float4(diffuseColor+specularColor+ambient,1.0);
}

technique11 DefferedTech{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vert()));
        SetPixelShader(CompileShader(ps_5_0,pixel()));
    }
}