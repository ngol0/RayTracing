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

class Renderer
{
public:
	Renderer();

	void OnSizeChanged(uint32_t width, uint32_t height);
	void Render(const Camera& camera, const Scene& scene);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

private:
	glm::vec4 TraceRay(const Ray& ray, const Scene& sphere);

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr; //an array of data, holds actual image data for each pixel
};
