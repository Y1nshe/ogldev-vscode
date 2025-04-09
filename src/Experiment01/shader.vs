#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 inColor;

uniform mat4 gWorld;
uniform bool isWireframe;

out vec4 Color;
out vec4 WireframeColor;

void main()
{
    gl_Position = gWorld * vec4(Position, 1.0);
    Color = vec4(inColor, 1.0);
    WireframeColor = vec4(1.0, 1.0, 1.0, 1.0); // 白色边线
}
