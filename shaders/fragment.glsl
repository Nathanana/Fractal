#version 460 core

in vec2 TexCoord;
out vec4 FragColor;

uniform vec3 camPos;
uniform vec3 camFront;
uniform vec3 camRight;
uniform vec3 camUp;
uniform float fov;
uniform float time;
uniform vec3 resolution;
uniform float scale;
uniform float adaptiveSpeed;

uniform int maxIterations;
uniform bool autoRotate;

const int MAX_STEPS = 200;
const float MIN_DIST = 0.0001;
const float MAX_DIST = 100.0;

float mandelboxDE(vec3 pos, out float orbitTrap) {
    vec3 z = pos;
    float dr = 1.0;

    const float scale = -1.5;
    const float minRadius = 0.5;
    const float fixedRadius = 2.25;

    orbitTrap = 1000.0;

    for (int i = 0; i < maxIterations; i++) {
        z = clamp(z, -1.0, 1.0) * 2.0 - z;

        float r2 = dot(z, z);

        if (r2 < minRadius * minRadius) {
            float t = (fixedRadius * fixedRadius) / (minRadius * minRadius);
            z *= t;
            dr *= t;
        } else if (r2 < fixedRadius * fixedRadius) {
            float t = (fixedRadius * fixedRadius) / r2;
            z *= t;
            dr *= t;
        }

        z = z * scale + pos;
        dr = dr * abs(scale) + 1.0;

        orbitTrap = min(orbitTrap,
                        abs(z.x) + abs(z.y) + abs(z.z));
    }

    return length(z) / abs(dr);
}

float sceneSDF(vec3 p, out float orbitTrap) {
    vec3 objPos = p / scale;
    
    if (autoRotate) {
        float angle = time * 0.1;
        float s = sin(angle);
        float c = cos(angle);
        mat3 rotY = mat3(
            c, 0.0, s,
            0.0, 1.0, 0.0,
            -s, 0.0, c
        );
        objPos = rotY * objPos;
    }

    return mandelboxDE(objPos, orbitTrap) * scale;
}

float rayMarch(vec3 ro, vec3 rd, out int steps, out bool hit, out float orbitTrap) {
    float depth = 0.0;
    steps = 0;
    hit = false;
    orbitTrap = 1000.0;
    
    for (int i = 0; i < MAX_STEPS; i++) {
        steps = i;
        vec3 p = ro + rd * depth;
        float trap;
        float dist = sceneSDF(p, trap);
        
        orbitTrap = min(orbitTrap, trap);

        if (dist < MIN_DIST) {
            hit = true;
            return depth;
        }

        depth += dist * 0.9;
        
        if (depth >= MAX_DIST) {
            break;
        }
    }
    
    return MAX_DIST;
}

vec3 calcNormal(vec3 p, float dist) {
    float eps = 0.001;
    float trap;
    
    vec2 e = vec2(eps, 0.0);
    return normalize(vec3(
        sceneSDF(p + e.xyy, trap) - sceneSDF(p - e.xyy, trap),
        sceneSDF(p + e.yxy, trap) - sceneSDF(p - e.yxy, trap),
        sceneSDF(p + e.yyx, trap) - sceneSDF(p - e.yyx, trap)
    ));
}

float calcSoftShadow(vec3 ro, vec3 rd, float mint, float maxt) {
    float res = 1.0;
    float t = mint;
    float trap; // Dummy
    
    for (int i = 0; i < 16; i++) {
        float h = sceneSDF(ro + rd * t, trap);
        
        if (h < 0.001) {
            return 0.0;
        }
        
        res = min(res, 8.0 * h / t);
        t += h;
        
        if (t > maxt) {
            break;
        }
    }
    
    return clamp(res, 0.0, 1.0);
}

float calcAO(vec3 p, vec3 n) {
    float occ = 0.0;
    float sca = 1.0;
    float trap; // Dummy
    
    for (int i = 0; i < 5; i++) {
        float h = 0.001 + 0.15 * float(i) / 4.0;
        float d = sceneSDF(p + h * n, trap);
        occ += (h - d) * sca;
        sca *= 0.95;
        
        if (d < 0.0) break;
    }
    
    return clamp(1.0 - 2.5 * occ, 0.0, 1.0);
}

vec3 getColor(vec3 p, vec3 normal, float orbitTrap) {
    float t = clamp(orbitTrap * 0.5, 0.0, 1.0);

    vec3 col1 = vec3(0.1, 0.5, 0.9); // Bright blue
    vec3 col2 = vec3(0.9, 0.3, 0.5); // Pink-red
    vec3 col3 = vec3(0.2, 0.9, 0.6); // Cyan-green
    vec3 col4 = vec3(0.9, 0.7, 0.2); // Orange-yellow
    vec3 col5 = vec3(0.6, 0.2, 0.9); // Purple

    vec3 baseColor;
    if (t < 0.2) {
        baseColor = mix(col1, col2, t * 5.0);
    } else if (t < 0.4) {
        baseColor = mix(col2, col3, (t - 0.2) * 5.0);
    } else if (t < 0.6) {
        baseColor = mix(col3, col4, (t - 0.4) * 5.0);
    } else if (t < 0.8) {
        baseColor = mix(col4, col5, (t - 0.6) * 5.0);
    } else {
        baseColor = mix(col5, col1, (t - 0.8) * 5.0);
    }
    
    float normalVar = dot(normal, vec3(0.577)) * 0.5 + 0.5;
    baseColor = mix(baseColor * 0.8, baseColor * 1.2, normalVar);

    float sparkle = pow(abs(sin(p.x * 50.0) * sin(p.y * 50.0) * sin(p.z * 50.0)), 20.0);
    baseColor += vec3(sparkle * 0.3);
    
    return baseColor;
}

