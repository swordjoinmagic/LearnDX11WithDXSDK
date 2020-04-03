#include "../Common/basic.hlsl"

cbuffer cbPerObject{
    matrix mvp;
    uint index;
};

Texture2DArray texArray : register(t0);

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
struct vertexOut{
    float4 pos : SV_Position;    
    float2 uv : TEXCOORD0;
};

vertexOut vert(appdata_basic v){
    vertexOut o;
    o.pos = mul(float4(v.vertex,1.0),mvp);
    o.uv = v.texcoord;
    return o;
}

float4 pixel(vertexOut i) : SV_Target{
    return texArray.Sample(samLinear,float3(i.uv,index));
}

technique11{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vert()));
        SetPixelShader(CompileShader(ps_5_0,pixel()));
    }
}