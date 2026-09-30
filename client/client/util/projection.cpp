#include "client/util/projection.h"
#include <cmath>

namespace proj {

static constexpr double D2R = 3.14159265358979323846 / 180.0;

bool world_to_screen(const Vec3& pos, const Vec3& cam,
                     float yaw_deg, float pitch_deg, float fov_deg,
                     float screen_w, float screen_h,
                     float& sx, float& sy) {
    const double dx = pos.x - cam.x;
    const double dy = pos.y - cam.y;
    const double dz = pos.z - cam.z;

    // yaw: MC uses 0 = +Z; convert to math convention
    const double yaw = yaw_deg * D2R;
    const double pitch = pitch_deg * D2R;

    // rotate into camera space
    const double cy = cos(yaw), sy_ = sin(yaw);
    const double cp = cos(pitch), sp = sin(pitch);

    // MC camera basis:
    // forward = (-sin(yaw)*cos(pitch), -sin(pitch), cos(yaw)*cos(pitch))
    const double fx = -sy_ * cp, fy = -sp, fz = cy * cp;
    // right = (cos(yaw), 0, sin(yaw))
    const double rx = cy, ry = 0.0, rz = sy_;
    // up = right x forward
    const double ux = ry * fz - rz * fy;
    const double uy = rz * fx - rx * fz;
    const double uz = rx * fy - ry * fx;

    const double zc = dx * fx + dy * fy + dz * fz;   // depth
    const double xc = dx * rx + dy * ry + dz * rz;   // screen x
    const double yc = dx * ux + dy * uy + dz * uz;   // screen y

    if (zc <= 0.05) return false;   // behind camera

    const double tan_half = tan(fov_deg * D2R * 0.5);
    const double aspect = (double)screen_w / (double)screen_h;

    const double ndc_x = (xc / (zc * tan_half * aspect));
    const double ndc_y = (yc / (zc * tan_half));

    sx = (float)((ndc_x * 0.5 + 0.5) * screen_w);
    sy = (float)((-ndc_y * 0.5 + 0.5) * screen_h);
    return true;
}

} // namespace proj
