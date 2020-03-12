#include <Hazel.h>

#include <Platform/OpenGL/OpenGLShader.h>

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_CameraController(1280.0f / 720.0f, true)
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


		auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");
		//m_TextureShader = Hazel::Shader::Create("assets/shaders/Texture.glsl");

		m_ShaderLibrary.Load("Color", "assets/shaders/FlatColor.glsl");
		//m_FlatColorShader = Hazel::Shader::Create("assets/shaders/FlatColor.glsl");

		m_ThinkTexture = Hazel::Texture2D::Create("assets/textures/thinking_smol.png");
		m_DuckTexture = Hazel::Texture2D::Create("assets/textures/cat.png");

		textureShader->Bind();
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnUpdate(Hazel::Timestep ts) override
	{
		// Update
		m_CameraController.OnUpdate(ts);

		// Render
		Hazel::RenderCommand::SetClearColor({ 0.15f, 0.15f, 0.15f, 1.0f });
		Hazel::RenderCommand::Clear();

		Hazel::Renderer::BeginScene(m_CameraController.GetCamera());

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / 3));
		glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 0, 1));

		auto colorShader = m_ShaderLibrary.Get("Color");
		colorShader->Bind();
		for (int y = 0; y < 3; y++) {
			for (int x = 0; x < 3; x++) {
				glm::vec3 pos(x * 0.68f - 0.68f - 2 / 3, y * 0.68f - 0.68f - 2 / 3, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * rotate * scale;
				glm::vec3 color(0.0f);
				if ((x + y) % 2 == 0)
					color = m_Color1;
				else
					color = m_Color2;
				std::dynamic_pointer_cast<Hazel::OpenGLShader>(colorShader)->UploadUniformFloat3("u_Color", color);
				Hazel::Renderer::Submit(colorShader, m_SquareVA, transform);
			}
		}

		m_DuckTexture->Bind();
		Hazel::Renderer::Submit(m_ShaderLibrary.Get("Texture"), m_TextureVA);
		m_ThinkTexture->Bind();
		Hazel::Renderer::Submit(m_ShaderLibrary.Get("Texture"), m_TextureVA);

		Hazel::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Color Picker");
		ImGui::ColorEdit3("Color 1: ", glm::value_ptr(m_Color1));
		ImGui::ColorEdit3("Color 2: ", glm::value_ptr(m_Color2));
		ImGui::End();
	}

	void OnEvent(Hazel::Event& e) override
	{
		m_CameraController.OnEvent(e);
		//Hazel::EventDispatcher dispatcher(event);
		//dispatcher.Dispatch<Hazel::KeyReleasedEvent>(HZ_BIND_EVENT_FN(ExampleLayer::OnKeyReleasedEvent));
	}

private:
	Hazel::ShaderLibrary m_ShaderLibrary;

	Hazel::Ref<Hazel::VertexArray> m_TextureVA;
	Hazel::Ref<Hazel::VertexArray> m_SquareVA;

	Hazel::Ref<Hazel::Texture2D> m_ThinkTexture, m_DuckTexture;

	glm::vec3 m_Color1 = { 0.2f, 0.3f, 0.8f };
	glm::vec3 m_Color2 = { 0.8f, 0.1f, 0.2f };

	Hazel::OrthographicCameraController m_CameraController;
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