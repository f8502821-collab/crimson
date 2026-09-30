#version 330
// CRIMSON post-process: applied to the game frame while the menu is open.
// Uniforms are time-animated; intensity is scaled by menu_open blend.
in vec2 v_uv;
out vec4 frag_color;

uniform sampler2D u_scene;
uniform float u_time;
uniform float u_amount;      // 0..1 master intensity
uniform vec2  u_res;

// hash noise
float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

void main() {
    vec2 uv = v_uv;
    vec2 centered = uv - 0.5;

    // --- chromatic aberration: RGB split growing toward edges, pulsing ------
    float ab = (0.0022 + 0.0016 * sin(u_time * 1.4)) * u_amount;
    vec2 dir = centered * ab * (1.5 + 2.5 * dot(centered, centered));
    float r = texture(u_scene, uv + dir).r;
    float g = texture(u_scene, uv).g;
    float b = texture(u_scene, uv - dir).b;
    vec3 col = vec3(r, g, b);

    // --- crimson edge bleed: red tint pushing in from screen borders -------
    float edge = smoothstep(0.35, 0.78, length(centered) * 1.9);
    float bleed = edge * (0.32 + 0.10 * sin(u_time * 0.9)) * u_amount;
    col = mix(col, col * vec3(1.0, 0.30, 0.38) + vec3(0.10, 0.0, 0.02), bleed);

    // --- scanlines (subtle, slow drift) -------------------------------------
    float scan = sin((uv.y + u_time * 0.02) * u_res.y * 1.35) * 0.5 + 0.5;
    col *= 1.0 - scan * 0.055 * u_amount;

    // --- film grain ----------------------------------------------------------
    float gr = noise(uv * u_res * 0.5 + vec2(u_time * 61.0, u_time * -47.0));
    col += (gr - 0.5) * 0.055 * u_amount;

    // --- animated vignette: breathing darkness at the edges ------------------
    float vig = smoothstep(0.95, 0.35, length(centered) * (1.35 + 0.06 * sin(u_time * 0.7)));
    col *= mix(1.0, vig, 0.85 * u_amount);

    // --- corner desaturation --------------------------------------------------
    float desat = smoothstep(0.55, 0.95, length(centered) * 1.6) * u_amount;
    float lum = dot(col, vec3(0.299, 0.587, 0.114));
    col = mix(col, vec3(lum), desat * 0.5);

    frag_color = vec4(col, 1.0);
}
