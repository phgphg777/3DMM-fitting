#ifndef OGL_BASIC
#define OGL_BASIC

#include <cmath>
#include <memory.h>

namespace ogl {

class vec3 {
public:
	union { 
		struct {
			float x, y, z;
		};
		float data[3];
	};

public:
	typedef float value_type;

	vec3(float _x=0, float _y=0, float _z=0) : x(_x), y(_y), z(_z) {}
	vec3(const vec3& v) : x(v.x), y(v.y), z(v.z) {}
	float& operator[] (int i) { return data[i]; }
	float operator[] (int i) const { return data[i]; }
	vec3& operator=(const vec3& v) { x = v.x; y = v.y; z = v.z; return *this; }
	vec3& operator+=(const vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
	vec3& operator-=(const vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	vec3& operator*=(float r) { x*=r; y*=r; z*=r; return *this; }
	vec3& operator/=(float r) { x/=r; y/=r; z/=r; return *this; }
	vec3 operator+(const vec3& v) const { return vec3(x + v.x, y + v.y, z + v.z); }
	vec3 operator-(const vec3& v) const { return vec3(x - v.x, y - v.y, z - v.z); }
	vec3 operator*(float r) const { return vec3(x*r, y*r, z*r); }
	vec3 operator/(float r) const { return vec3(x/r, y/r, z/r); }
	float norm() const { return sqrtf(x*x + y*y + z*z); }
	float dot(const vec3& v) const { return x*v.x + y*v.y + z*v.z; }
	vec3 cross(const vec3& v) const { return vec3(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x); }
	vec3 normalize() { (*this)/=(*this).norm(); return *this; }
};
inline vec3 operator*(float r, const vec3& v) {
	return v*r;
}

class vec4 {
public:
	union { 
		struct {
			float x, y, z, w;
		};
		float data[4];
	};

public:
	typedef float value_type;

	vec4(float _x=0, float _y=0, float _z=0, float _w=0) : x(_x), y(_y), z(_z), w(_w) {}
	vec4(const vec3& v, float _w=1.0f) : x(v.x), y(v.y), z(v.z), w(_w) {}
	float& operator[] (int i) { return data[i]; }
	float operator[] (int i) const { return data[i]; }
	vec3 xyz() const { return vec3(x,y,z); }
};


class mat4 {
	union {
		struct {
			float m00, m10, m20, m30, m01, m11, m21, m31, m02, m12, m22, m32, m03, m13, m23, m33;
		};
		float m[4][4];
		float data[16];
	};

public:
	typedef float value_type;

	mat4(float s=1.0f) {
		for(int i=0; i<16; i++)
			if(i%5==0) data[i] = s;
			else data[i] = 0.0f;
	}
	mat4(const mat4& B) {
		memcpy(this, &B, sizeof(mat4));
	}
	void operator=(const mat4& B) {
		memcpy(this, &B, sizeof(mat4));
	}
	float& operator[](int i) {
		return data[i];
	}
	float operator[](int i) const {
		return data[i];
	}
	float& operator()(int i, int j) {
		return data[j * 4 + i];
	}
	float operator()(int i, int j) const {
		return data[j*4 + i];
	}
	mat4 operator*(const mat4& B) const {
		mat4 C;
		C[0] = (*this)[0]*B[0] + (*this)[4]*B[1] + (*this)[8]*B[2] + (*this)[12]*B[3];
		C[1] = (*this)[1]*B[0] + (*this)[5]*B[1] + (*this)[9]*B[2] + (*this)[13]*B[3];
		C[2] = (*this)[2]*B[0] + (*this)[6]*B[1] + (*this)[10]*B[2] + (*this)[14]*B[3];
		C[3] = (*this)[3]*B[0] + (*this)[7]*B[1] + (*this)[11]*B[2] + (*this)[15]*B[3];

		C[4] = (*this)[0]*B[4] + (*this)[4]*B[5] + (*this)[8]*B[6] + (*this)[12]*B[7];
		C[5] = (*this)[1]*B[4] + (*this)[5]*B[5] + (*this)[9]*B[6] + (*this)[13]*B[7];
		C[6] = (*this)[2]*B[4] + (*this)[6]*B[5] + (*this)[10]*B[6] + (*this)[14]*B[7];
		C[7] = (*this)[3]*B[4] + (*this)[7]*B[5] + (*this)[11]*B[6] + (*this)[15]*B[7];

		C[8] = (*this)[0]*B[8] + (*this)[4]*B[9] + (*this)[8]*B[10] + (*this)[12]*B[11];
		C[9] = (*this)[1]*B[8] + (*this)[5]*B[9] + (*this)[9]*B[10] + (*this)[13]*B[11];
		C[10] = (*this)[2]*B[8] + (*this)[6]*B[9] + (*this)[10]*B[10] + (*this)[14]*B[11];
		C[11] = (*this)[3]*B[8] + (*this)[7]*B[9] + (*this)[11]*B[10] + (*this)[15]*B[11];

		C[12] = (*this)[0]*B[12] + (*this)[4]*B[13] + (*this)[8]*B[14] + (*this)[12]*B[15];
		C[13] = (*this)[1]*B[12] + (*this)[5]*B[13] + (*this)[9]*B[14] + (*this)[13]*B[15];
		C[14] = (*this)[2]*B[12] + (*this)[6]*B[13] + (*this)[10]*B[14] + (*this)[14]*B[15];
		C[15] = (*this)[3]*B[12] + (*this)[7]*B[13] + (*this)[11]*B[14] + (*this)[15]*B[15];
		return C;
	}
	vec4 operator*(const vec4& v) const {
		vec4 w;
		w.x = (*this)[0]*v.x + (*this)[4]*v.y + (*this)[8]*v.z + (*this)[12]*v.w;
		w.y = (*this)[1]*v.x + (*this)[5]*v.y + (*this)[9]*v.z + (*this)[13]*v.w;
		w.z = (*this)[2]*v.x + (*this)[6]*v.y + (*this)[10]*v.z + (*this)[14]*v.w;
		w.w = (*this)[3]*v.x + (*this)[7]*v.y + (*this)[11]*v.z + (*this)[15]*v.w;
		return w;
	}
};

inline mat4 scale(float r)
{
	mat4 A;
	A[0] = A[5] = A[10] = r;
	return A;
}
inline mat4 translate(float x, float y, float z)
{
	mat4 A;
	A[12] = x;
	A[13] = y;
	A[14] = z;
	return A;
}
inline mat4 translate(const vec3& v)
{
	return translate(v.x, v.y, v.z);
}
inline mat4 rotate(float degree, float x, float y, float z)
{
	float oneDegree = 3.14159265358979f/180;
	float c = cosf(degree*oneDegree);
	float s = sinf(degree*oneDegree);
	mat4 A;
	A[0] = x*x*(1-c) + c;
	A[1] = x*y*(1-c) + z*s;
	A[2] = x*z*(1-c) - y*s;
	A[4] = y*x*(1-c) - z*s;
	A[5] = y*y*(1-c) + c;
	A[6] = y*z*(1-c) + x*s;
	A[8] = z*x*(1-c) + y*s;
	A[9] = z*y*(1-c) - x*s;
	A[10] = z*z*(1-c) + c;
	return A;
}
inline mat4 rotate(float degree, const vec3& v)
{
	return rotate(degree, v.x, v.y, v.z);
}
inline mat4 rotateXandY(float x_degree, float y_degree) 
//First, rotate w.r.t x-axis. Then, rotate w.r.t rotated y-axis
{
	mat4 RotX = rotate(y_degree, { 1, 0, 0 });
	vec4 upvector = RotX * vec4({ 0, 1, 0 }, 1.0f);
	return rotate(x_degree, upvector.xyz()) * RotX;
}
inline mat4 frustum(float left, float right, float bottom, float top, float zNear, float zFar)
{
	mat4 A;
    A[0] = 2*zNear / (right-left);
    A[5] = 2*zNear / (top-bottom);
    A[8]  = (right+left) / (right-left);
    A[9]  = (top+bottom) / (top-bottom);
    A[10] = (zNear+zFar) / (zNear-zFar);
    A[11] = -1;  
	A[14] = 2*zNear*zFar / (zNear-zFar);
    A[15] = 0;
	return A;
}
inline mat4 perspective(float fovy, float aspect, float zNear=0.001f, float zFar=1000.0f)
{
	float oneDegree = 3.14159265358979f/180;
	float tangent = tanf(fovy/2 * oneDegree);
    float half_h = zNear * tangent;
    float half_w = half_h * aspect;         

    return frustum(-half_w, half_w, -half_h, half_h, zNear, zFar);
}
inline mat4 lookAt(const vec3& eye, const vec3& target, const vec3& up)
{
	vec3 zDir = eye-target;
	zDir /= zDir.norm();
	vec3 xDir = up.cross(zDir);
	xDir /= xDir.norm();
	vec3 yDir = zDir.cross(xDir);
	yDir /= yDir.norm();

	mat4 A;
	A[0] = xDir.x;
	A[1] = yDir.x;
	A[2] = zDir.x;
		   
	A[4] = xDir.y;
	A[5] = yDir.y;
	A[6] = zDir.y;

	A[8]  = xDir.z;
	A[9]  = yDir.z;
	A[10] = zDir.z;
	
	A[12] = -(xDir.dot(eye));
	A[13] = -(yDir.dot(eye));
	A[14] = -(zDir.dot(eye));
	return A;
}

}; //namespace ogl


#endif //OGL_MATRIX