# Vertex shader is basic, passing position to UV
vertex_shader = """
// Updated to GLSL 430 to ensure double precision support
#version 430
in vec2 v_pos;
out vec2 v_uv;

void main() {
    v_uv = v_pos * 0.5 + 0.5;
    gl_Position = vec4(v_pos, 0.0, 1.0);
}
"""

# Fragment shader adapted to use uniform parameters
fragment_shader = """
#version 430
#extension GL_ARB_gpu_shader_float64 : enable

in vec2 v_uv;
out vec4 fragColor;

uniform vec2 iResolution;
uniform vec3 camPos;
uniform mat3 camRot;
uniform float fractalLod;
uniform int MAX_ITER;
uniform float BAILOUT;
uniform float POWER;

const float BASE_MAX_DIST = 100.0;
const float BASE_MIN_DIST = 0.001;
const int BASE_STEPS = 128;

double mandelbulbDE_double(dvec3 pos, double dBAILOUT, double dPOWER) {
    dvec3 z = pos;
    double dr = 1.0;
    double r = 0.0;

    for(int i = 0; i < MAX_ITER; i++) {
        r = length(z);
        if(r > dBAILOUT) break;

        // Cast to float for trig/pow/log
        float r_f     = float(r);
        float theta   = acos(clamp(float(z.z / r), -1.0, 1.0));
        float phi     = atan(float(z.y), float(z.x));

        dr = pow(r_f, float(dPOWER - 1.0)) * float(dPOWER) * dr + 1.0;
        float zr = pow(r_f, float(dPOWER));
        theta *= float(dPOWER);
        phi   *= float(dPOWER);

        // Convert back to double for accumulation
        z = dvec3(zr * sin(theta) * cos(phi),
                  zr * sin(theta) * sin(phi),
                  zr * cos(theta)) + pos;
    }

    // Final distance estimate in float
    float r_f = float(r);
    return 0.5 * log(r_f) * r / dr; // r/dr are doubles, log(r_f) is float
}


// Float wrapper
float mandelbulbDE(vec3 pos) {
    if(fractalLod < 2.0) {
        // Low LOD: float precision only
        vec3 z = pos;
        float dr = 1.0;
        float r = 0.0;
        for(int i = 0; i < MAX_ITER; i++) {
            r = length(z);
            if(r > BAILOUT) break;

            float theta = acos(clamp(z.z/r, -1.0, 1.0));
            float phi = atan(z.y, z.x);

            dr = pow(r, POWER-1.0) * POWER * dr + 1.0;
            float zr = pow(r, POWER);
            theta *= POWER;
            phi *= POWER;

            z = zr * vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta)) + pos;
        }
        return 0.5 * log(r) * r / dr;
    } else {
        // High LOD: double precision
        return float(mandelbulbDE_double(dvec3(pos), double(BAILOUT), double(POWER)));
    }
}

vec3 getNormal(vec3 p) {
    float eps = max(0.00001, 0.0005 / fractalLod);
    vec2 h = vec2(eps, 0.0);
    return normalize(vec3(
        mandelbulbDE(p + h.xyy) - mandelbulbDE(p - h.xyy),
        mandelbulbDE(p + h.yxy) - mandelbulbDE(p - h.yxy),
        mandelbulbDE(p + h.yyx) - mandelbulbDE(p - h.yyx)
    ));
}

void main() {
    vec2 uv = (v_uv - 0.5) * 2.0;
    uv.x *= iResolution.x / iResolution.y;

    vec3 rayDir = normalize(camRot * vec3(uv, 1.0));
    vec3 rayPos = camPos;

    float totalDist = 0.0;
    float hitCount = 0.0;
    vec3 p;

    float maxDist = BASE_MAX_DIST;
    
    // Smooth LOD-adaptive step size and max steps
    float minDist = BASE_MIN_DIST / pow(1.0 + fractalLod, 2.5);
    int maxSteps = min(4096, BASE_STEPS + int(BASE_STEPS * fractalLod * 2.0));

    for(int i = 0; i < maxSteps; i++) {
        p = rayPos + rayDir * totalDist;
        float dist = mandelbulbDE(p);
        if(dist < minDist) break;
        totalDist += dist;
        hitCount += 1.0;
        if(totalDist > maxDist) {
            totalDist = maxDist;
            break;
        }
    }

    if(totalDist < maxDist) {
        vec3 normal = getNormal(p);
        vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
        float diffuse = max(0.0, dot(normal, lightDir));

        float ao = smoothstep(0.0, 1.0, hitCount / float(BASE_STEPS));
        ao = 0.5 + 0.5 * ao;

        // Stabilized coloring
        vec3 fractalColor = 0.5 + 0.5 * cos((p * 0.05) * (1.0 + fractalLod * 0.1) + vec3(0.0, 0.6, 1.2));
        vec3 finalColor = fractalColor * (diffuse * 0.7 + 0.3) * ao;
        fragColor = vec4(finalColor, 1.0);
    } else {
        fragColor = vec4(0.05, 0.1, 0.2, 1.0);
    }
}

"""