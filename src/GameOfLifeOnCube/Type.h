#pragma once

#include <cstdint>
#include <cstdio>

namespace Gol3d {

struct Position
{
    int x;
    int y;
};

struct FPosition
{
    float x;
    float y;
};

struct Rect
{
    int width;
    int height;
};

enum class InputFlag : uint8_t
{
    None = 0,
    Select = 1 << 0,
    Enter  = 1 << 1,
    Cancel = 1 << 2,
};

struct Vector3d
{
    float x;
    float y;
    float z;

    void Print() const noexcept
    {
        printf("(%2.3f, %2.3f, %2.3f)\n", x, y, z);
    }

    Vector3d& operator+=(const Vector3d& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
};
inline Vector3d operator+(const Vector3d& lhs, const Vector3d& rhs) noexcept
{
    return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}
inline Vector3d operator-(const Vector3d& lhs, const Vector3d& rhs) noexcept
{
    return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}
inline Vector3d operator*(const Vector3d& lhs, const float value) noexcept
{
    return { lhs.x * value, lhs.y * value, lhs.z * value };
}
inline Vector3d operator*(const float value, const Vector3d& rhs) noexcept
{
    return { rhs.x * value, rhs.y * value, rhs.z * value };
}

struct Axes3d
{
    Vector3d x;
    Vector3d y;
    Vector3d z;

    void Print() const noexcept
    {
        x.Print();
        y.Print();
        z.Print();
    }

    Axes3d& operator+=(const Axes3d& rhs) noexcept
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

};
inline Axes3d operator+(const Axes3d& lhs, const Axes3d& rhs) noexcept
{
    return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}
inline Axes3d operator-(const Axes3d& lhs, const Axes3d& rhs) noexcept
{
    return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}
inline Axes3d operator*(const Axes3d& lhs, const float value) noexcept
{
    return { lhs.x * value, lhs.y * value, lhs.z * value };
}
inline Axes3d operator*(const float value, const Axes3d& rhs) noexcept
{
    return { rhs.x * value, rhs.y * value, rhs.z * value };
}

struct Quaternion
{
    float x;
    float y;
    float z;
    float w;

    Quaternion Inverse() noexcept
    {
        return { -x, -y, -z, w };
    }
    explicit operator Vector3d() const noexcept
    {
        return { x, y, z };
    }
};
inline Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs) noexcept
{
    return {
        lhs.x * rhs.w + lhs.w * rhs.x - lhs.z * rhs.y + lhs.y * rhs.z,
        lhs.y * rhs.w + lhs.z * rhs.x + lhs.w * rhs.y - lhs.x * rhs.z,
        lhs.z * rhs.w - lhs.y * rhs.x + lhs.x * rhs.y + lhs.w * rhs.z,
        lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z
    }; 
}

}