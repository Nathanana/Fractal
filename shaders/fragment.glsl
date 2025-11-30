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

uniform int maxIterations;
uniform float power;
uniform bool autoRotate;

const int MAX_STEPS = 256;
const float MIN_DIST = 0.0001;
const float MAX_DIST = 100.0;
const float EPSILON = 0.0001;

// Mandelbulb distance estimator
float mandelbulbDE(vec3 pos) {
    vec3 z = pos;
    float dr = 1.0;
    float r = 0.0;
    
    for (int i = 0; i < maxIterations; i++) {
        r = length(z);
        
        if (r > 2.0) break;
        
        // Convert to polar coordinates
        float theta = acos(z.z / r);
        float phi = atan(z.y, z.x);
        dr = pow(r, power - 1.0) * power * dr + 1.0;
        
        // Scale and rotate the point
        float zr = pow(r, power);
        theta = theta * power;
        phi = phi * power;
        
        // Convert back to cartesian coordinates
        z = zr * vec3(
            sin(theta) * cos(phi),
            sin(phi) * sin(theta),
            cos(theta)
        );
        z += pos;
    }
    
    return 0.5 * log(r) * r / dr;
}

// Scene distance function
float sceneSDF(vec3 p) {
    // Apply optional rotation for visual effect
    if (autoRotate) {
        float angle = time * 0.1;
        float s = sin(angle);
        float c = cos(angle);
        mat3 rotY = mat3(
            c, 0.0, s,
            0.0, 1.0, 0.0,
            -s, 0.0, c
        );
        p = rotY * p;
    }
    
    return mandelbulbDE(p);
}

// Ray marching function
float rayMarch(vec3 ro, vec3 rd) {
    float depth = 0.0;
    
    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 p = ro + rd * depth;
        float dist = sceneSDF(p);
        
        if (dist < MIN_DIST * depth) {
            return depth;
        }
        
        depth += dist * 0.5; // Conservative step
        
        if (depth >= MAX_DIST) {
            return MAX_DIST;
        }
    }
    
    return MAX_DIST;
}

// Calculate normal using tetrahedron technique
vec3 calcNormal(vec3 p) {
    vec2 e = vec2(EPSILON, 0.0);
    return normalize(vec3(
        sceneSDF(p + e.xyy) - sceneSDF(p - e.xyy),
        sceneSDF(p + e.yxy) - sceneSDF(p - e.yxy),
        sceneSDF(p + e.yyx) - sceneSDF(p - e.yyx)
    ));
}

// Simple lighting
vec3 calculateLighting(vec3 p, vec3 normal, vec3 rd) {
    // Light position rotates around the scene
    vec3 lightPos = vec3(sin(time * 0.5) * 5.0, 3.0, cos(time * 0.5) * 5.0);
    vec3 lightDir = normalize(lightPos - p);
    
    // Ambient
    vec3 ambient = vec3(0.1, 0.1, 0.15);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(0.8, 0.7, 0.6);
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(-rd, reflectDir), 0.0), 32.0);
    vec3 specular = spec * vec3(1.0, 1.0, 1.0) * 0.5;
    
    // Rim lighting
    float rim = 1.0 - max(dot(-rd, normal), 0.0);
    rim = pow(rim, 3.0);
    vec3 rimColor = rim * vec3(0.2, 0.3, 0.5);
    
    // Ambient occlusion approximation
    float ao = 1.0 - float(maxIterations) / 256.0 * 0.5;
    
    return (ambient + diffuse + specular + rimColor) * ao;
}

// Color based on distance and iterations
vec3 getColor(vec3 p) {
    float dist = length(p);
    
    // Create gradient based on distance
    vec3 col1 = vec3(0.2, 0.3, 0.8);
    vec3 col2 = vec3(0.8, 0.2, 0.5);
    vec3 col3 = vec3(0.9, 0.7, 0.3);
    
    float t = fract(dist * 0.5);
    vec3 baseColor;
    
    if (t < 0.5) {
        baseColor = mix(col1, col2, t * 2.0);
    } else {
        baseColor = mix(col2, col3, (t - 0.5) * 2.0);
    }
    
    return baseColor;
}

void main() {
    // Calculate ray direction
    vec2 uv = (TexCoord * 2.0 - 1.0) * vec2(resolution.x / resolution.y, 1.0);
    
    float tanHalfFov = tan(fov / 2.0);
    vec3 rd = normalize(
        camFront + 
        camRight * uv.x * tanHalfFov +
        camUp * uv.y * tanHalfFov
    );
    
    // Ray origin is the camera position
    vec3 ro = camPos;
    
    // March the ray
    float dist = rayMarch(ro, rd);
    
    vec3 color;
    
    if (dist < MAX_DIST) {
        // Hit the fractal
        vec3 p = ro + rd * dist;
        vec3 normal = calcNormal(p);
        vec3 baseColor = getColor(p);
        vec3 lighting = calculateLighting(p, normal, rd);
        
        color = baseColor * lighting;
        
        // Add depth fog
        float fogAmount = 1.0 - exp(-dist * 0.02);
        vec3 fogColor = vec3(0.05, 0.05, 0.1);
        color = mix(color, fogColor, fogAmount);
        
        // Add glow effect
        float glow = exp(-dist * 0.1) * 0.3;
        color += vec3(glow * 0.5, glow * 0.3, glow * 0.8);
    } else {
        // Background gradient
        float t = uv.y * 0.5 + 0.5;
        color = mix(vec3(0.05, 0.05, 0.1), vec3(0.01, 0.02, 0.05), t);
        
        // Add stars
        vec2 starUV = uv * 50.0;
        float star = step(0.99, fract(sin(dot(floor(starUV), vec2(12.9898, 78.233))) * 43758.5453));
        color += vec3(star * 0.5);
    }
    
    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));
    
    FragColor = vec4(color, 1.0);
}