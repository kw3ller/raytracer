#include "AABB.h"
#include "iostream"



AABB::AABB() {

}

AABB::AABB(const Interval& x, const Interval& y, const Interval& z) 
		: m_X(x), m_Y(y), m_Z(z) 
{

}

AABB::AABB(const glm::vec3& a, const glm::vec3& b) {
	m_X = (a.x <= b.x) ? Interval(a.x, b.x) : Interval(b.x, a.x);
	m_Y = (a.y <= b.y) ? Interval(a.y, b.y) : Interval(b.y, a.y);
	m_Y = (a.z <= b.z) ? Interval(a.z, b.z) : Interval(b.z, a.z);
}

bool AABB::Hit(const Ray& ray) const {

	// NOTE: maybe t0 should be FLOAT_MIN ?!
	Interval rayT(FLOAT_MIN, FLOAT_MAX);
	
	for (int axis = 0; axis < 3; axis++) {
		const Interval& ax = AxisInterval(axis);
		const float adInv = 1.0f / ray.Direction[axis];

		float t0 = (ax.m_Min - ray.Origin[axis]) * adInv;
		float t1 = (ax.m_Max - ray.Origin[axis]) * adInv;

		if (t0 < t1) {
			if (t0 > rayT.m_Min) rayT.m_Min = t0;
			if (t1 < rayT.m_Max) rayT.m_Max = t1;
		}
		else {
			if (t1 > rayT.m_Min) rayT.m_Min = t1;
			if (t0 < rayT.m_Max) rayT.m_Max = t0;
		}

		if (rayT.m_Max <= rayT.m_Min) {
			return false;
		}
	}
	return true;
}