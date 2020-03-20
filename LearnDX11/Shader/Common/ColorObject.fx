cbuffer cbPerObject{
    float4x4 mvp;
    float4 color;
};

struct a2v{
    float3 vertex : POSITION;
};

struct v2f{
    float4 pos : SV_Position;
};

v2f vertex(a2v v){
    v2f o;
    o.pos = mul(float4(v.vertex,1.0f),mvp);

    return o;
}

float4 fragment(v2f i):SV_Target{
    return color;
}

technique11 ColorTech{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vertex()));
        SetPixelShader(CompileShader(ps_5_0,fragment()));
    }
}