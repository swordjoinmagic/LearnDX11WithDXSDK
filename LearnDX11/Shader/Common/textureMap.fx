cbuffer cbPerObject{
    float4x4 mvp;
};

struct a2v{
    float3 vertex : POSITION;
    float2 texcoord : TEXCOORD0;
};

struct v2f{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

Texture2D mainTex;
SamplerState state{
    Filter =  MIN_MAG_MIP_LINEAR;
    AddressU = clamp;
    AddressV = clamp;
};

v2f vertex(a2v v){
    v2f o;
    o.pos = mul(float4(v.vertex,1.0f),mvp);
    o.uv = v.texcoord;
    return o;
}

float4 fragment(v2f i):SV_Target{
    return mainTex.Sample(state,i.uv);
}

technique11 TextureMapTech{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vertex()));
        SetPixelShader(CompileShader(ps_5_0,fragment()));
    }
}