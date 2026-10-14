#pragma once

inline const char* VERTEX_SHADER_SRC = R"GLSL(

#version 460 core

layout(location=0) in vec3 in_pos;
layout(location=1) in vec4 in_inst;

uniform mat4 mvp;
uniform float time;

flat out vec3 v_col;

float hash(float n){
    return fract(sin(n)*43758.5453);
}

vec3 hsv2rgb(vec3 c){

    vec4 K = vec4(
        1.0,
        2.0/3.0,
        1.0/3.0,
        3.0
    );

    vec3 p = abs(
        fract(c.xxx + K.xyz)
        * 6.0 - K.www
    );

    return c.z * mix(
        K.xxx,
        clamp(
            p - K.xxx,
            0.0,
            1.0
        ),
        c.y
    );
}

mat3 rot(vec3 a, float ang){

    a = normalize(a);

    float c = cos(ang);
    float s = sin(ang);

    float oc = 1.0 - c;

    return mat3(

        oc*a.x*a.x+c,
        oc*a.x*a.y-a.z*s,
        oc*a.z*a.x+a.y*s,

        oc*a.x*a.y+a.z*s,
        oc*a.y*a.y+c,
        oc*a.y*a.z-a.x*s,

        oc*a.z*a.x-a.y*s,
        oc*a.y*a.z+a.x*s,
        oc*a.z*a.z+c
    );
}

void main(){

    vec3 p = in_pos;

    float seed = in_inst.w;

    float sec = floor(time);

    float r1 = hash(seed + sec * 1.37);
    float r2 = hash(seed + sec * 2.91);
    float r3 = hash(seed + sec * 4.73);
    float r4 = hash(seed + sec * 7.11);

    vec3 axis = normalize(vec3(
        r1 * 2.0 - 1.0,
        r2 * 2.0 - 1.0,
        r3 * 2.0 - 1.0
    ));

    float dir =
        mix(
            -1.0,
            1.0,
            step(0.5,r4)
        );

    float speed =
        0.2 + r2 * 10.0;

    float jump =
        r3 * 6.2831853;

    float ang =
        time *
        speed *
        dir +
        jump;

    p = rot(axis, ang) * p;

    p += in_inst.xyz;

    float hue =
        fract(
            hash(seed) +
            time * 0.05 +
            r1 * 0.3
        );

    v_col =
        hsv2rgb(
            vec3(
                hue,
                0.9,
                1.0
            )
        );

    gl_Position =
        mvp *
        vec4(p,1.0);
}

)GLSL";

inline const char* FRAGMENT_SHADER_SRC = R"GLSL(

#version 460 core

flat in vec3 v_col;

out vec4 fragColor;

void main(){

    fragColor =
        vec4(v_col,1.0);
}

)GLSL";
