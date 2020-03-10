#include <Hazel.h>

#include <Platform/OpenGL/OpenGLShader.h>

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-3.2f, 3.2f, -1.8f, 1.8f)
	{
		m_TextureVA = Hazel::VertexArray::Create();

		float vertices[4 * 5] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		Hazel::Ref<Hazel::VertexBuffer> vertexBuffer = Hazel::VertexBuffer::Create(vertices, sizeof(vertices));

		Hazel::BufferLayout layout = {
			{ Hazel::ShaderDataType::Float3, "a_Position" },
			{ Hazel::ShaderDataType::Float2, "a_TextCoord" }
		};
		vertexBuffer->SetLayout(layout);

		m_TextureVA->AddVertexBuffer(vertexBuffer);

		unsigned int indices[6] = {
			0, 1, 2,
			2, 3, 0
		};

		Hazel::Ref<Hazel::IndexBuffer> indexBuffer = Hazel::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));

		m_TextureVA->SetIndexBuffer(indexBuffer);


		m_SquareVA = Hazel::VertexArray::Create();

		float squareVertices[4 * 3] = {
			-1.0f, -1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f
		};

		Hazel::Ref<Hazel::VertexBuffer> squareVB = Hazel::VertexBuffer::Create(squareVertices, sizeof(squareVertices));
		squareVB->SetLayout({
			{ Hazel::ShaderDataType::Float3, "a_Position" }
			});

		m_SquareVA->AddVertexBuffer(squareVB);

		unsigned int squareIndices[6] = {
			0, 1, 2,
			2, 3, 0
		};

		Hazel::Ref<Hazel::IndexBuffer> squareIB = Hazel::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));

		m_SquareVA->SetIndexBuffer(squareIB);


		m_TextureShader = Hazel::Shader::Create("assets/shaders/Texture.glsl");

		m_FlatColorShader = Hazel::Shader::Create("assets/shaders/FlatColor.glsl");

		m_Texture = Hazel::Texture2D::Create("assets/textures/thinking_smol.png");

		m_TextureShader->Bind();
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_TextureShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnUpdate(Hazel::Timestep ts) override
	{
		Hazel::RenderCommand::SetClearColor({ 0.15f, 0.15f, 0.15f, 1.0f });
		Hazel::RenderCommand::Clear();

		float time = ts;
		if (Hazel::Input::IsKeyPressed(HZ_KEY_W))
			y += 4 * time;
		if (Hazel::Input::IsKeyPressed(HZ_KEY_S))
			y -= 4 * time;
		if (Hazel::Input::IsKeyPressed(HZ_KEY_A))
			x -= 4 * time;
		if (Hazel::Input::IsKeyPressed(HZ_KEY_D))
			x += 4 * time;
		if (Hazel::Input::IsKeyPressed(HZ_KEY_Q))
			rotation += 90 * time;
		if (Hazel::Input::IsKeyPressed(HZ_KEY_E))
			rotation -= 90 * time;
		m_Camera.SetPosition({ x, y, 0.0f });
		m_Camera.SetRotation(rotation);

		Hazel::Renderer::BeginScene(m_Camera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / 3));
		glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 0, 1));

		m_FlatColorShader->Bind();
		for (int y = 0; y < 3; y++) {
			for (int x = 0; x < 3; x++) {
				glm::vec3 pos(x * 0.68f - 0.68f - 2 / 3, y * 0.68f - 0.68f - 2 / 3, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * rotate * scale;
				glm::vec3 color(0.0f);
				if ((x + y) % 2 == 0)
					color = m_Color1;
				else
					color = m_Color2;
				std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", color);
				Hazel::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
			}
		}

		m_Texture->Bind();
		rotate = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 0, 1));
		Hazel::Renderer::Submit(m_TextureShader, m_TextureVA, glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f)) * rotate);

		Hazel::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Color Picker");
		ImGui::ColorEdit3("Color 1: ", glm::value_ptr(m_Color1));
		ImGui::ColorEdit3("Color 2: ", glm::value_ptr(m_Color2));
		ImGui::End();
	}

	void OnEvent(Hazel::Event& event) override
	{
		//Hazel::EventDispatcher dispatcher(event);
		//dispatcher.Dispatch<Hazel::KeyReleasedEvent>(HZ_BIND_EVENT_FN(ExampleLayer::OnKeyReleasedEvent));
	}

private:
	Hazel::Ref<Hazel::Shader> m_TextureShader;
	Hazel::Ref<Hazel::VertexArray> m_TextureVA;

	Hazel::Ref<Hazel::Shader> m_FlatColorShader;
	Hazel::Ref<Hazel::VertexArray> m_SquareVA;

	Hazel::Ref<Hazel::Texture2D> m_Texture;

	glm::vec3 m_Color1 = { 0.2f, 0.3f, 0.8f };
	glm::vec3 m_Color2 = { 0.8f, 0.1f, 0.2f };

	Hazel::OrthographicCamera m_Camera;

	float rotation = 0.0f;
	float x = 0.0f, y = 0.0f;
};

class Sandbox : public Hazel::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{

	}

};

Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}