#version 460

in vec3 Normal;
in vec3 Position;
in vec2 TexCoord;
in vec4 ShadowCoord;

//uniform variables declaration
layout(binding = 0) uniform sampler2D Tex1;
const int levels = 4;
const float scaleFactor = 1.0/levels;
uniform sampler2DShadow ShadowMap;

uniform struct MaterialInfo{
    vec3 Ka; //Ambient
    vec3 Kd; //Diffuse
    vec3 Ks; //Specular
    float Shininess;
}Material;

uniform struct LightInfo{
    vec3 Position; //Position in camCoords
    vec3 Intensity; //Intensity in A, D and S
}Light;

layout (location = 0) out vec4 FragColor;

//blinn-phong shading technique used to create toon shading for project
vec3 blinnPhong(vec3 pos, vec3 n)
{
    vec3 texColor = texture(Tex1, TexCoord).rgb;
    vec3 ambient =  Light.Intensity * texColor;

    vec3 s = normalize(vec3(vec3(Light.Position) - pos));
    float sDotN = max(dot(s, n), 0.0f);

    vec3 diffuse = Material.Kd * floor(sDotN * levels) * scaleFactor;

    if (sDotN > 0.0f)
    {
        vec3 v = normalize(-pos.xyz);
        vec3 h = normalize(v + s);
    }

    return ambient + Light.Intensity * diffuse;
}

//phong model used to calculate diffuse and specular values
vec3 phongModelDiffAndSpec()
{
    vec3 n = Normal;
    vec3 s = normalize(vec3(Light.Position) - Position.xyz);
    vec3 v = normalize(-Position.xyz);
    vec3 r = reflect(-s, n);

    float sDotN = max(dot(s, n), 0.0);
    vec3 diffuse = Light.Intensity * Material.Kd * sDotN;
    vec3 spec = vec3(0.0);

    if (sDotN > 0.0f)
        spec = Light.Intensity * Material.Ks *
            pow(max(dot(r,v), 0.0), Material.Shininess);

    return diffuse + spec;
}

subroutine void RenderPassType();
subroutine uniform RenderPassType RenderPass;

subroutine(RenderPassType)
void shadeWithShadow()
{
    vec3 ambient = Light.Intensity * Material.Ka;
    vec3 diffAndSpec = phongModelDiffAndSpec();

    float shadow = 1.0f;
    if(ShadowCoord.z >= 0){
        shadow = textureProj(ShadowMap, ShadowCoord);
    }

    FragColor = vec4(diffAndSpec * shadow + ambient, 1.0);

    FragColor = pow(FragColor, vec4(1.0/2.2));
}

subroutine(RenderPassType)
void recordDepth()
{
}

void main() {
    RenderPass();
    FragColor = vec4(blinnPhong(Position, normalize(Normal)), 1.0);
}
