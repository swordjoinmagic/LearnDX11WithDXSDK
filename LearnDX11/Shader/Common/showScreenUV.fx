cbuffer cbPerObject{
    float4x4 mvp;
};

struct a2v{
    float3 vertex : POSITION;
};

struct v2f{
    float4 pos : SV_Position;
    float4 scrPos : TEXCOORD0;
};

v2f vertex(a2v v){
    v2f o;
    o.pos = mul(float4(v.vertex,1.0f),mvp);
    o.scrPos = o.pos * 0.5 + o.pos.w;
    return o;
}

float4 fragment(v2f i):SV_Target{
    return float4(1,1,1,1);
}

technique11 ColorTech{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vertex()));
        SetPixelShader(CompileShader(ps_5_0,fragment()));
    }
}