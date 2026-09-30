#pragma once
// World->screen projection from yaw/pitch/fov (matches MC's view math).
namespace proj {

struct Vec3 { double x, y, z; };

// Returns true if in front of camera; writes screen px into sx/sy.
bool world_to_screen(const Vec3& pos, const Vec3& cam,
                     float yaw_deg, float pitch_deg, float fov_deg,
                     float screen_w, float screen_h,
                     float& sx, float& sy);

} // namespace proj
