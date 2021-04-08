#ifndef ADY_MATH_H
#define ADY_MATH_H

#include "ady_types.h"
#include <math.h>
#include <float.h>

inline i32 intmax(i32 a, i32 b) {
	return (a > b) ? a : b;
}

inline i32 intmin(i32 a, i32 b) {
	return (a < b) ? a : b;
}

inline f32 floatmax(f32 a, f32 b) {
	return (a > b) ? a : b;
}

inline f32 floatmin(f32 a, f32 b) {
	return (a < b) ? a : b;
}

const f32 PI = 3.14159265359f;

inline f32 DegToRad(f32 angle) {
	return (angle * (PI / 180.0f));
}

// Vector 2 ----------------------------------------------------------------

typedef struct vec2 {
	union {
		struct { f32 x; f32 y; };
		struct { f32 u; f32 v; };
		f32 data[2];
	};
} vec2;

inline vec2 Vec2(f32 x, f32 y) {
	return { x, y };
}

typedef struct ivec2 {
	union {
		struct { i32 x; i32 y; };
		struct { i32 u; i32 v; };
		i32 data[2];
	};
} ivec2;

inline ivec2 IVec2(i32 x, i32 y) {
	return { x, y };
}

inline i32 Vec2To1DIndex(vec2 v, i32 width) {
	return (i32)v.x + (i32)v.y * width;
}

inline b32 Vec2Equals(vec2 v1, vec2 v2) {
	return (b32)((v1.x == v2.x) && (v1.y == v2.y));
}

// Vector 3 ----------------------------------------------------------------

typedef struct vec3 {
	union {
		struct { f32 x; f32 y; f32 z; };
		struct { f32 u; f32 v; f32 w; };
		struct { f32 r; f32 g; f32 b; };
		f32 data[3];
	};
} vec3;

inline vec3 Vec3(f32 x, f32 y, f32 z) {
	return { x, y, z };
}

inline vec3 ZeroVec() {
	return { 0.0f, 0.0f, 0.0f };
}

inline vec3 OneVec() {
	return { 1.0f, 1.0f, 1.0f };
}

inline vec3 UpVec() {
	return { 0.0f, 1.0f, 0.0f };
}

inline b32 VecEquals(vec3 v1, vec3 v2) {
	return (b32)((v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z));
}

inline vec3 NegVec(vec3 v1) {
	return { -1.0f * v1.x, -1.0f * v1.y, -1.0f * v1.z };
}

