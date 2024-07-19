#pragma once

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"
#include "Sphere.h"
#include "Walnut/Image.h"
#include "glm/vec3.hpp"
#include <cstdint>
#include <glm/fwd.hpp>
#include <memory>
#include <vector>

// idea is to have a scene discription of what we are trying to render
class Renderer
{
public:
  // these are the render settings
  struct Settings
  {
    bool Accumulate = true;
    // number of light bounces (must be at least 1) 
    uint32_t Bounces = 10;
    // for antialiasing:
    // turn on or off
    bool AntiAliasing = false;
    // the size for each pixel -> default = 0.001f
    float PixelSize = 0.001f;
    // the number of rays to sample through each pixel for antiAliasing -> default = 10
    uint8_t SamplesPerPixel = 10;
  };

public:
  // should later have a scene
  Renderer() = default;

  void OnResize(uint32_t width, uint32_t height);
  void Render(const Scene &scene, const Camera &camera);

  std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }
  void ResetFrameIndex() { m_FrameIndex = 1; }
  Settings &GetSettings() { return m_Settings; }

private:
  // RayGen shader
  glm::vec4 PerPixel(uint32_t x, uint32_t y);

  // upper level test
  void TraceRay(const Ray& ray, HitPayload& payload, const uint32_t nodeIndx);

  // check ray against BVH with nodeIndx
  void IntersectBVH(const Ray& ray, HitPayload& payload, const uint32_t nodeIndx);

  // traces ray and gives payload to get information if we hit stuff, how far away irt is etc 
  // trace ray against Hittables from hittIndx1 to hittIndx2
  void TraceRay(const Ray &ray, HitPayload &payload);
  // closest hit shader wich calculates the closest hit of a ray
  void ClosestHit(const Ray &ray, HitPayload &payload, int objectIndex);
  // Miss shader when a ray misses all objects
  void Miss(const Ray &ray, HitPayload &payload);

private:
  std::shared_ptr<Walnut::Image> m_FinalImage;
  Settings m_Settings;

  // used for the std::for_each() -> multithreading
  std::vector<uint32_t> m_ImageHorizontalIterator;
  std::vector<uint32_t> m_ImageVerticalIterator;

  const Scene *m_ActiveScene = nullptr;
  const Camera *m_ActiveCamera = nullptr;

  // the image that gets shown
  uint32_t *m_ImageData = nullptr;
  // accumulates the image data to trace different paths
  glm::vec4 *m_AccumulationData = nullptr;

  // to accumulate the m_AccumulationData it's important that we know wich frame we are in (since camera is still)
  // it is used to workout an average of the m_AccumulationData
  uint32_t m_FrameIndex = 1;
};
