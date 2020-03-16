cbuffer cbPerObject{
    // 移除平移操作后的vp矩阵
    matrix mvp;
};

// 天空盒纹理
TextureCube skyBox : register(t0);

SamplerState state1{
    Filter =  MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};


struct a2v{
    float3 vertex : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD0;
};
struct v2f{
    float4 pos : SV_Position;
    // 用于采样天空盒的纹理
    float3 uv : TEXCOORD0;    
};

v2f vert(a2v v){
    v2f o;
    
    o.pos = mul(float4(v.vertex,1.0),mvp);
    o.uv = v.vertex;

    return o;
}

float4 pixel(v2f i) : SV_Target{
    return skyBox.Sample(state1,i.uv.xyz);
}

technique11{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vert()));
        SetPixelShader(CompileShader(ps_5_0,pixel()));
    }
}