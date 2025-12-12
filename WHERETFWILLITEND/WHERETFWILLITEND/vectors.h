#pragma once
#include <cmath>
#include <iostream>

template <class t> struct Vec2 {
	struct {t x, y;};
	Vec2() : x(0), y(0) {}
	Vec2(t _u, t _v) : x(_u),y(_v) {}
	Vec2(t _u) : x(_u), y(_u) {}
	inline Vec2<t> operator +(const Vec2<t> &V) const { return Vec2<t>(x+V.x, y+V.y); }
	inline Vec2<t> operator -(const Vec2<t> &V) const { return Vec2<t>(x-V.x, y-V.y); }
	inline Vec2<t> operator *(float f) const { return Vec2<t>(x*f, y*f); }
	float length() const { return std::sqrt(x * x + y * y); }
	template <class > friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
};

template <class t> struct Vec3 {
	struct {t x, y, z;};
	Vec3() : x(0), y(0), z(0) {}
	Vec3(t _x, t _y, t _z) : x(_x),y(_y),z(_z) {}
	Vec3(t _x) : x(_x), y(_x), z(_x) {}
	inline Vec3<t> operator ^(const Vec3<t> &v) const { return Vec3<t>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
	inline Vec3<t> operator +(const Vec3<t> &v) const { return Vec3<t>(x+v.x, y+v.y, z+v.z); }
	inline Vec3<t> operator -(const Vec3<t> &v) const { return Vec3<t>(x-v.x, y-v.y, z-v.z); }
	inline Vec3<t> operator *(float f) const { return Vec3<t>(x*f, y*f, z*f); }
	inline t operator *(const Vec3<t> &v) const { return x*v.x + y*v.y + z*v.z; }
	float length () const { return std::sqrt(x*x+y*y+z*z); }
	Vec3<t> & normalize() { *this = (*this)*(1.0/ length()); return *this; }
	template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
};

typedef Vec2<float> Vec2f;
typedef Vec2<int>   Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;

template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) {
	s << "(" << v.x << ", " << v.y << ")\n";
	return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
	s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
	return s;
}