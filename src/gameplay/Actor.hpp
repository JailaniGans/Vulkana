// ===================================================================
// Vulkana Actor - Base Class for Game Objects
// ===================================================================
// An Actor is a game object that wraps an entt::entity. It serves as
// the high-level gameplay object abstraction, managing components,
// transform, and lifecycle within a scene.
// ===================================================================

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace Vulkana
{
    // Forward declarations
    class ActorComponent;
    class SceneComponent;
    class World;

    // ================================================================
    // Actor Base Class
    // ================================================================
    class Actor
    {
    public:
        explicit Actor(class World* world = nullptr);
        virtual ~Actor();

        // Deleted copy/move to prevent duplication
        Actor(const Actor&) = delete;
        Actor& operator=(const Actor&) = delete;
        Actor(Actor&&) = delete;
        Actor& operator=(Actor&&) = delete;

        // ================================================================
        // Actor Lifecycle
        // ================================================================

        /// Called once when the actor is spawned/initialized
        virtual void BeginPlay();

        /// Called every frame to update actor logic
        virtual void Tick(float deltaTime);

        /// Called once when the actor is destroyed
        virtual void EndPlay();

        // ================================================================
        // Component Management
        // ================================================================

        /// Attach a component to this actor. Actor takes ownership.
        ActorComponent* AddComponent(std::unique_ptr<ActorComponent> component);

        /// Get a component by type (templated)
        template <typename ComponentT>
        ComponentT* GetComponent() const
        {
            for (auto& comp : m_components)
            {
                ComponentT* cast = dynamic_cast<ComponentT*>(comp.get());
                if (cast)
                {
                    return cast;
                }
            }
            return nullptr;
        }

        /// Get all components
        const std::vector<std::unique_ptr<ActorComponent>>& GetComponents() const
        {
            return m_components;
        }

        // ================================================================
        // Transform & Position
        // ================================================================

        void SetActorLocation(const glm::vec3& location);
        void SetActorRotation(const glm::vec3& rotation);
        void SetActorScale(const glm::vec3& scale);

        glm::vec3 GetActorLocation() const;
        glm::vec3 GetActorRotation() const;
        glm::vec3 GetActorScale() const;
        glm::mat4 GetActorTransform() const;

        // ================================================================
        // Properties
        // ================================================================

        void SetActorName(const std::string& name) { m_name = name; }
        std::string GetActorName() const { return m_name; }

        uint32_t GetActorId() const { return m_actorId; }
        entt::entity GetEntityHandle() const { return m_entity; }

        bool IsActive() const { return m_isActive; }
        void SetActive(bool active) { m_isActive = active; }

        World* GetWorld() const { return m_world; }

    protected:
        World* m_world;
        entt::entity m_entity;
        uint32_t m_actorId;
        std::string m_name;
        bool m_isActive;

        std::vector<std::unique_ptr<ActorComponent>> m_components;

        // Transform data
        glm::vec3 m_location{0.0f};
        glm::vec3 m_rotation{0.0f};
        glm::vec3 m_scale{1.0f};
    };

} // namespace Vulkana
