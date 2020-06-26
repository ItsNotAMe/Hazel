#pragma once

#include <Hazel.h>

namespace Hazel {

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;
	private:
		OrthographicCameraController m_CameraController;

		Ref<Framebuffer> m_Framebuffer;

		Ref<Texture2D> m_HmmTexture;
		Ref<Texture2D> m_SpriteSheet;
		Ref<SubTexture2D> m_RoofTexture, m_EntranceTexture;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize;

		glm::vec3 m_StartColor = { 0.2f, 0.3f, 0.8f };
		glm::vec3 m_EndColor = { 0.2f, 0.3f, 0.8f };
		glm::vec4 m_ColorTint = { 1.0f,  1.0f,  1.0f, 1.0f };
		float m_PositionZ = 0.7f;
		float m_Alpha = 0.7f;
		float m_Rotation = 0.0f;
		int m_SquaresPerLine = 3;
		float m_Margin = 0.01f;
		int m_TextureRepeatCount = 1;
		bool m_DrawSquares = true, m_DrawTextures = false;
	};

}