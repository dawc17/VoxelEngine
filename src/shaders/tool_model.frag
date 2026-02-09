#version 460 core
out vec4 FragColor;

in vec3 Color;
in float FaceShade;

uniform float timeOfDay;
uniform float ambientLight;

void main()
{
    float totalLight = max(timeOfDay, ambientLight);
    float finalLight = totalLight * FaceShade;
    FragColor = vec4(Color * finalLight, 1.0);
}