inline vec3 AddVec(vec3 v1, vec3 v2) {
	return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

inline vec3 SubVec(vec3 v1, vec3 v2) {
	return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

inline vec3 ScaleVec(vec3 v1, f32 f) {
	return { v1.x * f, v1.y * f, v1.z * f };
}

inline vec3 MulVec(vec3 v1, vec3 v2) {
	return { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
}

inline f32 Dot(vec3 v1, vec3 v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline vec3 Cross(vec3 v1, vec3 v2) {
	return { v1.y * v2.z - v1.z * v2.y,
			 v1.z * v2.x - v1.x * v2.z,
			 v1.x * v2.y - v1.y * v2.x };
}

inline f32 Distance(vec3 v1, vec3 v2) {
	vec3 dist = SubVec(v2, v1);
	return sqrtf(Dot(dist, dist));
}

// TODO: Replace sqrtf with custom implementation?
inline f32 VecLen(vec3 v) {
	return sqrtf(Dot(v, v));
}

inline vec3 NormVec(vec3 v) {
	f32 len = VecLen(v);
	len = (len == 0.0f) ? 1.0f : 1.0f / len;
	return ScaleVec(v, len);
}

// Matrix ------------------------------------------------------------------
// Styled after raysan5's raymath, and HandmadeMath

typedef union mat4 {
	struct {
		f32 m0, m1, m2, m3;
		f32 m4, m5, m6, m7;
		f32 m8, m9, m10, m11;
		f32 m12, m13, m14, m15;
	};

	f32 data[4][4];

	// reminder of the layout
	//
	// [0][0] [0][1] [0][2] [0][3]
	// [1][0] [1][1] [1][2] [1][3]
	// [2][0] [2][1] [2][2] [2][3]
	// [3][0] [3][1] [3][2] [3][3]
	//

} mat4;

inline mat4 MulMat(mat4 m1, mat4 m2) {
	mat4 result = {};
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			f32 sum = 0;
			for (int i = 0; i < 4; i++) {
				sum += m2.data[row][i] * m1.data[i][col];
			}
			result.data[row][col] = sum;
		}
	}
	return result;
}

inline mat4 DiagonalMat(f32 diag) {
	mat4 result = {};

	result.data[0][0] = diag;
	result.data[1][1] = diag;
	result.data[2][2] = diag;
	result.data[3][3] = diag;

	return result;
}

inline mat4 TransposeMat(mat4 m) {
	mat4 result = {};

	result.data[0][0] = m.data[0][0];
	result.data[0][1] = m.data[1][0];
	result.data[0][2] = m.data[2][0];
	result.data[0][3] = m.data[3][0];

	result.data[1][0] = m.data[0][1];
	result.data[1][1] = m.data[1][1];
	result.data[1][2] = m.data[2][1];
	result.data[1][3] = m.data[3][1];

	result.data[2][0] = m.data[0][2];
	result.data[2][1] = m.data[1][2];
	result.data[2][2] = m.data[2][2];
	result.data[2][3] = m.data[3][2];

	result.data[3][0] = m.data[0][3];
	result.data[3][1] = m.data[1][3];
	result.data[3][2] = m.data[2][3];
	result.data[3][3] = m.data[3][3];

	return result;
}

inline mat4 TranslateMat(vec3 v) {
	mat4 result = DiagonalMat(1.0f);

	result.data[0][3] = v.x;
	result.data[1][3] = v.y;
	result.data[2][3] = v.z;

	return TransposeMat(result);
}

// Note: angle is in degrees, axis will be normalized internally
inline mat4 RotateMat(f32 angle, vec3 axis) {
	mat4 result = DiagonalMat(1.0f);
	axis = NormVec(axis);

	f32 SinTheta = sinf(DegToRad(angle));
	f32 CosTheta = cosf(DegToRad(angle));
	f32 CosValue = 1.0f - CosTheta;

	result.data[0][0] = (axis.x * axis.x * CosValue) + CosTheta;
	result.data[0][1] = (axis.x * axis.y * CosValue) + (axis.z * SinTheta);
	result.data[0][2] = (axis.x * axis.z * CosValue) - (axis.y * SinTheta);

	result.data[1][0] = (axis.y * axis.x * CosValue) - (axis.z * SinTheta);
	result.data[1][1] = (axis.y * axis.y * CosValue) + CosTheta;
	result.data[1][2] = (axis.y * axis.z * CosValue) + (axis.x * SinTheta);

	result.data[2][0] = (axis.z * axis.x * CosValue) + (axis.y * SinTheta);
	result.data[2][1] = (axis.z * axis.y * CosValue) - (axis.x * SinTheta);
	result.data[2][2] = (axis.z * axis.z * CosValue) + CosTheta;

	return result;
}

inline mat4 ScaleMat(vec3 v) {
	mat4 result = DiagonalMat(1.0f);

	result.data[0][0] = v.x;
	result.data[1][1] = v.y;
	result.data[2][2] = v.z;

	return result;
}


inline mat4 OrthographicMat(f32 left, f32 right, f32 bot, f32 top, f32 znear, f32 zfar) {
	mat4 result = {};

	result.data[0][0] = 2.0f / (right - left);
	result.data[1][1] = 2.0f / (top - bot);
	result.data[2][2] = 2.0f / (znear - zfar);
	result.data[3][3] = 1.0f;

	result.data[3][0] = (left + right) / (left - right);
	result.data[3][1] = (bot + top) / (bot - top);
	result.data[3][2] = (zfar + znear) / (znear - zfar);

	return result;
}

// Note: fov asks for an angle in degrees
inline mat4 PerspectiveMat(f32 fov, f32 aspect_ratio, f32 znear, f32 zfar) {
	mat4 result = {};

	f32 cotan = 1.0f / tanf(DegToRad(0.5f * fov));

	result.data[0][0] = cotan / aspect_ratio;
	result.data[1][1] = cotan;
	result.data[2][2] = (zfar) / (zfar - znear);
	result.data[3][2] = -(znear * zfar) / (zfar - znear);
	result.data[2][3] = 1.0f;
	result.data[3][3] = 0.0f;

	return result;
}

inline mat4 LookAtMat(vec3 eye, vec3 target, vec3 up) {
	mat4 result = {};

	vec3 n = NormVec(SubVec(target, eye));
	vec3 u = NormVec(Cross(up, n));
	vec3 v = Cross(n, u);

	// vec3 negeye = NegVec(eye);

	result.data[0][0] = u.x;
	result.data[0][1] = v.x;
	result.data[0][2] = n.x;
	result.data[0][3] = 0.0f;

	result.data[1][0] = u.y;
	result.data[1][1] = v.y;
	result.data[1][2] = n.y;
	result.data[1][3] = 0.0f;

	result.data[2][0] = u.z;
	result.data[2][1] = v.z;
	result.data[2][2] = n.z;
	result.data[2][3] = 0.0f;

	result.data[3][0] = -Dot(eye, u);
	result.data[3][1] = -Dot(eye, v);
	result.data[3][2] = -Dot(eye, n);
	result.data[3][3] = 1.0f;

	// result = TransposeMat(result);

	return result;
}

// ----------------------------------------------------------------------------
// Quaternion

/*
struct Quaternion {
	union {
		struct {
			f32 x, y, z, w;
		};
		struct {
			vec3 axis;
			f32 r;
		};
	};
};

inline f32 MagQuat(Quaternion q1) {
	return sqrtf(powf(q1.x, 2) + powf(q1.y, 2) + powf(q1.z, 2) + powf(q1.w, 2));
}

inline Quaternion NormQuat(Quaternion q1) {
	f32 magnitude = MagQuat(q1);
	q1.x /= magnitude;
	q1.y /= magnitude;
	q1.z /= magnitude;
	q1.w /= magnitude;
	return q1;
}

inline Quaternion CongQuat(Quaternion q) {
	return { -q.x, -q.y, -q.z, q.w };
}

inline Quaternion MulQuat(Quaternion q1, Quaternion q2) {
	Quaternion result = {};
	result.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
	result.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
	result.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;
	result.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
	return result;
}

inline Quaternion InverseQuat(Quaternion q) {
	Quaternion cq = CongQuat(q);
	f32 divisor = 1.0f / powf(MagQuat(q), 2);
	cq.x /= divisor;
	cq.y /= divisor;
	cq.z /= divisor;
	cq.w /= divisor;
	return cq;
}

// angle in degrees, it is converted to radians internally
inline Quaternion RotateQuat(Quaternion orientation, vec3 axis, f32 angle) {
	Quaternion rotation = {};
	angle = DegToRad(angle);
	rotation.x = axis.x * sinf(angle / 2);
	rotation.y = axis.y * sinf(angle / 2);
	rotation.z = axis.z * sinf(angle / 2);
	rotation.w = cosf(angle / 2);

	orientation = MulQuat(rotation, orientation);
	return orientation;
}

inline mat4 RotMatFromQuat(Quaternion q) {
	mat4 result = {};

	f32 x = q.x;
	f32 y = q.y;
	f32 z = q.z;
	f32 w = q.w;

	result.m0 = 1 - 2*y*y - 2*z*z;
	result.m1 = 2*x*y - 2*w*z;
	result.m2 = 2*x*z + 2*w*y;
	result.m3 = 0.0f;

	result.m4 = 2*x*y + 2*w*z;
	result.m5 = 1 - 2*x*x - 2*z*z;
	result.m6 = 2*y*z + 2*w*z;
	result.m7 = 0.0f;

	result.m8 = 2*x*z - 2*w*y;
	result.m9 = 2*y*z - 2*w*x;
	result.m10 = 1 - 2*x*x - 2*y*y;
	result.m11 = 0.0f;

	result.m12 = 0.0f;
	result.m13 = 0.0f;
	result.m14 = 0.0f;
	result.m15 = 1.0f;

	return result;
}

inline mat4 ViewMatFromQuat(Quaternion q, vec3 pos) {
	mat4 result = {};

	mat4 rotmat = TransposeMat(RotMatFromQuat(q));
	mat4 transmat = TranslateMat(NegVec(pos));

	result = MulMat(rotmat, transmat);
	return result;
}
*/

#endif