vec3 calculateLighting(vec3 p, vec3 normal, vec3 rd, float ao) {
    vec3 lightPos1 = vec3(sin(time * 0.3) * 10.0, 5.0, cos(time * 0.3) * 10.0);
    vec3 lightPos2 = vec3(-5.0, 8.0, -5.0);
    vec3 lightPos3 = vec3(5.0, -3.0, 8.0);
    
    vec3 lightDir1 = normalize(lightPos1 - p);
    vec3 lightDir2 = normalize(lightPos2 - p);
    vec3 lightDir3 = normalize(lightPos3 - p);
    
    float shadow1 = calcSoftShadow(p + normal * 0.002, lightDir1, 0.02, 10.0);
    
    vec3 ambient = vec3(0.2, 0.2, 0.25);
    
    float diff1 = max(dot(normal, lightDir1), 0.0);
    float diff2 = max(dot(normal, lightDir2), 0.0);
    float diff3 = max(dot(normal, lightDir3), 0.0);
    
    vec3 diffuse = vec3(0.0);
    diffuse += diff1 * vec3(1.0, 0.95, 0.9) * 0.6 * shadow1;
    diffuse += diff2 * vec3(0.9, 0.95, 1.0) * 0.4;
    diffuse += diff3 * vec3(1.0, 0.9, 0.85) * 0.3;
    
    vec3 reflectDir1 = reflect(-lightDir1, normal);
    float spec1 = pow(max(dot(-rd, reflectDir1), 0.0), 32.0);
    vec3 specular = spec1 * vec3(1.0, 1.0, 1.0) * 0.4 * shadow1;
    
    float rim = 1.0 - max(dot(-rd, normal), 0.0);
    rim = pow(rim, 4.0);
    vec3 rimColor = rim * vec3(0.3, 0.5, 0.7) * 0.5;
    
    float skyLight = max(0.0, 0.5 + 0.5 * normal.y);
    vec3 skyColor = vec3(0.3, 0.4, 0.6) * skyLight * 0.3;
    
    return (ambient + diffuse + specular + rimColor + skyColor) * ao;
}

void main() {
    vec2 uv = (TexCoord * 2.0 - 1.0) * vec2(resolution.x / resolution.y, 1.0);
    
    float tanHalfFov = tan(fov / 2.0);

    vec3 color = vec3(0.0);
    float aaSize = 1.0 / resolution.y;
    
    for (int aaY = 0; aaY < 2; aaY++) {
        for (int aaX = 0; aaX < 2; aaX++) {
            vec2 offset = vec2(float(aaX), float(aaY)) * aaSize - aaSize * 0.5;
            vec2 sampleUV = uv + offset;
            
            vec3 rd = normalize(
                camFront + 
                camRight * sampleUV.x * tanHalfFov +
                camUp * sampleUV.y * tanHalfFov
            );
            
            vec3 ro = camPos;
            
            int steps;
            bool hit;
            float orbitTrap;
            float dist = rayMarch(ro, rd, steps, hit, orbitTrap);
            
            vec3 sampleColor;
            
            if (hit && dist < MAX_DIST) {
                vec3 p = ro + rd * dist;
                vec3 normal = calcNormal(p, dist);
                
                float ao = calcAO(p, normal);
                vec3 baseColor = getColor(p, normal, orbitTrap);
                vec3 lighting = calculateLighting(p, normal, rd, ao);
                
                sampleColor = baseColor * lighting;
                
                float depthFade = exp(-dist * 0.01);
                sampleColor *= mix(0.5, 1.0, depthFade);
                
            } else {
                vec3 worldDir = rd;
                
                vec2 starUV = vec2(
                    atan(worldDir.z, worldDir.x),
                    asin(worldDir.y)
                ) * 10.0;
                
                float star = 0.0;
                for (int i = 0; i < 3; i++) {
                    vec2 gridUV = starUV * (float(i + 1) * 3.0);
                    vec2 gridID = floor(gridUV);
                    float hash = fract(sin(dot(gridID, vec2(12.9898, 78.233))) * 43758.5453);
                    
                    if (hash > 0.98) {
                        vec2 cellUV = fract(gridUV) - 0.5;
                        float d = length(cellUV);
                        star += smoothstep(0.05, 0.0, d) * (hash - 0.98) * 50.0;
                    }
                }
                
                float gradient = pow(abs(worldDir.y) * 0.5 + 0.5, 2.0);
                sampleColor = mix(vec3(0.01, 0.01, 0.02), vec3(0.02, 0.02, 0.05), gradient);
                sampleColor += vec3(star) * vec3(1.0, 0.95, 0.9);
            }
            
            color += sampleColor;
        }
    }
    
    color /= 4.0;

    color = pow(color, vec3(1.0 / 2.2));
    
    FragColor = vec4(color, 1.0);
}