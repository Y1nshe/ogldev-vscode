#version 330

// 移除颜色输入
// in vec4 Color;
in vec4 WireframeColor;
in vec3 WorldPos_FS; // 添加世界坐标输入

uniform bool isWireframe;

out vec4 FragColor;

void main()
{
    if (isWireframe) {
        FragColor = WireframeColor;
    } else {
        // 根据世界坐标计算颜色
        // 使用 abs(normalize(WorldPos_FS)) 将位置向量映射到颜色
        FragColor = vec4(abs(normalize(WorldPos_FS)), 1.0);
    }
}
