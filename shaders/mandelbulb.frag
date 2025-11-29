#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform vec3 cameraPos;
uniform vec3 cameraFront;
uniform vec3 cameraUp;
uniform vec3 cameraRight;
uniform float fov;
uniform float aspect;
uniform int maxIterations;
uniform float bailout;
uniform float power;

// distance estimator for Mandelbulb
float mandelbulbDE(vec3 pos, int iterations) {
    vec3 z = pos;
    float dr = 1.0; 
    float r = 0.0;
    for (int i = 0; i < iterations; ++i) {
        r = length(z);
        if (r > bailout) break;
        
        dr = pow(r, power - 1.0) * power * dr + 1.0; 
        
        // spherical coords
        float theta = acos(clamp(z.z / r, -1.0, 1.0));
        float phi = atan(z.y, z.x);
        float zr = pow(r, power);
        theta *= power;
        phi *= power;
        z = zr * vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
        
        z += pos;
    }
    r = max(r, 1e-8);
    return 0.5 * log(r) * r / dr;
}

// normal via central differences
vec3 calcNormal(vec3 p, int iterations, float nEps) {
    float dx = mandelbulbDE(p + vec3(nEps, 0.0, 0.0), iterations) - mandelbulbDE(p - vec3(nEps, 0.0, 0.0), iterations);
    float dy = mandelbulbDE(p + vec3(0.0, nEps, 0.0), iterations) - mandelbulbDE(p - vec3(0.0, nEps, 0.0), iterations);
    float dz = mandelbulbDE(p + vec3(0.0, 0.0, nEps), iterations) - mandelbulbDE(p - vec3(0.0, 0.0, nEps), iterations);
    return normalize(vec3(dx, dy, dz));
}

// simple soft shadow
float softShadow(vec3 ro, vec3 rd, int iters, float mint, float maxt) {
    float res = 1.0;
    float t = mint;
    for (int i = 0; i < 60; ++i) {
        float h = mandelbulbDE(ro + rd * t, iters);
        if (h < 1e-6) return 0.0;
        
        // Shadow Fix: Reduced multiplier from 8.0 to 4.0 for lighter shadows
        res = min(res, 4.0 * h / t);
        
        t += clamp(h, 0.01, 0.5);
        if (t > maxt) break;
    }
    return clamp(res, 0.0, 1.0);
}

vec4 rayMarch(vec3 ro, vec3 rd) {
    float t = 0.0;
    const float maxDist = 2000.0;
    const int maxSteps = 300;
    const float HIT_EPSILON = 0.00005; 
    const float NORMAL_EPSILON = 0.0001; 

    for (int i = 0; i < maxSteps; ++i) {
        vec3 p = ro + rd * t;

        float lodFactor = smoothstep(7.0, 50.0, t);
        int iters = int(mix(float(maxIterations), float(maxIterations) * 0.5, lodFactor));

        float stepMul = mix(0.9, 0.99, lodFactor); 

        float d = mandelbulbDE(p, iters);

        if (d < HIT_EPSILON) {
            
            vec3 normal = calcNormal(p, iters, NORMAL_EPSILON);

            // lighting
            vec3 lightDir = normalize(vec3(1.0, 1.0, 0.6));
            float diff = max(dot(normal, lightDir), 0.0);

            float shadow = softShadow(p + normal * 0.001, lightDir, max(8, iters / 4), 0.001, 20.0);
            
            float ao = clamp(float(iters) / float(maxIterations), 0.3, 1.0); 

            vec3 base = vec3(0.7, 0.55, 0.4) * (0.45 + 0.55 * diff) * shadow * ao;

            vec3 view = normalize(-rd);
            vec3 refl = reflect(-lightDir, normal);
            float spec = pow(max(dot(view, refl), 0.0), 28.0) * 0.9;

            vec3 color = base + vec3(spec);
            return vec4(color, 1.0);
        }

        if (t > maxDist) break;

        t += max(d * stepMul, HIT_EPSILON * 0.2); 
    }

    float g = 0.5 * (rd.y + 1.0);
    vec3 bg = mix(vec3(0.02,0.03,0.06), vec3(0.10, 0.12, 0.14), g);
    return vec4(bg, 1.0);
}

void main() {
    vec2 uv = (TexCoord - 0.5) * 2.0;
    uv.x *= aspect;
    float fovScale = tan(radians(fov) * 0.5);
    
    vec3 rd = normalize(cameraFront + cameraRight * uv.x * fovScale + cameraUp * uv.y * fovScale);

    FragColor = rayMarch(cameraPos, rd);
}