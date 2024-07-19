#pragma once
#include "Interval.h"
#include "Ray.h"

class AABB {
	
	// --- VARIABLES ---

public:

	Interval m_X;
	Interval m_Y;
	Interval m_Z;

protected:

private:

	// --- METHODS ---

public:

	// intervals are emtpy by default
	AABB();

	// normal initialization
	AABB(const Interval& x, const Interval& y, const Interval& z);

	// treat point a and b as extremes to construct the box
	AABB(const glm::vec3& a, const glm::vec3& b);

	// returns interval at given axis
	const Interval& AxisInterval(int n) const {
		if (n == 1) return m_Y;
		if (n == 2) return m_Z;
		return m_X;
	}

	// returns axis of biggest interval
	int BiggestIntervalAxis() const {
		if (m_Y.Size() > m_X.Size() && m_Y.Size() > m_Z.Size()) {
			return 1;
		} 
		if (m_Z.Size() > m_X.Size() && m_Z.Size() > m_Y.Size()) {
			return 2;
		}
		return 0;
	}

	// check for hit 
	bool Hit(const Ray& ray) const;



protected:

private:


	
};