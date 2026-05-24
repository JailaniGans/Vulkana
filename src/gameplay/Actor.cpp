// ===================================================================
// Vulkana Actor Implementation
// ===================================================================

#include "Actor.hpp"
#include <stdexcept>

namespace Vulkana
{
    Actor::Actor(World* world)
        : m_world(world), m_actorId(0), m_isActive(true)
    {
        // Entity creation is deferred to the World or Scene in a fuller implementation
        m_entity = entt::null;
    }

    Actor::~Actor()
    {
        EndPlay();
    }

    void Actor::BeginPlay()
    {
        // Default implementation does nothing
    }

    void Actor::Tick(float deltaTime)
    {
        (void)deltaTime;
        // Update components
        for (auto& comp : m_components)
        {
            if (comp)
            {
                comp->Tick(deltaTime);
            }
        }
    }

    void Actor::EndPlay()
    {
        // Destroy components
        m_components.clear();
    }

    ActorComponent* Actor::AddComponent(std::unique_ptr<ActorComponent> component)
    {
        if (!component)
            return nullptr;

        ActorComponent* ptr = component.get();
        m_components.push_back(std::move(component));
        return ptr;
    }

    void Actor::SetActorLocation(const glm::vec3& location)
    {
        m_location = location;
    }

    void Actor::SetActorRotation(const glm::vec3& rotation)
    {
        m_rotation = rotation;
    }

    void Actor::SetActorScale(const glm::vec3& scale)
    {
        m_scale = scale;
    }

    glm::vec3 Actor::GetActorLocation() const { return m_location; }
    glm::vec3 Actor::GetActorRotation() const { return m_rotation; }
    glm::vec3 Actor::GetActorScale() const { return m_scale; }

    glm::mat4 Actor::GetActorTransform() const
    {
        glm::mat4 t = glm::translate(glm::mat4(1.0f), m_location);
        // For brevity, only translation is applied here
        return t;
    }

} // namespace Vulkana
