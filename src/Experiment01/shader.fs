#version 330

in vec4 Color;
in vec4 WireframeColor;

uniform bool isWireframe;

out vec4 FragColor;

void main()
{
    if (isWireframe) {
        FragColor = WireframeColor;
    } else {
        FragColor = Color;
    }
}
