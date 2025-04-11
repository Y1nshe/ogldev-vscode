#version 330

in vec2 TexCoord0; // 从顶点着色器接收纹理坐标
// in vec4 WireframeColor; // 移除线框颜色输入 (如果不再绘制线框)
// in vec3 WorldPos_FS; // 移除世界位置输入 (如果不再需要)

out vec4 FragColor;

// uniform int isWireframe; // 移除 isWireframe uniform
uniform sampler2D gSampler; // 纹理采样器

void main()
{
    // if (isWireframe == 1) {
    //     FragColor = WireframeColor;
    // } else {
    //     FragColor = vec4(WorldPos_FS.x, WorldPos_FS.y, WorldPos_FS.z, 1.0f);
    // }
    FragColor = texture(gSampler, TexCoord0); // 从纹理采样颜色
}
