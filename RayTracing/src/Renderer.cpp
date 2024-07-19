#include "Renderer.h"
#include "Walnut/Random.h"
#include <cstdint>
#include <execution>
#include <cstring>
#include <future>
#include <glm/ext/quaternion_geometric.hpp>
#include <limits>
#include <thread>
#include <vector>
#include "Utils.h"

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// no resize nesesary
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
		{
			return;
		}

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];

	m_ImageHorizontalIterator.resize(width);
	m_ImageVerticalIterator.resize(height);

	// initialize iterator with values
	for (uint32_t i = 0; i < width; i++)
		m_ImageHorizontalIterator[i] = i;
	for (uint32_t i = 0; i < height; i++)
		m_ImageVerticalIterator[i] = i;
	;
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	if (m_FrameIndex == 1)
	{
		// set m_AccumulationData to zeros
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));
	}

	// if MT 1 multithreading is enabled
#define MT 1
#if MT

  // get number of CPU cores for number of threads
	unsigned int numThreads = 16;
	unsigned int heightSize = m_FinalImage->GetHeight();
	unsigned int chunkSize = heightSize / numThreads;

	std::vector<std::pair<unsigned int, unsigned int>> threadRanges;
	for (unsigned int i = 0; i < numThreads; i++)
	{
		unsigned int start = i * chunkSize;
		unsigned int end = (i == numThreads - 1) ? heightSize : start + chunkSize;
		threadRanges.emplace_back(start, end);
	}
	std::vector<std::thread> threads;
	for (unsigned int i = 0; i < numThreads; i++)
	{
		threads.emplace_back([this](unsigned int start, unsigned int end)
			{
				for (unsigned int y = start; y < end; y++) {
					for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
						glm::vec4 color = PerPixel(x, y);

						m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

						glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
						accumulatedColor /= (float)m_FrameIndex;

						accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));

						m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
					}

				} 
			},
			threadRanges[i].first, threadRanges[i].second);
	}

	for (auto& thread : threads)
	{
		thread.join();
	}

#else

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{

			glm::vec4 color = PerPixel(x, y);

			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

			glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
			accumulatedColor /= (float)m_FrameIndex;

			accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));

			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
		}
	}

#endif

	// upload the image data to the gpu
	m_FinalImage->SetData(m_ImageData);

	if (m_Settings.Accumulate)
	{
		m_FrameIndex++;
	}
	else
	{
		m_FrameIndex = 1;
	}
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	glm::vec3 light(0.0f);
	uint8_t numberOfSamples = m_Settings.AntiAliasing ? m_Settings.SamplesPerPixel : 1;
	float halfPixelSize = m_Settings.PixelSize / 2;
	// for fast "random" number generation
	uint32_t seed = x + y * m_FinalImage->GetWidth();
	// NOTE: only using += m_FrameIndex makes stripes (go away after waiting)
	seed += m_FrameIndex * m_FrameIndex * m_FrameIndex;

	for (uint8_t i = 0; i < numberOfSamples; i++)
	{

		Ray ray;
		ray.Origin = m_ActiveCamera->GetPosition();
		ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

		if (m_Settings.AntiAliasing)
		{
			// TODO: maybe faster or prettier code?!
			float randomOffsetX = Utils::RandomFloat(-halfPixelSize, halfPixelSize, seed);
			seed += 1;
			float randomOffsetY = Utils::RandomFloat(-halfPixelSize, halfPixelSize, seed);
			ray.Direction.r += randomOffsetX;
			ray.Direction.g += randomOffsetY;
		}

		glm::vec3 contribution(1.0f);

		for (int i = 0; i < m_Settings.Bounces; i++)
		{
			seed += 1;

			HitPayload payload;
			// old
			//TraceRay(ray, payload);
			// new 
			TraceRay(ray, payload, 0);
			if (payload.HitDistance < 0.0f)
			{
				// nothing got hit
				// -> give a blue gradient backround
				glm::vec3 unitDirection = glm::normalize(ray.Direction);
				float a = 0.5 * (unitDirection.y + 1.0);
				// right to contribution is blended value 
				light += contribution * ((1.0f - a) * glm::vec3(1.0f, 1.0f, 1.0f) + a * glm::vec3(0.5f, 0.7f, 1.0f));
				break;
			}

			// TODO: add diffuse lighting
			const Hittable& hittableObj = *(m_ActiveScene->HittableObjs[payload.ObjectIndex]);
			const Material& material = m_ActiveScene->Materials[hittableObj.MaterialIndex];

			// light += contribution * material.Reflect(ray, payload, seed);

			// contribution *= 0.5f;

			glm::vec3 color;
			bool continueTracing = material.Reflect(ray, payload, seed, color);
			contribution *= color;

			if (!continueTracing) {
				break;
			}


			// TODO: check this! in method above
			// check if "reflected" ray goes into sphere
			/*if (!dot(ray.Direction, payload.WorldNormal) > 0) {
				break;
			}*/

			// ray.Origin = payload.WorldPosition + 0.0001f * payload.WorldNormal;
			// lambertian reflection -> theoretically the term could cancel out, but will probably never happen...
			// ray.Direction = glm::normalize(payload.WorldNormal + Utils::InUnitSphere(seed));

			// light += 0.5f * (glm::vec3(payload.WorldNormal.x, payload.WorldNormal.y, payload.WorldNormal.z) + glm::vec3(1.0f));
		}
	}

	light = light / (float)numberOfSamples;

	return glm::vec4(light, 1.0f);

	// for (int i = 0; i < bounces; i++)
	// {
	//   seed += i;

	//   Renderer::HitPayload payload = TraceRay(ray);
	//   if (payload.HitDistance < 0.0f)
	//   {
	//     glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
	//     // the sky is an ambient light source -> that's why we can add light
	//     // light += skyColor * contribution;
	//     break;
	//   }

	//   // just the usual light
	//   // glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));
	//   // float lightIntensity = glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f); // == cos(angle)

	//   const Sphere &sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
	//   const Material &material = m_ActiveScene->Materials[sphere.MaterialIndex];

	//   // sphereColor *= lightIntensity;
	//   // light += material.Albedo * contribution;

	//   contribution *= material.Albedo;
	//   // we add emission of emissive material -> we are generating ligtht
	//   light += material.GetEmission() * material.Albedo;
	//   light += material.Albedo * 0.5f;

	//   ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
	//   // ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal + material.Roughness * Utils::RandomVec3(-0.5f, 0.5f, seed));
	//   ray.Direction = glm::normalize(payload.WorldNormal + Utils::InUnitSphere(seed));
	//   // ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));
	// }

	// return glm::vec4(light, 1.0f);
}

