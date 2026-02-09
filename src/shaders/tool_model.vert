#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in float aFaceShade;

out vec3 Color;
out float FaceShade;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(aPos, 1.0);
    Color = aColor;
    FaceShade = aFaceShade;
}