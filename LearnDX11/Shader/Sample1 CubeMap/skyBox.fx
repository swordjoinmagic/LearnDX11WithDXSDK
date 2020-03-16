cbuffer cbPerObject{
    // 移除平移操作后的vp矩阵
    matrix vp;
};

// 天空盒纹理
TextureCube skyBox : register(t0);

SamplerState state1{
    Filter =  MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

RasterizerState NoCull{
    CullMode = None;
};
DepthStencilState LessEqualDSS{
    DepthFunc = LESS_EQUAL;
};

struct a2v{
    float3 vertex : POSITION;
};
struct v2f{
    float4 pos : SV_Position;
    // 用于采样天空盒的纹理
    float3 uv : TEXCOORD0;    
};

v2f vert(a2v v){
    v2f o;
    
    o.pos = mul(float4(v.vertex,1.0),vp);
    // 将pos的z分量设为w(使得经过齐次除法后,pos的z分量恒为1.0)
    o.pos = o.pos.xyww;
    o.uv = v.vertex;

    return o;
}

float4 pixel(v2f i) : SV_Target{
    return skyBox.Sample(state1,i.uv);
}

technique11{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vert()));
        SetPixelShader(CompileShader(ps_5_0,pixel()));

        SetRasterizerState(NoCull);
        SetDepthStencilState(LessEqualDSS,0);
    }
}