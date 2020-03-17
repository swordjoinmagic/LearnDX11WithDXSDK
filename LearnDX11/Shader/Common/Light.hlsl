/*
    包含常用光源的结构体以及常用光照函数
*/
struct Light{
	// 理论上来说，平行光不具备位置属性
	// 这里为了渲染阴影贴图（获得光源的VP矩阵）临时设置一个
	float3 pos;    
	// 平行光方向
	float3 dir;	
	float3 lightColor;
};

// 点光源
struct PointLight {
	float3 pos;
    float Constant;	// 衰减常数项
	float3 lightColor;
	float Linear;	// 衰减一次项
	float Quadratic;	// 衰减二次项
};

// 聚光灯
struct SpotLight {
	float3 pos;
    float Constant;	// 衰减常数项
	float3 dir; // 表示聚光灯朝向方向spotDir
    float Linear;	// 衰减一次项
	float3 lightColor;	
	float Quadratic;	// 衰减二次项
	// 聚光灯外角的余弦值
	float outerCutOff;
	// 聚光灯内角的余弦值
	float cutOff;
};

// 给定一个点光源,法线方向,当前片元位置(世界空间),给出使用半兰伯特模型处理的漫反射光照结果
float ProcessPointLightDiffuseWithLambert(PointLight pointLight,float3 worldNormal,float3 worldPos){
    // 世界法线
    float3 N = normalize(worldNormal);
    // 光源的入射方向,注意这里是从片元指向光源位置
    float3 L = normalize(pointLight.pos - worldPos);

    // 半兰伯特光照模型
    float NDotL = dot(N,L)*0.5 + 0.5;

    // 计算点光源衰减

    // 光源距离物体的位置
    float distan = length(pointLight.pos - worldPos);
    float atten = 1.0 / (pointLight.Constant + pointLight.Linear*distan + pointLight.Quadratic * (distan*distan));

    return NDotL * atten;
}

float ProcessPointLightSpecWithLambert(
    PointLight pointLight,
    float3 worldNormal,
    float3 viewPos,
    float3 worldPos,    
    float gloss // 光滑程度,越高,则高光越集中,越亮; 越低,则高光越分散,越暗;
    ){
    // 世界空间的法线方向
    float3 N = normalize(worldNormal);
    // 世界空间的观察方向,注意是从物体指向观察者(摄像机)
    float3 V = normalize(viewPos - worldPos);
    // 世界空间的光源方向,从物体指向光源
    float3 L = normalize(pointLight.pos - worldPos);
    
    // 计算half向量(worldViewDir和worldLightDir中间向量)
    float3 halfDir = normalize(V+L);

    // half向量和法线的夹角余弦值与高光反射分量成正比
    float spec = pow(max(0,dot(halfDir,N)),gloss);

    // 计算光源衰减        
    float distan = length(pointLight.pos - worldPos);
    float atten = 1.0 / (pointLight.Constant + pointLight.Linear*distan + pointLight.Quadratic * (distan*distan));

    return spec * atten;
}

// 获得当前片元受到聚光灯的强度
float GetSpotLightIntensity(
    SpotLight light,
    float3 worldNormal,
    float3 worldPos){
    // 世界法线
    float3 N = normalize(worldNormal);
    // 光源的入射方向,注意这里是从片元指向光源位置
    float3 L = normalize(light.pos - worldPos);

    // 聚光灯朝向与物体指向光源方向的夹角余弦值
    float theta = dot(L,normalize(-light.dir));
    // 内角与外角的余弦值差
    float epsilon = light.cutOff - light.outerCutOff;
    // 聚光灯强度,为θ在内外角余弦值之间的插值,约束在[0,1]之间
    float intensity = clamp((theta - light.outerCutOff) / epsilon,0.0,1.0);

    return intensity;
}

/*
    计算聚光灯光源下该片元的漫反射分量(Lambert模型)
*/
float3 ProcessSpotLightDiffuseWithLambert(
    SpotLight light,    // 光源
    float3 worldNormal, // 世界空间的法线
    float3 worldPos    // 片元的世界坐标
    ){

    // 世界法线
    float3 N = normalize(worldNormal);
    // 光源的入射方向,注意这里是从片元指向光源位置
    float3 L = normalize(light.pos - worldPos);

    // 半兰伯特光照模型
    float NDotL = dot(N,L)*0.5 + 0.5;

    // 计算光源衰减
    float distan = length(light.pos - worldPos);
    float atten = 1.0 / (light.Constant + light.Linear*distan + light.Quadratic * (distan*distan));

    // 聚光灯强度
    float intensity = GetSpotLightIntensity(light,worldNormal,worldPos);

    float3 diffuse = light.lightColor * NDotL * atten * intensity;

    return diffuse;
}

/*
    计算聚光灯光源下,该片元的高光反射分量(Lambert模型)
*/
float3 ProcessSpotLightSpecWithLambert(
    SpotLight light,
    float3 worldNormal,
    float3 viewPos,
    float3 worldPos,
    float gloss     // 光滑程度,越高,则高光越集中,越亮; 越低,则高光越分散,越暗;
    ){
    // 世界空间的法线方向
    float3 N = normalize(worldNormal);
    // 世界空间的观察方向,注意是从物体指向观察者(摄像机)
    float3 V = normalize(viewPos - worldPos);
    // 世界空间的光源方向,从物体指向光源
    float3 L = normalize(light.pos - worldPos);
    
    // 计算half向量(worldViewDir和worldLightDir中间向量)
    float3 halfDir = normalize(V+L);

    // half向量和法线的夹角余弦值与高光反射分量成正比
    float spec = pow(max(0,dot(halfDir,N)),32);

    // 计算光源衰减        
    float distan = length(light.pos - worldPos);
    float atten = 1.0 / (light.Constant + light.Linear*distan + light.Quadratic * (distan*distan));

    // 聚光灯强度
    float intensity = GetSpotLightIntensity(light,worldNormal,worldPos);

    float3 specular = light.lightColor * spec * intensity * atten;

    return specular;
}