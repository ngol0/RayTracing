#pragma once

#include "Walnut/Image.h"
#include <memory>
#include <glm/glm.hpp>

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

namespace Utils
{
	static int32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = uint8_t(color.r * 255.f);
		uint8_t g = uint8_t(color.g * 255.f);
		uint8_t b = uint8_t(color.b * 255.f);
		uint8_t a = uint8_t(color.a * 255.f);

		return (a << 24) | (b << 16) | (g << 8) | r;
	}
}

struct Setting
{
	bool isAccummulated = false;
	bool hasEmissiveSource = false;
	bool isMirrored = false;
};

class Renderer
{
private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_imageData = nullptr; //an array of data, holds actual image data for each pixel
	glm::vec4* m_accumulatedColor = nullptr; //an array of color data, 4 bytes per channel == 16 bytes in total

	int m_frameIndex = 1;

	const Scene* m_scene = nullptr;
	const Camera* m_camera = nullptr;
	Setting m_setting;

	struct HitPayload
	{
		float hitDistance;
		glm::vec3 worldPos;
		glm::vec3 normal;
		int objectIndex;
	};

public:
	Renderer();

	void OnSizeChanged(uint32_t width, uint32_t height);
	void Render(const Camera& camera, const Scene& scene);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

	void ResetFrameIndex() { m_frameIndex = 1; }
	Setting& GetSetting() { return m_setting; }

private:
	glm::vec4 PerPixel(uint32_t x, uint32_t y);
	glm::vec3 TraceRayRecursive(const Ray& ray, int remainingBounce);

	HitPayload TraceRay(const Ray& ray);
	HitPayload Miss();
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
};
