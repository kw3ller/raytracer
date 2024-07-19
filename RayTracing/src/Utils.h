#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <limits>

#define FLOAT_MIN std::numeric_limits<float>::min()
#define FLOAT_MAX std::numeric_limits<float>::max()

namespace Utils
{
    static uint32_t ConvertToRGBA(glm::vec4 &color)
    {
        uint8_t a = (uint8_t)(color.a * 255.0f);
        uint8_t b = (uint8_t)(color.b * 255.0f);
        uint8_t g = (uint8_t)(color.g * 255.0f);
        uint8_t r = (uint8_t)(color.r * 255.0f);
        return 0x00000000 | a << 24 | b << 16 | g << 8 | r;
    };

    // generates a "random" integer
    static uint32_t PCG_Hash(uint32_t input)
    {
        uint32_t state = input * 747796405u + 2891336453u;
        uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
        return (word >> 22u) ^ word;
    }

    // returns "random" float between 0 and 1
    static float RandomFloat(uint32_t &seed)
    {
        seed = PCG_Hash(seed);
        return (float)seed / (float)std::numeric_limits<uint32_t>::max();
    }

    // returns "random" float with min max values
    static float RandomFloat(float min, float max, uint32_t &seed)
    {
        return RandomFloat(seed) * (max - min) + min;
    }

    // returns "random" unit vector in unitSphere
    static glm::vec3 InUnitSphere(uint32_t &seed)
    {
        return glm::normalize(glm::vec3(RandomFloat(seed) * 2.0f - 1.0f, RandomFloat(seed) * 2.0f - 1.0f, RandomFloat(seed) * 2.0f - 1.0f));
    }

    // fast "random" vec 3 with min max values
    static glm::vec3 RandomVec3(float min, float max, uint32_t &seed)
    {
        return glm::vec3(RandomFloat(seed) * (max - min) + min, RandomFloat(seed) * (max - min) + min, RandomFloat(seed) * (max - min) + min);
    }

    // reflect a ray
    static inline glm::vec3 Reflect(const glm::vec3& vector, const glm::vec3& normal) {
        return vector - 2 * dot(vector, normal) * normal;
    }

    // refract a ray -> (vector has to be unit vector
    static inline glm::vec3 Refract(const glm::vec3& vector, const glm::vec3& normal, float etaiOverEtat) {
        auto cosTheta = std::fmin(glm::dot(-vector, normal), 1.0f);
        glm::vec3 rOutPerp = etaiOverEtat * (vector + cosTheta * normal);
        glm::vec3 rOutParallel = - sqrt(std::fabs(1.0f - (rOutPerp.x * rOutPerp.x + rOutPerp.y * rOutPerp.y + rOutPerp.z * rOutPerp.z))) * normal;
        return rOutPerp + rOutParallel;
    }

    // reflectance -> schlick approximation
    static inline float Reflectance(float cosine, float refractionIndex) {
        float r0 = (1 - refractionIndex) / (1 + refractionIndex);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
    

} // namespace Utils
