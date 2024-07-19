#pragma once
#include "Hittable.h"
#include "Material.h"
#include <glm/glm.hpp>

class Sphere : public Hittable
{
public:
    float Radius = 0.5f;

public:
    void CheckHit(const Ray &ray, HitPayload &payload) const override;

    void Hit(HitPayload &payload, const Ray &ray) const override;
};

