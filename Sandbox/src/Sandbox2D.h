#pragma once

#include <Hazel.h>

class Sandbox2D : public Hazel::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(Hazel::Timestep ts) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(Hazel::Event& e) override;
private:
	Hazel::OrthographicCameraController m_CameraController;

	Hazel::Ref<Hazel::Texture2D> m_Texture;

	glm::vec4 m_Color = { 0.2f, 0.3f, 0.8f, 1.0f };
	glm::vec4 m_ColorTint = { 1.0f,  1.0f,  1.0f, 1.0f };
	float m_Rotation = 0.0f;
	int m_SquaresPerLine = 3;
	int m_TextureRepeatCount = 1;
	bool m_DrawSquares = true, m_DrawTextures = false;
};