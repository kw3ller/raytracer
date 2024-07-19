#include "Camera.h"
#include "Material.h"
#include "Renderer.h"
#include "Scene.h"
#include "Sphere.h"
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"
#include "imgui.h"

#include "glm/gtc/type_ptr.hpp"
#include <cstddef>

class ExampleLayer : public Walnut::Layer
{


private:
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;
	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;

	float m_LastRenderTime = 0.0f;

public:
	ExampleLayer()
		: m_Camera(45.0f, 0.1f, 100.0f)
	{
		Material& materialMiddle = m_Scene.Materials.emplace_back();
		materialMiddle.Albedo = { 0.1f, 0.2f, 0.3f };
		materialMiddle.Roughness = 0.0f;
		Material& materialLeft = m_Scene.Materials.emplace_back();
		materialLeft.Albedo = { 0.7f, 0.7f, 0.7f };
		materialLeft.Roughness = 0.2f;
		materialLeft.m_MaterialType = MatrialType::Metall;
		Material& materialRight = m_Scene.Materials.emplace_back();
		materialRight.Albedo = { 0.85f, 0.65f, 0.125f };
		materialRight.Roughness = 0.1f;
		materialRight.m_MaterialType = MatrialType::Dialectric;
		materialRight.RefractionIndex = 1.00f / 1.33f; // usually 1.5f
		Material& materialRightRight = m_Scene.Materials.emplace_back();
		materialRightRight.Albedo = { 0.85f, 0.65f, 0.125f };
		materialRightRight.Roughness = 0.1f;
		materialRightRight.m_MaterialType = MatrialType::Metall;
		Material& materialUnder = m_Scene.Materials.emplace_back();
		materialUnder.Albedo = { 0.56f, 0.62f, 1.0f };
		materialUnder.Roughness = 0.1f;
		materialUnder.m_MaterialType = MatrialType::Lambertian;

		// NOTE: why not pretty much always take emplace_back? -> would avoid extra copying, right?
		// middle sphere
		{
			Sphere* sphere = new Sphere;
			sphere->Position = { 0.0f, 0.0f, 0.0f };
			sphere->Radius = 1.0f;
			sphere->MaterialIndex = 0;
			m_Scene.HittableObjs.emplace_back(sphere);
		}
		// left sphere
		{
			Sphere* sphere = new Sphere;
			sphere->Position = { -2.0f, 0.0f, 0.0f };
			sphere->Radius = 1.0f;
			sphere->MaterialIndex = 1;
			m_Scene.HittableObjs.emplace_back(sphere);
		}
		// right sphere
		{
			Sphere* sphere = new Sphere;
			sphere->Position = { 2.0f, 0.0f, 0.0f };
			sphere->Radius = 1.0f;
			sphere->MaterialIndex = 2;
			m_Scene.HittableObjs.emplace_back(sphere);
		}
		// right right sphere
		{
			Sphere* sphere = new Sphere;
			sphere->Position = { 6.0f, 0.0f, 0.0f };
			sphere->Radius = 1.0f;
			sphere->MaterialIndex = 3;
			m_Scene.HittableObjs.emplace_back(sphere);
		}
		// big sphere
		{
			Sphere* sphere = new Sphere;
			sphere->Position = { 0.0f, -101.0f, 0.0f };
			sphere->Radius = 100.0f;
			sphere->MaterialIndex = 4;
			m_Scene.HittableObjs.emplace_back(sphere);
		}

		// create the BVH
		m_Scene.BuildBVH();
		std::cout << "number of BVH nodes:";
		std::cout << m_Scene.BVHNodes.size();

		// print all biuunding volumes 
		for (int i = 0; i < m_Scene.BVHNodes.size(); i++) {
			std::cout << i << ":" << std::endl;
			std::cout << "xMin" << " : " << m_Scene.BVHNodes[i].m_BoundingBox.m_X.m_Min << std::endl;
			std::cout << "xMax" << " : " << m_Scene.BVHNodes[i].m_BoundingBox.m_X.m_Max << std::endl;

			std::cout << "yMin" << " : " << m_Scene.BVHNodes[i].m_BoundingBox.m_Y.m_Min << std::endl;
			std::cout << "yMax" << " : " << m_Scene.BVHNodes[i].m_BoundingBox.m_Y.m_Max << std::endl;

			std::cout << "zMin" << " : " << m_Scene.BVHNodes[i].m_BoundingBox.m_Z.m_Min << std::endl;
			std::cout << "zMax" << " : " << m_Scene.BVHNodes[i].m_BoundingBox.m_Z.m_Max << std::endl;
		}


	}

	virtual void OnUpdate(float ts) override
	{
		// means the camera has updated (moved)
		if (m_Camera.OnUpdate(ts))
		{
			m_Renderer.ResetFrameIndex();
		}
	}

	virtual void OnUIRender() override
	{

		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		if (ImGui::Button("Render"))
		{
			Render();
		}

		ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);

		ImGui::Checkbox("AntiAliasing", &m_Renderer.GetSettings().AntiAliasing);
		if (ImGui::Button("Reset"))
		{
			m_Renderer.ResetFrameIndex();
		}

		ImGui::Begin("Scene");
		for (size_t i = 0; i < m_Scene.HittableObjs.size(); i++)
		{
			// imgui uses names as ID -> because we have same names (e.g. Position) we need to differentiate between them
			ImGui::PushID(i);

			Hittable &hittable = *m_Scene.HittableObjs[i];
			ImGui::DragFloat3("Position", glm::value_ptr(hittable.Position), 0.1f);

			// add radius slider for spheres only
			Sphere* sphere = dynamic_cast<Sphere*> (&hittable);
			if (sphere) {
				ImGui::DragFloat("Radius", &(sphere->Radius), 0.1f, 0.0f);
			}
			
			ImGui::DragInt("Material Index", &hittable.MaterialIndex, 1.0f, 0, (int)m_Scene.Materials.size() - 1);

			ImGui::Separator();

			ImGui::PopID();
		}
		for (size_t i = 0; i < m_Scene.Materials.size(); i++)
		{

			ImGui::PushID(i);

			Material& material = m_Scene.Materials[i];
			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo));
			ImGui::DragFloat("Roughness", &material.Roughness, 0.05f, 0.0f, 1.0f);
			// ImGui::DragFloat("Metallic", &material.Metallic, 0.05f, 0.0f, 1.0f);
			ImGui::DragFloat("Emission Color", glm::value_ptr(material.EmissionColor));
			ImGui::DragFloat("Emission power", &material.EmissionPower, 0.05f, 0.0f, std::numeric_limits<float>::max());

			ImGui::Separator();

			ImGui::PopID();
		}
		ImGui::End();

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		std::shared_ptr<Walnut::Image> image = m_Renderer.GetFinalImage();

		if (image)
		{
			ImGui::Image(image->GetDescriptorSet(),
				{ (float)image->GetWidth(), (float)image->GetHeight() }, ImVec2(0, 1), ImVec2(1, 0));
		}

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render()
	{

		Walnut::Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render(m_Scene, m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}


};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
		{
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Exit")) {
					app->Close();
				}
				ImGui::EndMenu();
			} });
	return app;
}
