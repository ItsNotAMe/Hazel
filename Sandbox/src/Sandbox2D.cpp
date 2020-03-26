#include "Sandbox2D.h"

#include <Platform/OpenGL/OpenGLShader.h>

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnAttach()
{
	HZ_PROFILE_FUNCTION();

	m_Texture = Hazel::Texture2D::Create("assets/textures/thinking_smol.png");
}

void Sandbox2D::OnDetach()
{
	HZ_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Hazel::Timestep ts)
{
	HZ_PROFILE_FUNCTION();

	// Update
	m_CameraController.OnUpdate(ts);

	// Render
	{
		HZ_PROFILE_SCOPE("Render Prep");
		Hazel::RenderCommand::SetClearColor({ 0.15f, 0.15f, 0.15f, 1.0f });
		Hazel::RenderCommand::Clear();
	}

	{
		HZ_PROFILE_SCOPE("Render Draw");
		Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());

		for (int y = 0; y < m_SquaresPerLine; y++) {
			for (int x = 0; x < m_SquaresPerLine; x++) {
				float sizeF = 0.2f;
				float padding = 0.01f;
				float num = sizeF + padding;
				glm::vec3 pos(x * num - num * (m_SquaresPerLine - 1) / 2.0f, y * num - num * (m_SquaresPerLine - 1) / 2.0f, 0.0f);
				glm::vec2 size(sizeF, sizeF);
				if (m_DrawSquares)
					Hazel::Renderer2D::DrawQuad(pos, size, glm::radians(m_Rotation), m_Color);
				if (m_DrawTextures)
					Hazel::Renderer2D::DrawQuad(pos, size, glm::radians(m_Rotation), m_Texture, m_ColorTint, m_TextureRepeatCount);
			}
		}

		// Middle of screen
		Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 0.01f, 0.01f }, glm::radians(45.0f), { 1.0f, 1.0f, 1.0f, 1.0f });

		Hazel::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	HZ_PROFILE_FUNCTION();

	ImGui::Begin("Settings 2D");
	if (m_DrawSquares)
		ImGui::ColorEdit4("Square color: ", glm::value_ptr(m_Color));
	if(m_DrawTextures)
		ImGui::ColorEdit4("Texture tint color: ", glm::value_ptr(m_ColorTint));
	ImGui::SliderFloat("Rotation: ", &m_Rotation, -180.0f, 180.0f);
	ImGui::SliderInt("Squares per line: ", &m_SquaresPerLine, 1, 100);
	if (m_DrawTextures)
		ImGui::SliderInt("Texture repeat count: ", &m_TextureRepeatCount, 1, 10);
	ImGui::Checkbox("Draw squares", &m_DrawSquares);
	ImGui::Checkbox("Draw textures", &m_DrawTextures);
	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& e)
{
	m_CameraController.OnEvent(e);
}
