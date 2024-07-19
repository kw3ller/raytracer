#pragma once
#include "Ray.h"
#include "HitPayload.h"

// an abstract class for every object a ray might hit
class Hittable
{
public:
    glm::vec3 Position{0.0f};
    int MaterialIndex = 0;

public:
    // checks for hit and writes closest t in payload (negative if no hit)
    virtual void CheckHit(const Ray &ray, HitPayload &payload) const = 0;

    // writes in a payload closest hit of ray with an object (already knowing t (hitdistance))
    // so its worldPosition, WorldNormal etc.
    virtual void Hit(HitPayload &payload, const Ray &ra) const = 0;
};
