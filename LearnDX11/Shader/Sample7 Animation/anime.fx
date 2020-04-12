#include "../common/Light.hlsl"

static const int MAX_BONES = 100;

cbuffer cbPerObject{
    // 移除平移操作后的vp矩阵
    matrix mvp;
    matrix model;
    matrix transInvModel;
    Light light;
    float3 viewPos;
};

cbuffer cbAnimationData{
    matrix gBones[MAX_BONES];
};

struct vertexIn{
    float3 vertex : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;

    int boneID[4] : BONEID;
    float weights[4] : WEIGHTS;

	float2 texcoord : TEXCOORD;
};

struct vertexOut{
    float4 pos : SV_Position;
    float3 worldPos : POSITION;
    float3 worldNormal : NORMAL;
    float2 uv : TEXCOORD0;
};

Texture2D mainTex;

SamplerState state1{
    Filter =  MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};


vertexOut vert(vertexIn v){
    vertexOut o;
    
    matrix BoneTransform = gBones[v.boneID[0]] * v.weights[0];
    BoneTransform += gBones[v.boneID[1]] * v.weights[1];
    BoneTransform += gBones[v.boneID[2]] * v.weights[2];
    BoneTransform += gBones[v.boneID[3]] * v.weights[3];

    float4 localPos = mul(float4(v.vertex,1.0),BoneTransform);
    float4 localNormal = mul(float4(v.normal,0.0),BoneTransform);

    // float4 localPos = float4(v.vertex,1.0);
    // float4 localNormal = float4(v.normal,0);

    o.pos = mul(localPos,mvp);
    o.worldNormal = mul(localNormal,transInvModel).xyz;
    o.worldPos = mul(localPos,model).xyz;
    o.uv = v.texcoord;

    return o;
}

float4 pixel(vertexOut i) : SV_Target{    

    float3 diff = ProcessLightDiffuseWithLambert(light,i.worldNormal,i.worldPos);
    float3 specu = ProcessLightSpecWithLambert(light,i.worldNormal,i.worldPos,viewPos,255);

    float3 texColor = mainTex.Sample(state1,i.uv).rgb;
    float3 albedo = diff * texColor;
    float3 ambient = texColor * 0.1;

    return float4( albedo + specu + ambient,1.0);
}

technique11{
    pass P0{
        SetVertexShader(CompileShader(vs_5_0,vert()));
        SetPixelShader(CompileShader(ps_5_0,pixel()));
    }
}