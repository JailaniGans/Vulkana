// ===================================================================
// Vulkana ActorComponent - Base class for Actor components
// ===================================================================

#pragma once

#include <string>

namespace Vulkana
{
    class Actor;

    class ActorComponent
    {
    public:
        explicit ActorComponent(const std::string& name = "ActorComponent")
            : m_name(name), m_owner(nullptr)
        {
        }

        virtual ~ActorComponent() = default;

        ActorComponent(const ActorComponent&) = delete;
        ActorComponent& operator=(const ActorComponent&) = delete;

        virtual void Initialize() {}
        virtual void Tick(float dt) { (void)dt; }
        virtual void Shutdown() {}

        void SetOwner(Actor* owner) { m_owner = owner; }
        Actor* GetOwner() const { return m_owner; }

        std::string GetName() const { return m_name; }

    protected:
        std::string m_name;
        Actor* m_owner;
    };

    // SceneComponent & PrimitiveComponent declared as convenience
    class SceneComponent : public ActorComponent
    {
    public:
        SceneComponent(const std::string& name = "SceneComponent")
            : ActorComponent(name) {}

        glm::mat4 GetWorldTransform() const { return glm::mat4(1.0f); }
    };

    class PrimitiveComponent : public ActorComponent
    {
    public:
        PrimitiveComponent(const std::string& name = "PrimitiveComponent")
            : ActorComponent(name) {}

        // Rendering handle placeholders
        void SetMeshID(uint32_t id) { (void)id; }
    };

} // namespace Vulkana
