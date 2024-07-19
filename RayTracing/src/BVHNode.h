#pragma once

#include "AABB.h"


class BVHNode {
	

	// --- VARIABLES ---

public:

	AABB m_BoundingBox;

	uint32_t m_LeftChildIndx;
	uint32_t m_RightChildIndx;

	uint32_t m_FirstPrimIndx;
	uint32_t m_PrimCount;




protected:

private:

	// --- METHODS ---

public:

	// TODO: implement
	void CheckHit();

	

protected:

private:

};