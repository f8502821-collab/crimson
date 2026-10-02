#pragma once
// Easing + spring helpers driving ALL animations (injector + client).
namespace ez {

// t in [0,1]
float ease_out_back(float t);
float ease_out_expo(float t);
float ease_out_cubic(float t);
float ease_in_out_quad(float t);

float lerp(float a, float b, float t);
float clamp01(float v);

// Framerate-independent smoothing: moves value toward target.
float damp(float current, float target, float speed, float dt);

// Critically damped spring (no bounce) - stable at any dt.
struct Spring {
    float value = 0.f, velocity = 0.f;
    void step(float target, float stiffness, float damping, float dt);
    void snap(float v) { value = v; velocity = 0.f; }
};

} // namespace ez
