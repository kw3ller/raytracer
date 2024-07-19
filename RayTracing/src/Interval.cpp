#include "Interval.h"


Interval::Interval()
	: m_Min(FLOAT_MAX), m_Max(FLOAT_MIN) {
}

Interval::Interval(float min, float max)
	: m_Min(min), m_Max(max) {
}

float Interval::Size() const {
	return m_Max - m_Min;
}

bool Interval::Contains(float x) const {
	return m_Min <= x <= m_Max;
}

bool Interval::Surrounds(float x) const {
	return m_Min < x < m_Max;
}

Interval Interval::Expand(float delta) const {
	float padding = delta / 2;
	return Interval(m_Min - padding, m_Max + padding);
}

const Interval Interval::m_Empty = Interval(FLOAT_MAX, FLOAT_MIN);
const Interval Interval::m_Universe = Interval(FLOAT_MIN, FLOAT_MAX);