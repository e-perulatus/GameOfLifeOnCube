#pragma once

#include <cstdint>
#include <cstdio>
#include <type_traits>

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

    void Print() noexcept
    {
        printf("(%2.3f, %2.3f, %2.3f)\n", x, y, z);
    }

    Vector3d operator+(const Vector3d& rhs) const
    {
        return { x + rhs.x, y + rhs.y, z + rhs.z };
    }
    Vector3d operator-(const Vector3d& rhs) const
    {
        return { x - rhs.x, y - rhs.y, z - rhs.z };
    }
    Vector3d operator*(const float value) const
    {
        return { x * value, y *value, z * value };
    }
    Vector3d& operator+=(const Vector3d& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
};

struct Axes3d
{
    Vector3d x;
    Vector3d y;
    Vector3d z;

    void Print() noexcept
    {
        x.Print();
        y.Print();
        z.Print();
    }

    Axes3d operator+(const Axes3d& rhs) const
    {
        return { x + rhs.x, y + rhs.y, z + rhs.z };
    }
    Axes3d operator-(const Axes3d& rhs) const
    {
        return { x - rhs.x, y - rhs.y, z - rhs.z };
    }
    Axes3d operator*(const float value) const
    {
        return { x * value, y *value, z * value };
    }
    Axes3d& operator+=(const Axes3d& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

};

}