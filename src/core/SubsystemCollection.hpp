// ===================================================================
// Vulkana Subsystem Collection Manager
// ===================================================================
// Manages the lifecycle, ordering, and retrieval of registered subsystems.
// Implements Unreal-style subsystem collection with auto-registration,
// LIFO deinitialization, and type-safe subsystem queries.
// ===================================================================

#pragma once

#include "Subsystem.hpp"
#include <vector>
#include <memory>
#include <typeinfo>
#include <algorithm>
#include <stdexcept>
#include <iostream>

namespace Vulkana
{
    // ================================================================
    // Subsystem Collection
    // ================================================================
    // Maintains an ordered registry of subsystems and manages their
    // lifecycle (Initialize -> Tick -> Shutdown).
    // Thread-unsafe; intended for use within single-threaded engine loop.
    // ================================================================
    class SubsystemCollection
    {
    public:
        SubsystemCollection() = default;

        ~SubsystemCollection()
        {
            // LIFO shutdown: destroy in reverse order of registration
            Shutdown();
        }

        // Deleted copy/move to prevent collection duplication
        SubsystemCollection(const SubsystemCollection&) = delete;
        SubsystemCollection& operator=(const SubsystemCollection&) = delete;
        SubsystemCollection(SubsystemCollection&&) = delete;
        SubsystemCollection& operator=(SubsystemCollection&&) = delete;

        // ================================================================
        // Subsystem Registration & Initialization
        // ================================================================

        /// Register and adopt a subsystem. The collection takes ownership.
        /// The subsystem is initialized immediately upon registration.
        /// Throws std::runtime_error if subsystem registration fails.
        void RegisterSubsystem(std::unique_ptr<Subsystem> subsystem)
        {
            if (!subsystem)
            {
                throw std::runtime_error("Cannot register null subsystem");
            }

            // Store the raw pointer before moving for logging
            Subsystem* rawPtr = subsystem.get();
            std::string subsystemName(rawPtr->GetName());
            std::string subsystemType(rawPtr->GetTypeName());

            // Add to collection (transfers ownership)
            m_subsystems.push_back(std::move(subsystem));

            // Initialize the newly registered subsystem
            try
            {
                rawPtr->Initialize();
                std::cout << "[Subsystem] Registered and initialized: " 
                          << subsystemName << " (" << subsystemType << ")" 
                          << std::endl;
            }
            catch (const std::exception& e)
            {
                std::cerr << "[Subsystem] Failed to initialize: " 
                          << subsystemName << " - " << e.what() << std::endl;
                m_subsystems.pop_back();
                throw;
            }
        }

        // ================================================================
        // Subsystem Lifecycle Management
        // ================================================================

        /// Initialize all registered subsystems (called after registration complete).
        /// Subsystems are initialized in registration order.
        void Initialize()
        {
            for (auto& subsystem : m_subsystems)
            {
                if (!subsystem->IsInitialized())
                {
                    try
                    {
                        subsystem->Initialize();
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << "[Subsystem] Failed to initialize " 
                                  << subsystem->GetName() 
                                  << ": " << e.what() << std::endl;
                        throw;
                    }
                }
            }
        }

        /// Tick all registered subsystems (called each frame).
        /// Subsystems are ticked in registration order.
        void Tick(float dt)
        {
            for (auto& subsystem : m_subsystems)
            {
                if (subsystem->IsInitialized())
                {
                    try
                    {
                        subsystem->Tick(dt);
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << "[Subsystem] Error ticking " 
                                  << subsystem->GetName() 
                                  << ": " << e.what() << std::endl;
                    }
                }
            }
        }

        /// Shutdown all registered subsystems (LIFO order).
        /// Subsystems are shut down in reverse registration order.
        void Shutdown()
        {
            // Reverse iteration for LIFO shutdown
            for (auto it = m_subsystems.rbegin(); it != m_subsystems.rend(); ++it)
            {
                if (*it && (*it)->IsInitialized())
                {
                    try
                    {
                        (*it)->Shutdown();
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << "[Subsystem] Error shutting down " 
                                  << (*it)->GetName() 
                                  << ": " << e.what() << std::endl;
                    }
                }
            }
            m_subsystems.clear();
        }

        // ================================================================
        // Subsystem Query & Retrieval
        // ================================================================

        /// Retrieve a subsystem by its derived type T.
        /// Returns nullptr if not found.
        template <typename T>
        T* GetSubsystem() const
        {
            static_assert(std::is_base_of<Subsystem, T>::value,
                "T must derive from Vulkana::Subsystem");

            for (auto& subsystem : m_subsystems)
            {
                T* cast = dynamic_cast<T*>(subsystem.get());
                if (cast)
                {
                    return cast;
                }
            }
            return nullptr;
        }

        /// Retrieve a subsystem by its derived type T.
        /// Throws std::runtime_error if not found.
        template <typename T>
        T* GetSubsystemChecked() const
        {
            T* result = GetSubsystem<T>();
            if (!result)
            {
                throw std::runtime_error(
                    std::string("Subsystem not found: ") + typeid(T).name()
                );
            }
            return result;
        }

        /// Check if a subsystem of type T is registered.
        template <typename T>
        bool HasSubsystem() const
        {
            static_assert(std::is_base_of<Subsystem, T>::value,
                "T must derive from Vulkana::Subsystem");

            return GetSubsystem<T>() != nullptr;
        }

        /// Returns the total count of registered subsystems.
        size_t GetSubsystemCount() const
        {
            return m_subsystems.size();
        }

    private:
        std::vector<std::unique_ptr<Subsystem>> m_subsystems;
    };

} // namespace Vulkana
