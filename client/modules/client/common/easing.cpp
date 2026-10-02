#include "common/easing.h"
#include <cmath>

namespace ez {

static constexpr float PI = 3.14159265358979f;

float ease_out_back(float t) {
    const float c1 = 1.70158f, c3 = c1 + 1.f;
    const float x = t - 1.f;
    return 1.f + c3 * x * x * x + c1 * x * x;
}

float ease_out_expo(float t) {
    return t >= 1.f ? 1.f : 1.f - std::pow(2.f, -10.f * t);
}

float ease_out_cubic(float t) {
    const float u = 1.f - t;
    return 1.f - u * u * u;
}

float ease_in_out_quad(float t) {
    return t < 0.5f ? 2.f * t * t : 1.f - std::pow(-2.f * t + 2.f, 2.f) / 2.f;
}

float lerp(float a, float b, float t) { return a + (b - a) * t; }

float clamp01(float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }

float damp(float current, float target, float speed, float dt) {
    return lerp(current, target, 1.f - std::exp(-speed * dt));
}

void Spring::step(float target, float stiffness, float damping, float dt) {
    if (dt <= 0.f) { value = target; return; }
    dt = dt > 0.1f ? 0.1f : dt;
    const float f = -stiffness * (value - target) - damping * velocity;
    velocity += f * dt;
    value += velocity * dt;
}

} // namespace ez
