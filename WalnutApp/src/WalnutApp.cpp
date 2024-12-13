#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer() : m_camera(45.0f, 0.1f, 100.0f) 
	{
		// add materials
		Material& blueSphere = m_scene.materials.emplace_back();
		blueSphere.albedo = { 1.0f, 0.0f, 1.0f };
		blueSphere.metallic = 0.f;
		blueSphere.roughness = 1.f;
		blueSphere.emissionRate = 0.f;

		Material& pinkSphere = m_scene.materials.emplace_back();
		pinkSphere.albedo = { 0.2f, 0.3f, 1.0f };
		pinkSphere.metallic = 0.f;
		pinkSphere.roughness = 1.f;
		pinkSphere.emissionRate = 0.f;

		Material& mirroredSphere = m_scene.materials.emplace_back();
		mirroredSphere.albedo = { 0.8, 0.8, 0.8 };
		mirroredSphere.metallic = 0.f;
		mirroredSphere.roughness = 1.f;
		mirroredSphere.emissionColor = { 0.8f, 0.7f, 0.f };
		mirroredSphere.emissionRate = 0.f;

		Material& lightShpere = m_scene.materials.emplace_back();
		lightShpere.albedo = { 0.5f, 0.5f, 0.f };
		lightShpere.metallic = 0.f;
		lightShpere.roughness = 1.f;
		lightShpere.emissionColor = { 0.8f, 0.7f, 0.f };
		lightShpere.emissionRate = 2.f;

		// add spheres
		{
			Sphere sphere;
			sphere.center = { 0.0f, 0.0f, 0.0f };
			sphere.radius = 1.f;
			sphere.materialIndex = 0;
			
			m_scene.spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.center = { 0.0f, -101.0f, 0.0f };
			sphere.radius = 100.f;
			sphere.materialIndex = 1;

			m_scene.spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.center = { -2.0f, 0.0f, 0.0f };
			sphere.radius = 1.f;
			sphere.materialIndex = 2;

			m_scene.spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.center = { 2.0f, 0.0f, 0.0f };
			sphere.radius = 1.f;
			sphere.materialIndex = 2;

			m_scene.spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.center = { 2.0f, 0.0f, -2.0f };
			sphere.radius = 1.f;
			sphere.materialIndex = 3;

			m_scene.spheres.push_back(sphere);
		}
	}

	virtual void OnUpdate(float ts) override
	{
		if (m_camera.OnUpdate(ts))
			m_renderer.ResetFrameIndex();
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		if (ImGui::Button("Render"))
		{
			Render();
		}
		ImGui::Checkbox("Accumulated", &m_renderer.GetSetting().isAccummulated);
		ImGui::Checkbox("Has Emissive Source", &m_renderer.GetSetting().hasEmissiveSource);
		ImGui::Checkbox("Mirrored Sphere", &m_scene.materials[2].isMirrored);
		if (ImGui::Button("Reset Frame Index"))
		{
			m_renderer.ResetFrameIndex();
		}

		ImGui::End();

		ImGui::Begin("Scene");

		// for sphere
		ImGui::Text("Sphere Settings");
		for (size_t i = 0; i < m_scene.spheres.size(); i++)
		{
			ImGui::PushID(i);

			Sphere& sphere = m_scene.spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.center), 0.1f);
			ImGui::DragFloat("Radius", &sphere.radius, 0.1f, 0.f, FLT_MAX);
			ImGui::DragInt("Material Incex", &sphere.materialIndex, 1.f, 0, (int)m_scene.materials.size() - 1);

			ImGui::Separator();

			ImGui::PopID();
		}

		// for material
		ImGui::Text("Material Settings");
		for (size_t i = 0; i < m_scene.materials.size(); i++)
		{
			ImGui::PushID(i);

			Material& mat = m_scene.materials[i];
			ImGui::DragFloat("Roughness", &mat.roughness, 0.05f, 0.f, 1.f);
			ImGui::DragFloat("Metallic", &mat.metallic, 0.05f, 0.f, 1.f);
			ImGui::ColorEdit3("Albedo", glm::value_ptr(mat.albedo));

			ImGui::ColorEdit3("Emissive Color", glm::value_ptr(mat.emissionColor));
			ImGui::DragFloat("Emissive Rate", &mat.emissionRate, 0.05f, 0.f, FLT_MAX);

			ImGui::Separator();

			ImGui::PopID();
		}

		ImGui::End();


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.f, 0.f });
		ImGui::Begin("Viewport");

		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		auto image = m_renderer.GetFinalImage();

		// display image
		if (image)
		{
			ImGui::Image(image->GetDescriptorSet(), {(float)image->GetWidth(), (float)image->GetHeight()}, ImVec2{0,1}, ImVec2{1,0});
		}

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render()
	{
		Timer timer;

		m_renderer.OnSizeChanged(m_ViewportWidth, m_ViewportHeight);
		m_camera.OnResize(m_ViewportWidth, m_ViewportHeight);

		m_renderer.Render(m_camera, m_scene);

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	uint32_t m_ViewportWidth = 0; uint32_t m_ViewportHeight = 0;
	Renderer m_renderer;
	Camera m_camera;
	Scene m_scene;
	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}