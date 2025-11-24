vertex_shader = """
#version 330
in vec2 v_pos;
out vec2 v_uv;
void main() {
    v_uv = v_pos * 0.5 + 0.5;
    gl_Position = vec4(v_pos, 0.0, 1.0);
}
"""

fragment_shader = """
#version 330
in vec2 v_uv;
out vec4 fragColor;

uniform float iTime;
uniform vec2 iResolution;
uniform vec3 camPos;
uniform mat3 camRot;
uniform float fractalLod;
uniform int MAX_ITER;
uniform float BAILOUT;
uniform float POWER;

const float BASE_MAX_DIST = 100.0;
const float BASE_MIN_DIST = 0.001;

float mandelbulbDE(vec3 pos) {
    vec3 z = pos;
    float dr = 1.0;
    float r = 0.0;
    for(int i = 0; i < MAX_ITER; i++) {
        r = length(z);
        if(r > BAILOUT) break;

        float theta = acos(clamp(z.z/r, -1.0, 1.0));
        float phi = atan(z.y, z.x);

        dr = pow(r, POWER-1.0)*POWER*dr + 1.0;
        float zr = pow(r, POWER);
        theta *= POWER;
        phi *= POWER;

        z = zr * vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta)) + pos;
    }
    return 0.5 * log(r) * r / dr;
}

vec3 getNormal(vec3 p) {
    float eps = 0.0005 / fractalLod;
    vec2 h = vec2(eps, 0);
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
    float minDist = BASE_MIN_DIST / (fractalLod * fractalLod * fractalLod);

    for(int i = 0; i < max((128 * int(fractalLod)), 1000); i++) {
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

        float ao = smoothstep(0.0, 1.0, hitCount / 128.0);
        ao = 0.5 + 0.5 * ao;

        vec3 fractalColor = 0.5 + 0.5 * cos(p.yzx * 0.5 + vec3(0.0, 0.6, 1.2) + iTime * 0.05);
        vec3 finalColor = fractalColor * (diffuse * 0.7 + 0.3) * ao;
        fragColor = vec4(finalColor, 1.0);
    } else {
        fragColor = vec4(0.05, 0.1, 0.2, 1.0);
    }
}
"""
