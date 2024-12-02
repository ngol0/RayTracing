#include "Renderer.h"
#include <iostream>

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
	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render(const Camera& camera, const Scene& scene)
{
	Ray ray;
	ray.Origin = camera.GetPosition();

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); ++y)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); ++x)
		{
			ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];
			glm::vec4 color = TraceRay(ray, scene);
			color = glm::clamp(color, glm::vec4{ 0.f }, glm::vec4{ 1.f });
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}

	//upload image to gpu
	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::TraceRay(const Ray& ray, const Scene& scene)
{
	glm::vec3 sphereColor = glm::vec3{ 1.f, 0.f, 0.f };
	const Sphere* closerSphere = nullptr;
	float hitDistance = std::numeric_limits<float>::max();

	for (const Sphere& sphere : scene.spheres)
	{
		glm::vec3 oc = sphere.center - ray.Origin;
		float a = glm::dot(ray.Direction, ray.Direction);
		float b = -2.f * glm::dot(oc, ray.Direction);
		float c = glm::dot(oc, oc) - sphere.radius * sphere.radius;

		// quadratic formula
		// b^2 - 4ac
		float discriminant = b * b - 4.f * a * c;

		if (discriminant < 0) continue;

		//float t0 = (-b + glm::sqrt(discriminant)) / 2 * a;
		float t1 = (-b - glm::sqrt(discriminant)) / 2 * a; // smaller value >> closer point

		if (t1 < hitDistance)
		{
			hitDistance = t1;
			closerSphere = &sphere;
		}
	}

	// hit positions
	glm::vec3 hitpoint = ray.Origin + ray.Direction * hitDistance;

	//some basic lighting:
	if (closerSphere == nullptr) return glm::vec4{ 1.f };
	glm::vec3 normal = glm::normalize(hitpoint - closerSphere->center);
	glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));
	float lightIntensity = glm::max(glm::dot(normal, -lightDir), 0.0f); // == cos(angle)

	sphereColor = closerSphere->albedo * lightIntensity;

	return glm::vec4{ sphereColor, 1.f };
}

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