void Renderer::TraceRay(const Ray& ray, HitPayload& payload, const uint32_t nodeIndx) {
	

	payload.ObjectIndex = -1;
	payload.HitDistance = FLOAT_MAX;

	IntersectBVH(ray, payload, 0);


	if (payload.ObjectIndex < 0)
	{
		Miss(ray, payload);
		return;
	}

	ClosestHit(ray, payload, payload.ObjectIndex);

}


void Renderer::IntersectBVH(const Ray& ray, HitPayload& payload, const uint32_t nodeIndx) {
	const BVHNode& node = m_ActiveScene->BVHNodes.at(nodeIndx);

	int closestHittableObj = payload.ObjectIndex;
	float hitDistance = payload.HitDistance;

	if (node.m_BoundingBox.Hit(ray)) { // node got hit
		// node is Leaf
		if (node.m_PrimCount > 0) {
			for (uint32_t i = node.m_FirstPrimIndx; i < node.m_FirstPrimIndx + node.m_PrimCount ; i++)
			{
				const Hittable& hittableObj = *(m_ActiveScene->HittableObjs[i]);
				hittableObj.CheckHit(ray, payload);
				if (payload.HitDistance > 0.0f && payload.HitDistance < hitDistance)
				{
					hitDistance = payload.HitDistance;
					closestHittableObj = (int)i;
				}
			}
			if (closestHittableObj < 0)
			{
				return;
			}
			payload.HitDistance = hitDistance;
			payload.ObjectIndex = closestHittableObj;
		}
		else { // node is not leaf
			IntersectBVH(ray, payload, node.m_LeftChildIndx);
			IntersectBVH(ray, payload, node.m_RightChildIndx);
		}
	}
	else { // node did not get hit
		return;
	}

}

void Renderer::TraceRay(const Ray& ray, HitPayload& payload)
{
	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// where
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance


	int closestHittableObj = -1;
	float hitDistance = std::numeric_limits<float>::max();
	for (size_t i = 0; i < m_ActiveScene->HittableObjs.size(); i++)
	{
		const Hittable& hittableObj = *(m_ActiveScene->HittableObjs[i]);
		hittableObj.CheckHit(ray, payload);
		if (payload.HitDistance > 0.0f && payload.HitDistance < hitDistance)
		{
			hitDistance = payload.HitDistance;
			closestHittableObj = (int)i;
		}
	}

	if (closestHittableObj < 0)
	{
		Miss(ray, payload);
		return;
	}

	payload.HitDistance = hitDistance;
	ClosestHit(ray, payload, closestHittableObj);

	
}

void Renderer::ClosestHit(const Ray& ray, HitPayload& payload, int objectIndex)
{
	payload.ObjectIndex = objectIndex;

	const Hittable& closestHittableObj = *(m_ActiveScene->HittableObjs[objectIndex]);
	closestHittableObj.Hit(payload, ray);
}

void Renderer::Miss(const Ray& ray, HitPayload& payload)
{
	payload.HitDistance = -1.0f;
}
