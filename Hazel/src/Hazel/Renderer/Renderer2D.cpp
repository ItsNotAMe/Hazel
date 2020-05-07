#include "hzpch.h"
#include "Renderer2D.h"

#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Shader.h"

#include "Hazel/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Hazel {

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexId;
		float TilingFactor;
	};

	struct Renderer2DData
	{
		static const uint32_t MaxQuads = 10000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextures = 32;

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		std::array<Ref<Texture>, MaxTextures> TextureSlots;
		uint32_t TextureSlotIndex = 1;

		//glm::vec4 QuadVertexPositions[4];

		Renderer2D::Statistics Stats;
	};

	static Renderer2DData s_Data;

	void Renderer2D::Init()
	{
		HZ_PROFILE_FUNCTION();

		s_Data.QuadVertexArray = VertexArray::Create();

		s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
		s_Data.QuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color" },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float, "a_TexIndex" },
			{ ShaderDataType::Float, "a_TilingFactor" }
		});
		s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

		uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];
		uint32_t offset = 0;
		for (int i = 0; i < s_Data.MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}
		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
		s_Data.QuadVertexArray->SetIndexBuffer(quadIB);
		delete[] quadIndices;

		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		s_Data.TextureShader = Shader::Create("assets/shaders/Texture.glsl");

		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		s_Data.TextureShader->Bind();
		int32_t samplers[Renderer2DData::MaxTextures];
		for (uint32_t  i = 0; i < Renderer2DData::MaxTextures; i++)
			samplers[i] = i;
		s_Data.TextureShader->SetIntArray("u_Textures", samplers, Renderer2DData::MaxTextures);

		/*s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };*/
	}

	void Renderer2D::Shutdown()
	{
		HZ_PROFILE_FUNCTION();
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Renderer2D::Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_Data.Stats;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		HZ_PROFILE_FUNCTION();

		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer2D::EndScene()
	{
		HZ_PROFILE_FUNCTION();

		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer2D::Flush()
	{
		HZ_PROFILE_FUNCTION();

		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			s_Data.TextureSlots[i]->Bind(i);

		s_Data.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
		s_Data.QuadVertexArray->UnBind();

		s_Data.Stats.DrawCalls++;
	}

	void Renderer2D::FlushAndReset()
	{
		EndScene();

		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		HZ_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			FlushAndReset();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		if (rotation != 0.0f)
			transform *= glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1));
		transform *= glm::inverse(glm::translate(glm::mat4(1.0f), position));

		/*glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 0, 1))
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		
		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
		*/

		s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4(position.x - size.x / 2, position.y - size.y / 2, position.z, 1.0f);
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexId = 0.0f;
		s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4(position.x + size.x / 2, position.y - size.y / 2, position.z, 1.0f);
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexId = 0.0f;
		s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4(position.x + size.x / 2, position.y + size.y / 2, position.z, 1.0f);
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexId = 0.0f;
		s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4(position.x - size.x / 2, position.y + size.y / 2, position.z, 1.0f);
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexId = 0.0f;
		s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;

		/*s_Data.WhiteTexture->Bind();

		s_Data.TextureShader->SetFloat4("u_Color", color);
		s_Data.TextureShader->SetFloat("u_RepeatCount", 1.0f);
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		if (rotation != 0.0f)
			transform *= glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1));
		transform *= glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
		s_Data.TextureShader->SetMat4("u_Transform", transform);

		s_Data.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray);*/
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture>& texture, float repeatCount, const glm::vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, rotation, texture, repeatCount, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture>& texture, float repeatCount, const glm::vec4& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices || s_Data.TextureSlotIndex > Renderer2DData::MaxTextures - 1)
			FlushAndReset();

		float textureId = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureId = (float)i;
				break;
			}
		}

		if (textureId == 0.0f)
		{
			textureId = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		if (rotation != 0.0f)
			transform *= glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1));
		transform *= glm::inverse(glm::translate(glm::mat4(1.0f), position));

		s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4(position.x - size.x / 2, position.y - size.y / 2, position.z, 1.0f);
		s_Data.QuadVertexBufferPtr->Color = tintColor;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexId = textureId;
		s_Data.QuadVertexBufferPtr->TilingFactor = repeatCount;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4(position.x + size.x / 2, position.y - size.y / 2, position.z, 1.0f);
		s_Data.QuadVertexBufferPtr->Color = tintColor;
		s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexId = textureId;
		s_Data.QuadVertexBufferPtr->TilingFactor = repeatCount;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4(position.x + size.x / 2, position.y + size.y / 2, position.z, 1.0f);
		s_Data.QuadVertexBufferPtr->Color = tintColor;
		s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexId = textureId;
		s_Data.QuadVertexBufferPtr->TilingFactor = repeatCount;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4(position.x - size.x / 2, position.y + size.y / 2, position.z, 1.0f);
		s_Data.QuadVertexBufferPtr->Color = tintColor;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexId = textureId;
		s_Data.QuadVertexBufferPtr->TilingFactor = repeatCount;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;

		/*texture->Bind();

		s_Data.TextureShader->SetFloat4("u_Color", tintColor);
		s_Data.TextureShader->SetFloat("u_RepeatCount", repeatCount);
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		if (rotation != 0.0f)
			transform *= glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1));
		transform *= glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f)); 
		s_Data.TextureShader->SetMat4("u_Transform", transform);

		s_Data.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray);*/
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, float repeatCount, const glm::vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, rotation, subTexture, repeatCount, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, float repeatCount, const glm::vec4& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices || s_Data.TextureSlotIndex > Renderer2DData::MaxTextures - 1)
			FlushAndReset();

		const glm::vec2* texCoords = subTexture->GetTextCoords();

		float textureId = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *subTexture->GetTexture().get())
			{
				textureId = (float)i;
				break;
			}
		}

		if (textureId == 0.0f)
		{
			textureId = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = subTexture->GetTexture();
			s_Data.TextureSlotIndex++;
		}

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		if (rotation != 0.0f)
			transform *= glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1));
		transform *= glm::inverse(glm::translate(glm::mat4(1.0f), position));

		s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4(position.x - size.x / 2, position.y - size.y / 2, position.z, 1.0f);
		s_Data.QuadVertexBufferPtr->Color = tintColor;
		s_Data.QuadVertexBufferPtr->TexCoord = texCoords[0];
		s_Data.QuadVertexBufferPtr->TexId = textureId;
		s_Data.QuadVertexBufferPtr->TilingFactor = repeatCount;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4(position.x + size.x / 2, position.y - size.y / 2, position.z, 1.0f);
		s_Data.QuadVertexBufferPtr->Color = tintColor;
		s_Data.QuadVertexBufferPtr->TexCoord = texCoords[1];
		s_Data.QuadVertexBufferPtr->TexId = textureId;
		s_Data.QuadVertexBufferPtr->TilingFactor = repeatCount;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4(position.x + size.x / 2, position.y + size.y / 2, position.z, 1.0f);
		s_Data.QuadVertexBufferPtr->Color = tintColor;
		s_Data.QuadVertexBufferPtr->TexCoord = texCoords[2];
		s_Data.QuadVertexBufferPtr->TexId = textureId;
		s_Data.QuadVertexBufferPtr->TilingFactor = repeatCount;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4(position.x - size.x / 2, position.y + size.y / 2, position.z, 1.0f);
		s_Data.QuadVertexBufferPtr->Color = tintColor;
		s_Data.QuadVertexBufferPtr->TexCoord = texCoords[3];
		s_Data.QuadVertexBufferPtr->TexId = textureId;
		s_Data.QuadVertexBufferPtr->TilingFactor = repeatCount;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;

		/*texture->Bind();

		s_Data.TextureShader->SetFloat4("u_Color", tintColor);
		s_Data.TextureShader->SetFloat("u_RepeatCount", repeatCount);
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		if (rotation != 0.0f)
			transform *= glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1));
		transform *= glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
		s_Data.TextureShader->SetMat4("u_Transform", transform);

		s_Data.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray);*/
	}

};