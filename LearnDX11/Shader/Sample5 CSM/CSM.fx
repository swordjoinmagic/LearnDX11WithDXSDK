#include "../Common/Light.hlsl"

cbuffer cbPerObject{
    // matrix lightVPMatrix;
    // 移除平移操作后的vp矩阵
    matrix mvp;
    matrix model;
    matrix transInvModel;
    Light light;
    float3 viewPos;
    // 用于将物体从世界空间转换到光源空间,是一个VP矩阵
    matrix lightVPMatrixs[3];
};

Texture2D mainTex : register(t0);

Texture2DArray cascadedShadowMap : register(t1);

SamplerState shadowSamplerState{
    Filter =  MIN_MAG_MIP_LINEAR;
    AddressU = Border;
    AddressV = Border;
    BorderColor = float4(1,1,1,1);
};

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
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 worldNormal : TEXCOORD2;
    float4 shadowUV[3] : TEXCOORD3;
};

static const uint NUM_CASCADES = 3;

vertexOut vert(vertexIn v){
    vertexOut o;
    
    o.pos = mul(float4(v.vertex,1.0),mvp);
    o.worldNormal = mul(float4(v.normal,1.0),transInvModel).xyz;
    o.worldPos = mul(float4(v.vertex,1.0),model).xyz;
    o.uv = v.texcoord;

    float4 shadowUVs[3];
    for(uint i=0;i<NUM_CASCADES;i++){
        shadowUVs[i] = mul(float4(o.worldPos,1.0),lightVPMatrixs[i]);        
    }
    o.shadowUV = shadowUVs;

    return o;
}

float CSMCalculation(vertexOut i){
    float shadowFactor = 0;
    float shadowBias = 0.005;
    for(uint j=0;j<NUM_CASCADES;j++){
        // 阴影计算
        float4 shadowUV = i.shadowUV[j];
        // 进行齐次除法
        shadowUV.xyz /= shadowUV.w;

        float2 uv = float2(
            0.5*shadowUV.x+0.5,
            -0.5*shadowUV.y+0.5
        );
        shadowUV.xy = uv;

        if( shadowUV.x > 0.01 & shadowUV.x< 0.99 &
            shadowUV.y > 0.01 & shadowUV.y<0.99 &
            shadowUV.z>0.0 & shadowUV.z<1.0){
 
            // 获得当前像素坐标在光源空间下的实际深度
            float depth = shadowUV.z;

            // 采样获得当前片元坐标在光源空间下的最近深度
            float nearestDepth = cascadedShadowMap.Sample(shadowSamplerState,float3(uv,j)).r + shadowBias;
            // 如果当前片元深度小于等于最近深度,那么当前片元可见,否则处于阴影下
            shadowFactor = depth > nearestDepth ? 1 : 0;

            break;
        }
    }
    return shadowFactor;
}

float4 CSMArea(vertexOut i){
    for(uint j=0;j<NUM_CASCADES;j++){
        // 阴影计算
        float4 shadowUV = i.shadowUV[j];
        // 进行齐次除法
        shadowUV.xyz /= shadowUV.w;

        float2 uv = float2(
            0.5*shadowUV.x+0.5,
            -0.5*shadowUV.y+0.5
        );
        shadowUV.xy = uv;

        if( shadowUV.x > 0.01 & shadowUV.x< 0.99 &
            shadowUV.y > 0.01 & shadowUV.y<0.99 &
            shadowUV.z>0.0 & shadowUV.z<1.0){

            if(j==0) return float4(1,0,0,1);
            if(j==1) return float4(0,1,0,1);
            if(j==2) return float4(0,0,1,1);
        }
    }
    return float4(1,1,1,1);
}

float4 pixel(vertexOut i) : SV_Target{    

    float3 diff = ProcessLightDiffuseWithLambert(light,i.worldNormal,i.worldPos);
    float3 specu = ProcessLightSpecWithLambert(light,i.worldNormal,i.worldPos,viewPos,255);

    float3 texColor = mainTex.Sample(state1,i.uv).rgb;
    float3 albedo = diff * texColor;
    float3 ambient = texColor * 0.1;
    
    float shadowFactor = CSMCalculation(i);
    float4 CSMAreaColor = CSMArea(i);

    float4 finalColor = float4( (albedo + specu) * (1-shadowFactor) + ambient,1.0);
    finalColor *= CSMAreaColor;

    return finalColor;
    return float4(1-shadowFactor,1-shadowFactor,1-shadowFactor,1);
}

technique11{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vert()));
        SetPixelShader(CompileShader(ps_5_0,pixel()));
    }
}
