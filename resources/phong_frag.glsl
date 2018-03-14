#version 330 core 
in vec3 fragNor;
in vec3 baseColor;

in vec3 fragPosV;
in vec3 fragNorV;

out vec4 color;

uniform vec3 uLight;
uniform mat4 V;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float shine;

void main()
{
    vec3 fragColor;

    vec3 light_vec = normalize(uLight);
    float diffuse = max(dot(fragNor, light_vec), 0);

    vec3 worldNor = normalize(fragNorV);
    vec3 worldLight = (V * vec4(light_vec, 0)).xyz;

    vec3 viewVec = -normalize(fragPosV);
    vec3 halfVec = normalize(viewVec + worldLight);
    float halfNor = dot(halfVec, worldNor);
    float specularCoefficient = pow(halfNor, shine);

    fragColor = baseColor + diffuse*MatDif + MatAmb + specularCoefficient * MatSpec;

	color = vec4(fragColor, 1.0); 
}
