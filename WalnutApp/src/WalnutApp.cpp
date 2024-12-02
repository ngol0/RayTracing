#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer() : m_camera(45.0f, 0.1f, 100.0f) 
	{
		{
			Sphere sphere;
			sphere.center = { 0.0f, 0.0f, 0.0f };
			sphere.radius = 0.5f;
			sphere.albedo = { 1.0f, 0.0f, 1.0f };
			m_scene.spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.center = { 1.0f, 0.0f, -5.0f };
			sphere.radius = 1.5f;
			sphere.albedo = { 0.2f, 0.3f, 1.0f };
			m_scene.spheres.push_back(sphere);
		}
	}

	virtual void OnUpdate(float ts) override
	{
		m_camera.OnUpdate(ts);
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		if (ImGui::Button("Render"))
		{
			Render();
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