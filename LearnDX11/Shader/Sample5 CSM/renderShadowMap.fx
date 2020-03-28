#include "../Common/basic.hlsl"
cbuffer cbPerObject{
    matrix mvp;
};

vertexOut_pos vert(appdata_vertex v){
    vertexOut_pos o;
    o.pos = mul(float4(v.vertex,1.0),mvp);
    return o;
}

technique11{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vert()));
        PixelShader = NULL;
    }
}