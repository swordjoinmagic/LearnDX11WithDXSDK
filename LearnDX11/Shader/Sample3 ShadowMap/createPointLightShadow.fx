#include "../Common/basic.hlsl"
cbuffer cbPerObject{
    matrix mvp;
    matrix model;
    float farPlane;
    float3 lightPos;
};

struct vertexOut{
    float4 pos : SV_Position;
    float3 worldPos : POSITION;
};

vertexOut vert(appdata_vertex v){
    vertexOut o;
    o.pos = mul(float4(v.vertex,1.0),mvp);
    o.worldPos = mul(float4(v.vertex,1.0),model).xyz;
    return o;
}

float pixel(vertexOut i) : SV_TARGET{
    // 生成线性深度值

    // 取得光源和片元的距离(即片元的深度值)
    float lightDistance = length(i.worldPos - lightPos);

    // 将深度值归一化
    lightDistance = lightDistance / farPlane;

    // 写入深度值
    return lightDistance;
}

technique11{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vert()));
        SetPixelShader(CompileShader(ps_5_0,pixel()));
    }
}