// ===================================================================
// Vulkana Subsystem Base Class
// ===================================================================
// Implements Unreal-style modular subsystem lifecycle management.
// All engine managers derive from this to enable auto-registration,
// initialization ordering, and safe deinitialization.
// ===================================================================

#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <typeinfo>

namespace Vulkana
{
    // ================================================================
    // Subsystem Base Class
    // ================================================================
    // Provides lifecycle hooks: Initialize(), Tick(float dt), Shutdown()
    // Subsystems are automatically managed by SubsystemCollection.
    // ================================================================
    class Subsystem
    {
    public:
        explicit Subsystem(std::string_view name = "") 
            : m_name(name), m_isInitialized(false)
        {
        }

        virtual ~Subsystem() = default;

        // Deleted copy/move to prevent subsystem duplication
        Subsystem(const Subsystem&) = delete;
        Subsystem& operator=(const Subsystem&) = delete;
        Subsystem(Subsystem&&) = delete;
        Subsystem& operator=(Subsystem&&) = delete;

        // ================================================================
        // Lifecycle Hooks
        // ================================================================

        /// Called once when the subsystem is registered and initialized.
        /// Override to perform setup (resource allocation, initialization, etc).
        virtual void Initialize()
        {
            m_isInitialized = true;
        }

        /// Called every frame before game update. dt is delta time in seconds.
        /// Override to implement per-frame logic (ticking, polling, updates).
        virtual void Tick(float dt)
        {
            (void)dt; // Suppress unused parameter warning
        }

        /// Called once when the subsystem is being deinitialized.
        /// Override to perform cleanup (resource deallocation, finalization, etc).
        virtual void Shutdown()
        {
            m_isInitialized = false;
        }

        // ================================================================
        // State Query
        // ================================================================

        /// Returns true if this subsystem has been initialized.
        bool IsInitialized() const { return m_isInitialized; }

        /// Returns the human-readable name of this subsystem.
        std::string_view GetName() const { return m_name; }

        /// Returns the type name (via RTTI) of this subsystem.
        std::string_view GetTypeName() const 
        { 
            return std::string_view(typeid(*this).name()); 
        }

    protected:
        std::string m_name;
        bool m_isInitialized;
    };

    // ================================================================
    // Templated Subsystem Class (for type-safe derived subsystems)
    // ================================================================
    // Allows subsystems to define a static type-safe interface.
    // ================================================================
    template <typename DerivedT>
    class TSubsystem : public Subsystem
    {
    public:
        explicit TSubsystem(std::string_view name = "") 
            : Subsystem(name)
        {
        }

        virtual ~TSubsystem() = default;

        // Returns the static type identifier for this subsystem class.
        static std::string_view StaticTypeName()
        {
            return std::string_view(typeid(DerivedT).name());
        }
    };

} // namespace Vulkana
