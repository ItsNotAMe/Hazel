#pragma once

#include "entt.hpp"

#include "Hazel/Core/Timestep.h"

namespace Hazel {

	class Scene
	{
	public:
		Scene();
		~Scene();

		entt::entity CreateEntity();

		// Temporary
		entt::registry& Reg() { return m_Registry; }

		void OnUpdate(Timestep ts);
	private:
		entt::registry m_Registry;
	};

}