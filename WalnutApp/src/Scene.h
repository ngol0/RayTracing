#pragma once
#include <vector>
#include <glm/glm.hpp>


enum class MaterialType
{
	DIFFUSE,
	MIRRORED,

};

struct Material
{
	glm::vec3 albedo{ 1.f };
	float roughness = 1.f;
	float metallic = 0.f;

	glm::vec3 emissionColor;
	float emissionRate;

	bool isMirrored;
};

struct Sphere
{
	float radius{ 0.5f };
	glm::vec3 center{ 0.f };

	int materialIndex;
};

class Scene
{
public:
	std::vector<Sphere> spheres;
	std::vector<Material> materials;

	Scene() {}
};