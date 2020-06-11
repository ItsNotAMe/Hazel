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

	m_HmmTexture = Hazel::Texture2D::Create("assets/textures/thinking_smol.png");
	m_SpriteSheet = Hazel::Texture2D::Create("assets/game/textures/RPGpack_sheet.png");
	m_RoofTexture = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0, 4 }, { 128, 128 }, { 2, 3 });
	m_EntranceTexture = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 7, 9 }, { 128, 128 }, { 2, 1 });
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
	Hazel::Renderer2D::ResetStats();
	{
		HZ_PROFILE_SCOPE("Render Prep");
		Hazel::RenderCommand::SetClearColor({ 0.15f, 0.15f, 0.15f, 1.0f });
		Hazel::RenderCommand::Clear();
	}

	{
		HZ_PROFILE_SCOPE("Render Draw");
		Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());

		Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 30.0f, 30.0f }, glm::radians(m_Rotation), m_HmmTexture, m_TextureRepeatCount, m_ColorTint);

		for (int y = 0; y < m_SquaresPerLine; y++) {
			for (int x = 0; x < m_SquaresPerLine; x++) {
				float sizeF = 0.2f;
				float num = sizeF + m_Margin;
				glm::vec2 pos(x * num - num * (m_SquaresPerLine - 1) / 2.0f, y * num - num * (m_SquaresPerLine - 1) / 2.0f);
				glm::vec2 size(sizeF, sizeF);
				glm::vec4 color;
				if (m_SquaresPerLine != 1)
					color = glm::vec4(m_StartColor.r + (m_EndColor.r - m_StartColor.r) * ((float)x / (m_SquaresPerLine - 1)),
						m_StartColor.g + (m_EndColor.g - m_StartColor.g) * ((float)(x + y) / (m_SquaresPerLine * 2 - 2)),
						m_StartColor.b + (m_EndColor.b - m_StartColor.b) * ((float)y / (m_SquaresPerLine - 1)),
						m_Alpha);
				else
					color = glm::vec4(m_StartColor.r, m_StartColor.g, m_StartColor.b, m_Alpha);
				if (m_DrawSquares)
					Hazel::Renderer2D::DrawQuad(glm::vec3{ pos, m_PositionZ }, size, glm::radians(m_Rotation), color);
				if (m_DrawTextures)
					Hazel::Renderer2D::DrawQuad(glm::vec3(pos, 0.5f), size, glm::radians(m_Rotation), m_HmmTexture, m_TextureRepeatCount, m_ColorTint);
			}
		}

		Hazel::Renderer2D::DrawQuad({ 0.0f, 1.99f, 0.6f }, { 2.0f, 3.0f }, glm::radians(m_Rotation), m_RoofTexture);
		Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.7f }, { 2.0f, 1.0f }, glm::radians(m_Rotation), m_EntranceTexture);

		// Middle of screen
		Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, 1.0f }, { 0.01f, 0.01f }, glm::radians(45.0f), { 1.0f, 1.0f, 1.0f, 1.0f });

		Hazel::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	HZ_PROFILE_FUNCTION();

	ImGui::Begin("Settings 2D");
	if (m_DrawSquares) 
	{
		ImGui::ColorEdit3("Square start color: ", glm::value_ptr(m_StartColor));
		ImGui::ColorEdit3("Square end color: ", glm::value_ptr(m_EndColor));
	}
	if(m_DrawTextures)
		ImGui::ColorEdit4("Texture tint color: ", glm::value_ptr(m_ColorTint));
	ImGui::DragFloat("z: ", &m_PositionZ, 0.05f, -1.0f, 1.0f);
	ImGui::DragFloat("Alpha: ", &m_Alpha, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Rotation: ", &m_Rotation, 1.0f, -180.0f, 180.0f);
	ImGui::DragInt("Squares per line: ", &m_SquaresPerLine, 1.0f, 1, 100);
	ImGui::DragFloat("Margin: ", &m_Margin, 0.01f, 0.0f, 2.0f);
	if (m_DrawTextures)
		ImGui::DragInt("Texture repeat count: ", &m_TextureRepeatCount, 1.0f, 1, 10);
	ImGui::Checkbox("Draw squares", &m_DrawSquares);
	ImGui::Checkbox("Draw textures", &m_DrawTextures);

	auto stats = Hazel::Renderer2D::GetStats();
	ImGui::Text("Renderer2D stats:");
	ImGui::Text("Draw calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

	uint32_t textureId = m_HmmTexture->GetRendererID();
	ImGui::Image((void*)textureId, ImVec2{ 128, 128 });

	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& e)
{
	m_CameraController.OnEvent(e);
}
