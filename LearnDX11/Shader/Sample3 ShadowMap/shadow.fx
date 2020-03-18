#include "../Common/Light.hlsl"

cbuffer cbPerObject{
    // 移除平移操作后的vp矩阵
    matrix mvp;
    matrix model;
    matrix transInvModel;
    PointLight pointLight;
    float3 viewPos;

    // 远平面的距离
    float farPlane;
};

Texture2D mainTex;

// 点光源阴影贴图
TextureCube shadowMap;

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

// 根据片元的世界坐标,获得该处的阴影因子,如果为0,则不存在阴影,为1存在
float ShadowCalculation(float3 worldPos){
    // 由光源指向物体的方向向量,用于采样点光源阴影立方体贴图
    float3 lightDir = worldPos - pointLight.pos;

    // 获得当前片元位置在屏幕上的最近深度
    float closestDepth = shadowMap.Sample(state1,lightDir).r;
    // 将归一化的最近深度还原成观察空间下的深度
    closestDepth *= farPlane;

    // 得到当前片元的深度(即距离光源的最近距离)
    float currentDepth = length(lightDir);

    // 比较两个深度,若当前片元深度大于最近深度,
    // 则当前片元被其他物体挡住,产生阴影,
    // 若当前片元小于等于最近深度,则当前片元则为距离该光源最近的物体,不产生阴影
    float bias = 0.05;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

float4 pixel(vertexOut i) : SV_Target{    

    float diff = ProcessPointLightDiffuseWithLambert(pointLight,i.worldNormal,i.worldPos);
    float3 specu = ProcessPointLightSpecWithLambert(pointLight,i.worldNormal,viewPos,i.worldPos,32) * float3(1,1,1);    

    float3 texColor = mainTex.Sample(state1,i.uv).rgb;
    float3 albedo = diff * texColor;
    float3 ambient = texColor * 0.1;

    float shadowFactor = ShadowCalculation(i.worldPos);

    float3 finalColor = (albedo + specu) * (1-shadowFactor) + ambient;

    return float4(finalColor,1.0);

    // 由光源指向物体的方向向量,用于采样点光源阴影立方体贴图
    // float3 lightDir = i.worldPos - pointLight.pos;

    // 获得当前片元位置在屏幕上的最近深度
    // float closestDepth = shadowMap.Sample(state1,lightDir).r;

    // return float4(closestDepth,closestDepth,closestDepth,1.0);
}

technique11{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vert()));
        SetPixelShader(CompileShader(ps_5_0,pixel()));
    }
}