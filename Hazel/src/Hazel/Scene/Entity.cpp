#include "hzpch.h"
#include "Entity.h"

namespace Hazel {

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
		//HZ_CORE_TRACE("created entity id: {0}", (uint32_t)handle);
	}

}
