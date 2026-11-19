#pragma once
#include <cmath>
#include <algorithm>

// Enum chế độ thời gian chơi
enum class TimeMode {
    MIN_3 = 0,   // 3 Phút
    MIN_15 = 1,  // 15 Phút
    ENDLESS = 2  // Vô tận (Endless)
};

// Enum cấp độ khó
enum class Difficulty {
    EASY = 0,    // Dễ (Tốc độ rơi x0.5, Điểm x1)
    NORMAL = 1,  // Trung bình (Tốc độ rơi x1.0, Điểm x8)
    HARD = 2     // Khó (Tốc độ rơi x2.0, Điểm x20, mỗi 15 khối sinh 1 ô rác)
};

inline const char* getTimeModeName(TimeMode m) {
    switch (m) {
        case TimeMode::MIN_3: return "3 Phút";
        case TimeMode::MIN_15: return "15 Phút";
        case TimeMode::ENDLESS: return "Vô Tận";
    }
    return "3 Phút";
}

inline const char* getDifficultyName(Difficulty d) {
    switch (d) {
        case Difficulty::EASY: return "Dễ";
        case Difficulty::NORMAL: return "Trung Bình";
        case Difficulty::HARD: return "Khó";
    }
    return "Trung Bình";
}

// Vector 2D số thực
struct Vec2 {
    float x, y;
    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float _x, float _y) : x(_x), y(_y) {}
};

// Vector 3D số thực
struct Vec3 {
    float x, y, z;
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }

    float lengthSq() const { return x * x + y * y + z * z; }
    float length() const { return std::sqrt(lengthSq()); }

    Vec3 normalized() const {
        float l = length();
        if (l > 1e-6f) return (*this) * (1.0f / l);
        return Vec3(0, 0, 0);
    }
};

// Vector 3D số nguyên cho tọa độ ô lưới (Voxel grid)
struct Vec3i {
    int x, y, z;
    Vec3i() : x(0), y(0), z(0) {}
    Vec3i(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}

    bool operator==(const Vec3i& o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const Vec3i& o) const { return !(*this == o); }
    Vec3i operator+(const Vec3i& o) const { return Vec3i(x + o.x, y + o.y, z + o.z); }
    Vec3i operator-(const Vec3i& o) const { return Vec3i(x - o.x, y - o.y, z - o.z); }
};

// Màu sắc RGBA (0.0f - 1.0f)
struct Color4f {
    float r, g, b, a;
    Color4f() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    Color4f(float _r, float _g, float _b, float _a = 1.0f) : r(_r), g(_g), b(_b), a(_a) {}

    Color4f withAlpha(float newA) const {
        return Color4f(r, g, b, newA);
    }

    Color4f blend(const Color4f& o, float t) const {
        return Color4f(
            r + (o.r - r) * t,
            g + (o.g - g) * t,
            b + (o.b - b) * t,
            a + (o.a - a) * t
        );
    }
};
