#include "Renderer.h"
#include <iostream>
#include <Walnut/Random.h>

Renderer::Renderer()
{
}

void Renderer::OnSizeChanged(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// No resize necessary
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;
		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	delete[] m_imageData;
	m_imageData = new uint32_t[width * height];

	delete[] m_accumulatedColor;
	m_accumulatedColor = new glm::vec4[width * height];

	ResetFrameIndex();
}

void Renderer::Render(const Camera& camera, const Scene& scene)
{
	m_scene = &scene;
	m_camera = &camera;

	if (m_frameIndex == 1)
	{
		memset(m_accumulatedColor, 0, m_FinalImage->GetHeight() * m_FinalImage->GetWidth() * sizeof(glm::vec4));
	}

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); ++y)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); ++x)
		{
			glm::vec4 color = PerPixel(x, y);

			// accumulate color for that pixel
			m_accumulatedColor[x + y * m_FinalImage->GetWidth()] += color;
			glm::vec4 accumulatedColor = m_accumulatedColor[x + y * m_FinalImage->GetWidth()];
			accumulatedColor /= m_frameIndex;

			accumulatedColor = glm::clamp(accumulatedColor, glm::vec4{ 0.f }, glm::vec4{ 1.f });
			m_imageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
		}
	}

	//upload image to gpu
	m_FinalImage->SetData(m_imageData);

	if (m_setting.isAccummulated)
		m_frameIndex++;
	else
		m_frameIndex = 1;
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.origin = m_camera->GetPosition();
	ray.direction = m_camera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	int bounces = 3;
	glm::vec3 light = TraceRayRecursive(ray, bounces); // 8 bounces as max depth
	return glm::vec4(light, 1.0f);
}

glm::vec3 Renderer::TraceRayRecursive(const Ray& ray, int remainingBounces)
{
	glm::vec3 light{ 0.f };
	float multiplier = 1.f;

	if (remainingBounces <= 0)
		return glm::vec3(0.0f); //black 

	HitPayload payload = TraceRay(ray);

	if (payload.hitDistance < 0.f)
	{
		if (!m_setting.hasEmissiveSource)
		{
			glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
			return skyColor;
		}
		return glm::vec3{ 0.f };
	}

	const Sphere& sphere = m_scene->spheres[payload.objectIndex];
	const Material& material = m_scene->materials[sphere.materialIndex];

	if (!material.isMirrored)
	{
		light += material.emissionColor * material.emissionRate; // color of current ray, if not emissive - light = 0;

		// next ray
		Ray nextRay;
		nextRay.origin = payload.worldPos + payload.normal * 0.0001f;
		nextRay.direction = glm::normalize(payload.normal + Walnut::Random::InUnitSphere()); // lambertian reflection

		light += material.albedo * (TraceRayRecursive(nextRay, remainingBounces - 1)); // current albedo * color of next ray
	}

	else
	{
		//calculate light color here
		multiplier *= 0.9f;
		Ray nextRay;
		nextRay.direction = glm::reflect(ray.direction, payload.normal); // mirrored reflection
		nextRay.origin = payload.worldPos + payload.normal * 0.0001f;

		light += material.albedo * (TraceRayRecursive(nextRay, remainingBounces - 1)) * multiplier;
	}
	return light;
}

//glm::vec3 Renderer::TraceRayRecursive(const Ray& ray, int remainingBounces)
//{
//	float multiplier = 1.f;
//	if (remainingBounces <= 0)
//		return glm::vec3(0.0f); 
//
//	HitPayload payload = TraceRay(ray);
//
//	if (payload.hitDistance < 0.f)
//	{
//		if (!m_setting.hasEmissiveSource)
//			return glm::vec3(1.f, 1.f, 0.9f) * multiplier; // Sky color
//
//		return glm::vec3(0.0f);
//	}
//
//	const Sphere& sphere = m_scene->spheres[payload.objectIndex];
//	const Material& material = m_scene->materials[sphere.materialIndex];
//
//	glm::vec3 light(0.0f);
//
//	if (material.type == MaterialType::DIFFUSE)
//	{
//		// Accumulate emissive light and adjust contribution
//		light += material.emissionColor * material.emissionRate;
//
//		// Spawn a new ray for diffuse reflection
//		Ray newRay;
//		newRay.origin = payload.worldPos + payload.normal * 0.0001f;
//		newRay.direction = glm::normalize(payload.normal + Walnut::Random::InUnitSphere());
//
//		light += TraceRayRecursive(newRay, remainingBounces - 1) * material.albedo;
//	}
//	else if (material.type == MaterialType::MIRRORED)
//	{
//		multiplier *= 0.9f;
//		// Spawn a new ray for specular reflection
//		Ray newRay;
//		newRay.origin = payload.worldPos + payload.normal * 0.0001f;
//		newRay.direction = glm::reflect(ray.direction, payload.normal);
//
//		light += TraceRayRecursive(newRay, remainingBounces - 1) * material.albedo * multiplier;
//	}
//
//	return light;
//}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{
	int closerSphere = -1;
	float hitDistance = std::numeric_limits<float>::max();

	for (size_t i = 0; i < m_scene->spheres.size(); ++i)
	{
		const Sphere& sphere = m_scene->spheres[i];

		glm::vec3 oc = sphere.center - ray.origin;
		float a = glm::dot(ray.direction, ray.direction);
		float b = -2.f * glm::dot(oc, ray.direction);
		float c = glm::dot(oc, oc) - sphere.radius * sphere.radius;

		// quadratic formula
		// b^2 - 4ac
		float discriminant = b * b - 4.f * a * c;

		if (discriminant < 0) continue;

		//float t0 = (-b + glm::sqrt(discriminant)) / 2 * a;
		float t1 = (-b - glm::sqrt(discriminant)) / 2 * a; // smaller value >> closer point

		if (t1 > 0.f && t1 < hitDistance) // TODO: why does t1 have to be > 0???
		{
			hitDistance = t1;
			closerSphere = (int)i;
		}
	}

	if (closerSphere < 0) 
		return Miss();

	return ClosestHit(ray, hitDistance, closerSphere);
}

