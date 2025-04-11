#version 330 core

layout (location = 0) in vec3 Position; // 顶点位置
layout (location = 1) in vec3 Normal;   // 顶点法向量
layout (location = 2) in vec2 TexCoord; // <-- 新增: 顶点纹理坐标

// 输出到片元着色器
out vec3 WorldPos_FS; // 世界空间中的顶点位置
out vec3 Normal_FS;   // 世界空间中的法向量 (经过变换)
out vec2 TexCoord_FS; // <-- 新增: 传递给片元着色器的纹理坐标

// Uniform 变量
uniform mat4 gWVP;    // 世界-视图-投影 矩阵
uniform mat4 gWorld;  // 世界矩阵 (用于变换法向量和位置)

void main()
{
    // 计算裁剪空间中的顶点位置
    gl_Position = gWVP * vec4(Position, 1.0);
    // 计算世界空间中的顶点位置并传递给片元着色器
    WorldPos_FS = (gWorld * vec4(Position, 1.0)).xyz;
    // 变换法向量到世界空间 (使用法线矩阵) 并传递给片元着色器
    // 法线矩阵是世界矩阵的逆转置矩阵的左上 3x3 部分
    Normal_FS = mat3(transpose(inverse(gWorld))) * Normal;
    // 直接传递纹理坐标
    TexCoord_FS = TexCoord; // <-- 新增
}
