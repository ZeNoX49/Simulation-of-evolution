#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 VertexColor;

void main()
{
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normalize(Normal), lightDir), 0.0);
    float ambient = 0.3;
    vec3 color = VertexColor * (ambient + diff * (1.0 - ambient));
    FragColor = vec4(color, 1.0);
}