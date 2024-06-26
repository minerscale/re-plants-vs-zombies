#ifndef __SEXYVECTOR_H__
#define __SEXYVECTOR_H__

#include <math.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/norm.hpp>

namespace Sexy {
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class SexyVector2 {
public:
    union {
        struct {
            float x, y;
        };

        glm::vec2 vec;
    };

public:
    SexyVector2() : x(0), y(0) {}

    SexyVector2(const glm::vec2 &theVec) : vec(theVec) {}

    SexyVector2(float theX, float theY) : x(theX), y(theY) {}

    float Dot(const SexyVector2 &v) const { return x * v.x + y * v.y; }
    SexyVector2 operator+(const SexyVector2 &v) const { return SexyVector2(x + v.x, y + v.y); }
    SexyVector2 operator-(const SexyVector2 &v) const { return SexyVector2(x - v.x, y - v.y); }
    SexyVector2 operator-() const { return SexyVector2(-x, -y); }
    SexyVector2 operator*(float t) const { return SexyVector2(t * x, t * y); }
    SexyVector2 operator/(float t) const { return SexyVector2(x / t, y / t); }

    void operator+=(const SexyVector2 &v) {
        x += v.x;
        y += v.y;
    }

    void operator-=(const SexyVector2 &v) {
        x -= v.x;
        y -= v.y;
    }

    void operator*=(float t) {
        x *= t;
        y *= t;
    }

    void operator/=(float t) {
        x /= t;
        y /= t;
    }

    bool operator==(const SexyVector2 &v) { return x == v.x && y == v.y; }
    bool operator!=(const SexyVector2 &v) { return x != v.x || y != v.y; }

    float Magnitude() const { return sqrtf(x * x + y * y); }
    float MagnitudeSquared() const { return x * x + y * y; }

    SexyVector2 Normalize() const {
        float aMag = Magnitude();
        return aMag != 0 ? (*this) / aMag : *this;
    }

    SexyVector2 Perp() const { return {-y, x}; }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class SexyVector3 {
public:
    union {
        struct {
            float x, y, z;
        };

        glm::vec3 vec;
    };

public:
    SexyVector3() : x(0), y(0), z(0) {}

    SexyVector3(const glm::vec3 &theVec) : vec(theVec) {}

    SexyVector3(float theX, float theY, float theZ) : x(theX), y(theY), z(theZ) {}

    float Dot(const SexyVector3 &v) const { return x * v.x + y * v.y + z * v.z; }

    SexyVector3 Cross(const SexyVector3 &v) const { return {y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x}; }

    SexyVector3 operator+(const SexyVector3 &v) const { return {x + v.x, y + v.y, z + v.z}; }
    SexyVector3 operator-(const SexyVector3 &v) const { return {x - v.x, y - v.y, z - v.z}; }
    SexyVector3 operator*(float t) const { return {t * x, t * y, t * z}; }
    SexyVector3 operator/(float t) const { return {x / t, y / t, z / t}; }
    float Magnitude() const { return sqrtf(x * x + y * y + z * z); }

    SexyVector3 Normalize() const {
        float aMag = Magnitude();
        return aMag != 0 ? (*this) / aMag : *this;
    }
};
}; // namespace Sexy

#endif
