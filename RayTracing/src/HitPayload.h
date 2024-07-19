#pragma once
#include "glm/glm.hpp"

// TODO: maybe later put it in a namespace utils?!
struct HitPayload
{
    // the t in the linear function -> should be positive otherwise no hit or hit behind
    float HitDistance;
    // position of hit (in WorldSpace)
    glm::vec3 WorldPosition;
    // normal of hit (in WorldSpace) (always points out of object)
    glm::vec3 WorldNormal;
    // tells if ray is inside object or outside
    bool FrontFace;
    // which object got hit (from scene)
    int ObjectIndex;
};
