#ifndef EGG_MATH_QUAT_H
#define EGG_MATH_QUAT_H
#include "eggVector.h"
#include "types_egg.h"

namespace EGG {

Quatf operator*(const Quatf& lhs, const Vector3f& rhs);
Quatf operator*(const Quatf& lhs, const Quatf& rhs);

struct Quatf {
    Quatf() {}
    Quatf(f32 _w, Vector3f vec) : x(vec.x), y(vec.y), z(vec.z), w(_w) {}

    ~Quatf() {}

    void setAxisRotation(const Vector3f&, f32);
    Quatf conjugate() const;

    Vector3f rotateVector(const Vector3f& v);
    void slerpTo(const Quatf& q2, f32 t, Quatf& out) const;

    f32 x, y, z, w;
};
} // namespace EGG

#endif
