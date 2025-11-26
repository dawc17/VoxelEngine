#version 460 core
out vec4 FragColor;

in vec2 LocalUV;
flat in float TileIndex;
in float Light;

uniform sampler2DArray textureArray;

void main()
{
    // Sample from the texture array - layer is the tile index
    vec4 texColor = texture(textureArray, vec3(LocalUV, TileIndex));
    
    // Alpha test - discard nearly transparent pixels (for leaves, etc.)
    if (texColor.a < 0.5)
        discard;
    
    // Apply AO directly - Light already contains face shading * AO brightness
    // This gives the classic Minecraft look with dark crevices
    vec3 litColor = texColor.rgb * Light;
    
    FragColor = vec4(litColor, texColor.a);
}
