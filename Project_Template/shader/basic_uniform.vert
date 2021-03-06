#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

uniform float Time;

uniform float Freq = 2.5f;
uniform float Velocity = 2.5f;
uniform float Amp = 0.6f;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;
uniform mat4 ShadowMatrix;

uniform bool isAnimated;

out vec3 Position;
out vec3 Normal;
out vec2 TexCoord;
out vec4 ShadowCoord;

void main()
{
    if(isAnimated) //if animated, code used to animate model
    {
        vec4 pos = vec4(VertexPosition, 1.0f);

        float u = Freq * pos.x - Velocity * Time;
        pos.y = Amp * sin(u);

        vec3 n = vec3(0.0);
        n.xy = normalize(vec2(cos(u), 1.0));

        Position = (ModelViewMatrix * pos).xyz;
        Normal = NormalMatrix * n;
        TexCoord = VertexTexCoord;

        gl_Position = MVP * pos;
    }

    else //if not animated, default shader code
    {
        Position = (ModelViewMatrix * vec4(VertexPosition, 1.0)).xyz;
        Normal = normalize(NormalMatrix * VertexNormal);
        TexCoord = VertexTexCoord;
        ShadowCoord = ShadowMatrix * vec4(VertexPosition, 1.0);
        gl_Position = MVP * vec4(VertexPosition, 1.0);
    }
    
}