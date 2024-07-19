#include "Sphere.h"

void Sphere::CheckHit(const Ray &ray, HitPayload &payload) const
{
    glm::vec3 origin = ray.Origin - Position;

    float a = glm::dot(ray.Direction, ray.Direction);
    float b = 2.0f * glm::dot(origin, ray.Direction);
    float c = glm::dot(origin, origin) - Radius * Radius;

    // Quadratic forumula discriminant:
    // b^2 - 4ac

    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f)
    {
        // nothing gets hit
        payload.HitDistance = -1.0f;
        return;
    }

    // Quadratic formula:
    // (-b +- sqrt(discriminant)) / 2a

    // float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a); // Second hit distance (currently unused)
    // This is the closest t -> if it is negetive we are inside the object or the object is behind us -> and it will not be used further in renderer.cpp
    // TODO: überlegen: wie kriegen wir frontface/backface in payload rein -> wichtig für Glas Kugeln -> vllt nach Material sortieren/vorher prüfen?
    // vllt beide ts ausrechnen -> eines positiv eines negativ -> wir sind drinnen
    float sqrt = glm::sqrt(discriminant);
    payload.HitDistance = (-b - sqrt) / (2.0f * a);

    // ray is inside
    if (payload.HitDistance < 0.0f)
    {
        payload.HitDistance = (-b + sqrt) / (2.0f * a);
    }
    
    
}

void Sphere::Hit(HitPayload &payload, const Ray &ray) const
{
    
    glm::vec3 origin = ray.Origin - Position;
    payload.WorldPosition = origin + ray.Direction * payload.HitDistance;
    payload.WorldNormal = glm::normalize(payload.WorldPosition);
    payload.WorldPosition += Position;
    payload.FrontFace = glm::dot(ray.Direction, payload.WorldNormal) < 0;
    
};