Renderer::HitPayload Renderer::Miss()
{
	HitPayload payload;
	payload.hitDistance = -1.f;
	return payload;
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	HitPayload payload;
	payload.hitDistance = hitDistance;
	payload.objectIndex = objectIndex;

	const Sphere& sphere = m_scene->spheres[payload.objectIndex];

	// hit positions
	payload.worldPos = ray.origin + ray.direction * hitDistance;
	payload.normal = glm::normalize(payload.worldPos - sphere.center);

	return payload;
}

//glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
//{
//	Ray ray;
//	ray.origin = m_camera->GetPosition();
//	ray.direction = m_camera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];
//
//	glm::vec3 light{ 0.f };
//	glm::vec3 contribution{ 1.f }; // the amount of absorption?
//	float multiplier = 1.f;
//
//	// trace ray twice: one original and one bounce off from the destination of the prev one
//	int bounce = 2;
//	for (int i = 0; i < bounce; ++i)
//	{
//		HitPayload payload = TraceRay(ray);
//
//		if (payload.hitDistance < 0.f)
//		{
//			if (!m_setting.hasEmissiveSource)
//			{
//				glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
//				light += skyColor * contribution;
//				//light += skyColor * multiplier;
//			}
//			break;
//		}
//
//		const Sphere& sphere = m_scene->spheres[payload.objectIndex];
//		const Material& material = m_scene->materials[sphere.materialIndex];
//
//		//glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));
//		//float lightIntensity = glm::max(glm::dot(payload.normal, -lightDir), 0.0f); // == cos(angle)
//		//glm::vec3 sphereColor = material.albedo;
//		//sphereColor *= lightIntensity;
//
//		if (material.type == MaterialType::DIFFUSE)
//		{
//			//light += material.albedo * multiplier;
//			contribution *= material.albedo;
//
//			if (m_setting.hasEmissiveSource)
//			{
//				light += material.emissionColor * material.emissionRate * contribution;
//			}
//
//			ray.direction = glm::normalize(payload.normal + Walnut::Random::InUnitSphere()); // lambertian reflection
//		}
//
//		else
//		{
//			//calculate light color here
//
//			ray.direction = glm::reflect(ray.direction, payload.normal); // mirrored reflection
//		}
//
//		ray.origin = payload.worldPos + payload.normal * 0.0001f;
//	}
//
//	return glm::vec4{ light, 1.f };
//}

//glm::vec4 Renderer::TraceRay(const Ray& ray, float radius, glm::vec3 sphereCenter)
//{
//	// calculate normal coord (0 > 1)
//	/*glm::vec2 uv = glm::vec2{ 
//		(float)pixelCoor.x / (float)m_FinalImage->GetWidth(), 
//		(float)pixelCoor.y / (float)m_FinalImage->GetHeight() };*/
//
//	// change to -1 > 1
//	//uv = uv * 2.f - 1.f; // TODO: FIGURE OUT THE MAPPING LOGIC???
//
//	// calculate for raytracing
//	// A = ray origin
//	// d = ray direction = direction from ray origin to each pixel == pixel pos vector - camera pos vector (z is -1)
//	// C = sphere center
//	// t = hit distance
//	// Equation: t^2*d.d−2t*d.(C−Q)+(C−Q).(C−Q)−r^2=0
//
//	/*glm::vec3 rayOrigin = glm::vec3{ 0.f, 0.f, 1.f };
//	glm::vec3 rayDirection = glm::normalize(glm::vec3(uv.x - rayOrigin.x, uv.y - rayOrigin.y, -1.0f));*/
//
//	// each element in the equation
//	glm::vec3 oc = sphereCenter - ray.Origin;
//	float a = glm::dot(ray.Direction, ray.Direction);
//	float b = -2.f * glm::dot(oc, ray.Direction);
//	float c = glm::dot(oc, oc) - radius * radius;
//
//	// quadratic formula
//	// b^2 - 4ac
//	float discriminant = b * b - 4.f * a * c;
//
//	if (discriminant < 0) return glm::vec4{ 0.f, 0.f, 0.f, 1.f };
//
//	//float t0 = (-b + glm::sqrt(discriminant)) / 2 * a;
//	float t1 = (-b - glm::sqrt(discriminant)) / 2 * a; // smaller value >> closer point
//
//	// hit positions
//	//glm::vec3 h0 = rayOrigin + rayDirection * t0;
//	glm::vec3 hitpoint = ray.Origin + ray.Direction * t1;
//
//	//some basic lighting:
//	glm::vec3 normal = glm::normalize(hitpoint);
//	glm::vec3 sunPos = glm::vec3(1.f, 1.f, 1.f);
//	glm::vec3 directionFromHitpointToSun = glm::normalize(sunPos - hitpoint);
//
//	// calculate the sign of angle between normal and sun
//	float cos = glm::dot(directionFromHitpointToSun, normal);
//
//	glm::vec3 sphereColor = glm::vec3{ 1.f, 0.f, 0.f };
//	//sphereColor *= cos;
//	//sphereColor = normal * 0.5f + 0.5f; // this to visualize the normal calculation
//
//	return glm::vec4{ sphereColor, 1.f };
//}


