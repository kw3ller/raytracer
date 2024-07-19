#pragma once
#include "Utils.h"

class Interval {

	// --- VRIABLES ---

public:
	float m_Min;
	float m_Max;

	static const Interval m_Empty;
	static const Interval m_Universe;

protected:

private:

	// --- METHODS ---


public:

	Interval();

	Interval(float min, float max);

	// size of interval
	float Size() const;

	// is x contained in interval
	bool Contains(float x) const;

	// does interval truely surround x
	bool Surrounds(float x) const;

	// expands interval by delta/2 in both directions -> new object
	Interval Expand(float delta) const;



protected:

private:


};


