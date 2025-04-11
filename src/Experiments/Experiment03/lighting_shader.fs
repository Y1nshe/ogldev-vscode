#version 330 core

// 从顶点着色器接收的输入
in vec3 WorldPos_FS; // 世界空间中的片段位置
in vec3 Normal_FS;   // 插值后的世界空间法向量
in vec2 TexCoord_FS; // <-- 新增: 插值后的纹理坐标

// 输出的最终颜色
layout (location = 0) out vec4 FragColor;

// Uniform 变量
uniform vec3 gEyeWorldPos;          // 世界空间中的摄像机位置
uniform sampler2D gSampler;         // <-- 新增: 纹理采样器

// 光源属性
uniform vec3 gLightAmbientColor;  // 环境光颜色分量
uniform vec3 gLightDiffuseColor;  // 漫反射光颜色分量
uniform vec3 gLightSpecularColor; // 镜面反射光颜色分量
uniform vec3 gLightDirection;     // 平行光方向 (从光源指向场景)

// 新增: 光照强度系数
uniform float gAmbientIntensity;
uniform float gDiffuseIntensity;
uniform float gSpecularIntensity;

// 材质属性 (移除了 gMaterialDiffuseColor)
uniform vec3 gMaterialAmbientColor;  // 材质的环境光反射系数
uniform vec3 gMaterialDiffuseColor; // <-- 移除
uniform vec3 gMaterialSpecularColor; // 材质的镜面反射系数
uniform float gMaterialShininess;    // 材质的高光系数

void main()
{
    // 标准化法向量
    vec3 Normal = normalize(Normal_FS);

    // 从纹理采样颜色
    vec4 OriginalTextureColor = texture(gSampler, TexCoord_FS);

    // --- 降低纹理亮度 ---
    vec4 DarkColor = vec4(0.5, 0.5, 0.5, 1.0); // 定义灰色
    float mixFactor = 0.3; // 混合 30% 的灰色
    vec4 DimmedTextureColor = mix(OriginalTextureColor, DarkColor, mixFactor);
    // --------------------

    // 1. 计算环境光照
    vec4 AmbientColor = vec4(gMaterialAmbientColor * gLightAmbientColor * gAmbientIntensity, 1.0);

    // 2. 计算漫反射光照
    vec3 LightDirectionNormalized = normalize(-gLightDirection);
    float DiffuseFactor = dot(Normal, LightDirectionNormalized);
    DiffuseFactor = max(DiffuseFactor, 0.0);
    vec4 DiffuseColor = vec4(gLightDiffuseColor * gMaterialDiffuseColor * DiffuseFactor * gDiffuseIntensity, 1.0);

    // 3. 计算镜面反射光照
    vec3 ViewDirection = normalize(gEyeWorldPos - WorldPos_FS);
    vec3 LightReflect = normalize(reflect(gLightDirection, Normal));
    float SpecularFactor = dot(ViewDirection, LightReflect);
    SpecularFactor = max(SpecularFactor, 0.0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);
    if (DiffuseFactor > 0.0f && SpecularFactor > 0.0f) {
        SpecularFactor = pow(SpecularFactor, gMaterialShininess);
        SpecularColor = vec4(gMaterialSpecularColor * gLightSpecularColor * SpecularFactor * gSpecularIntensity, 1.0);
    }

    // 合并光照分量
    // 环境光也用调暗后的纹理颜色调制
    FragColor = (AmbientColor + DiffuseColor + SpecularColor) * DimmedTextureColor; // <-- 使用 DimmedTextureColor
}
