#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Sphere
{
	float radius{ 0.5f };
	glm::vec3 center{ 0.f };
	glm::vec3 albedo{ 1.f };

	Sphere() {}
	Sphere(float r, glm::vec3 c, glm::vec3 color) : radius(r), center(c), albedo(color) {}
};

class Scene
{
public:
	std::vector<Sphere> spheres;

	Scene()
	{
		
	}
};