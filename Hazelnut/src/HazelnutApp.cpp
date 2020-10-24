#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "EditorLayer.h"

namespace Hazel {

	class Hazelnut : public Application
	{
	public:
		Hazelnut()
			: Application("Hazel Editor")
		{
			PushLayer(new EditorLayer());
		}

		~Hazelnut()
		{

		}

	};

	Application* CreateApplication()
	{
		return new Hazelnut();
	}

}