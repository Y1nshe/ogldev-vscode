#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec3 inColor;

uniform mat4 gWorld;
uniform bool isWireframe;

out vec4 WireframeColor;
out vec3 WorldPos_FS;
out vec2 TexCoord0;

void main()
{
    vec4 worldPos = gWorld * vec4(Position, 1.0);
    gl_Position = worldPos;
    WorldPos_FS = worldPos.xyz;
    TexCoord0 = TexCoord;
    WireframeColor = vec4(1.0, 1.0, 1.0, 1.0); // 白色边线
}
