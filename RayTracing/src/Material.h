#pragma once

#include <glm/glm.hpp>
#include "Utils.h"
#include "Ray.h"
#include "HitPayload.h"
#include "iostream"

// all possible materialTypes
enum MatrialType
{
    Lambertian,
    Metall,
    Dialectric,
};


struct Material
{
    // the type of this material
    MatrialType m_MaterialType = MatrialType::Lambertian;


    glm::vec3 Albedo{1.0f};
    // in RioW they call it fuzz -> lets see if correct -> can not be 0!
    float Roughness = 1.0f;

    // float Metallic = 0.0f;

    // if Dialectric
    float RefractionIndex = 1.5f;


    // we multiply the below ones together
    glm::vec3 EmissionColor{0.0f};
    float EmissionPower = 0.0f;

    // for emissive materials
    glm::vec3 GetEmission() const  
    {
        return EmissionColor * EmissionPower;
    }

    // returns if tracing should continue material color (often, not always albedo) and changes ray to reflected ray or refracted
    bool Reflect(Ray& incomingRay, const HitPayload& payload, uint32_t& seed, glm::vec3& color) const {

        
        if (m_MaterialType == Lambertian) { // Lambertian material

            incomingRay.Origin = payload.WorldPosition + 0.0001f * payload.WorldNormal;
            // lambertian reflection -> theoretically the term could cancel out, but will probably never happen...
            incomingRay.Direction = glm::normalize(payload.WorldNormal + Utils::InUnitSphere(seed));

            color = Albedo;

            // check if "reflected" ray goes into sphere
            if (!dot(incomingRay.Direction, payload.WorldNormal) > 0) {
                return false;
            }

            
            // don't continue if rays start inside
            return payload.FrontFace;
        }
        else if (m_MaterialType == Metall) { // Metallic material

            glm::vec3 reflected = Utils::Reflect(incomingRay.Direction, payload.WorldNormal);

            incomingRay.Direction = glm::normalize(reflected) + (Roughness * Utils::InUnitSphere(seed));
            incomingRay.Origin = payload.WorldPosition + 0.0001f * payload.WorldNormal;

            color = Albedo;

            // check if "reflected" ray goes into sphere
            if (!dot(incomingRay.Direction, payload.WorldNormal) > 0) {
                return false;
            }
            
            // don't continue if rays start inside
            return payload.FrontFace;
        }
        else if (m_MaterialType == Dialectric) { // Dialectric Material

            // NOTE: our normals always point outward (normals in book point to ray)
            
            float ri = payload.FrontFace ? (1.0f / RefractionIndex) : RefractionIndex;

            float cosTheta = payload.FrontFace ? std::fmin(dot(-incomingRay.Direction, payload.WorldNormal), 1.0f)
                                                : std::fmin(dot(-incomingRay.Direction,  -payload.WorldNormal), 1.0f);
            float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

            bool canNotRefract = ri * sinTheta > 1.0f;

            if (canNotRefract || Utils::Reflectance(cosTheta, ri) > Utils::RandomFloat(seed)) {

                glm::vec3 reflected = payload.FrontFace ? Utils::Reflect(incomingRay.Direction, payload.WorldNormal)
                                        : Utils::Reflect(incomingRay.Direction, -payload.WorldNormal);
                incomingRay.Direction = glm::normalize(reflected);
                incomingRay.Origin = payload.FrontFace ? payload.WorldPosition + 0.0001f * payload.WorldNormal : payload.WorldPosition - 0.0001f * payload.WorldNormal;

            }
            else {
                glm::vec3 refracted = payload.FrontFace ? Utils::Refract(incomingRay.Direction, payload.WorldNormal, ri) :
                    Utils::Refract(incomingRay.Direction, -payload.WorldNormal, ri);
                // NOTE: roughness could be added like above: + (Roughness * Utils::InUnitSphere(seed)) and coloring prbably too
                incomingRay.Direction = glm::normalize(refracted);

                incomingRay.Origin = payload.FrontFace ? payload.WorldPosition - 0.0001f * payload.WorldNormal : payload.WorldPosition + 0.0001f * payload.WorldNormal;

            }

            
            color = glm::vec3(1.0f);
            return true;
        }
        else { // no Material
            return false;
        }
    }

};
