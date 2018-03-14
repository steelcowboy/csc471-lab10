#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform int uMesh;
out vec3 fragNor;
out vec3 baseColor;

out vec3 fragPosV;
out vec3 fragNorV;

void main()
{
	gl_Position = P * V * M * vertPos;
    vec3 base_color;

    if (uMesh == 0)
    {
        base_color = vec3(0, 89/255.0, 179/255.0);
    }
    else
    {
        base_color = vec3(20/255.0, 51/255.0, 6/255.0);
    }

    baseColor = base_color;
    fragNor = normalize((M * vec4(vertNor, 0)).xyz); 

    fragPosV = (V * M * vertPos).xyz;
    fragNorV = (V * M * vec4(vertNor, 0)).xyz;
}
