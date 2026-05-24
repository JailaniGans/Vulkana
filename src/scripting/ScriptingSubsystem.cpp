// ===================================================================
// Vulkana Scripting Subsystem - Implementation
// ===================================================================
// Implements the scripting subsystem with type binding and VM management.
// ===================================================================

#include "ScriptingSubsystem.hpp"
#include "core/Log.hpp"
#include <algorithm>

namespace Vulkana
{
    // ================================================================
    // ScriptingSubsystem Implementation
    // ================================================================

    ScriptingSubsystem::ScriptingSubsystem()
        : TSubsystem("ScriptingSubsystem"),
          m_vm(nullptr),
          m_isInitialized(false)
    {
    }

    ScriptingSubsystem::~ScriptingSubsystem()
    {
        if (m_vm)
        {
            m_vm->Shutdown();
            m_vm.reset();
        }
    }

    // ================================================================
    // Subsystem Lifecycle
    // ================================================================

    void ScriptingSubsystem::Initialize()
    {
        TSubsystem::Initialize();

        if (m_vm)
        {
            if (m_vm->Initialize())
            {
                m_isInitialized = true;
                LOG_INFO("ScriptingSubsystem initialized with VM");
            }
            else
            {
                LOG_ERROR("Failed to initialize virtual machine");
            }
        }
        else
        {
            LOG_INFO("ScriptingSubsystem initialized (no VM set)");
        }
    }

    void ScriptingSubsystem::Tick(float dt)
    {
        TSubsystem::Tick(dt);
        // VM-specific ticking could happen here if needed
        (void)dt;
    }

    void ScriptingSubsystem::Shutdown()
    {
        if (m_vm)
        {
            m_vm->Shutdown();
            m_vm.reset();
        }

        m_registeredTypes.clear();
        m_typeMethods.clear();
        m_typeProperties.clear();

        m_isInitialized = false;
        TSubsystem::Shutdown();

        LOG_INFO("ScriptingSubsystem shutdown complete");
    }

    // ================================================================
    // VM Lifecycle & Management
    // ================================================================

    void ScriptingSubsystem::SetVirtualMachine(std::unique_ptr<ScriptVM> vm)
    {
        if (m_vm)
        {
            LOG_WARN("Replacing existing virtual machine");
            m_vm->Shutdown();
        }

        m_vm = std::move(vm);

        if (m_vm)
        {
            LOG_INFO("Virtual machine set");
        }
    }

    // ================================================================
    // Script Execution
    // ================================================================

    bool ScriptingSubsystem::ExecuteScript(const std::string& code)
    {
        if (!m_vm)
        {
            LOG_ERROR("No virtual machine set");
            return false;
        }

        try
        {
            return m_vm->ExecuteString(code);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Script execution error: " + std::string(e.what()));
            return false;
        }
    }

    bool ScriptingSubsystem::ExecuteScriptFile(const std::string& filePath)
    {
        if (!m_vm)
        {
            LOG_ERROR("No virtual machine set");
            return false;
        }

        try
        {
            return m_vm->ExecuteFile(filePath);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Script file execution error: " + std::string(e.what()));
            return false;
        }
    }

    bool ScriptingSubsystem::CallScriptFunction(const std::string& functionName,
                                               const std::vector<std::any>& args)
    {
        if (!m_vm)
        {
            LOG_ERROR("No virtual machine set");
            return false;
        }

        try
        {
            return m_vm->CallFunction(functionName, args);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Script function call error: " + std::string(e.what()));
            return false;
        }
    }

    // ================================================================
    // Type Registration Interface
    // ================================================================

    bool ScriptingSubsystem::RegisterScriptType(const FScriptType& type)
    {
        if (!m_vm)
        {
            LOG_ERROR("No virtual machine set");
            return false;
        }

        if (m_registeredTypes.find(type.Name) != m_registeredTypes.end())
        {
            LOG_WARN("Script type already registered: " + type.Name);
            return false;
        }

        try
        {
            m_vm->RegisterType(type);
            m_registeredTypes[type.Name] = type;

            LOG_INFO("Registered script type: " + type.Name);
            return true;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Failed to register type " + type.Name + ": " + e.what());
            return false;
        }
    }

    bool ScriptingSubsystem::RegisterScriptMethod(const std::string& typeName,
                                                 const FScriptMethod& method)
    {
        if (!m_vm)
        {
            LOG_ERROR("No virtual machine set");
            return false;
        }

        auto typeIt = m_registeredTypes.find(typeName);
        if (typeIt == m_registeredTypes.end())
        {
            LOG_ERROR("Type not registered: " + typeName);
            return false;
        }

        try
        {
            m_vm->RegisterMethod(typeIt->second, method);
            m_typeMethods[typeName].push_back(method);

            LOG_INFO("Registered method " + method.Name + " on type " + typeName);
            return true;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Failed to register method " + method.Name + ": " + e.what());
            return false;
        }
    }

    bool ScriptingSubsystem::RegisterScriptProperty(const std::string& typeName,
                                                   const FScriptProperty& property)
    {
        if (!m_vm)
        {
            LOG_ERROR("No virtual machine set");
            return false;
        }

        auto typeIt = m_registeredTypes.find(typeName);
        if (typeIt == m_registeredTypes.end())
        {
            LOG_ERROR("Type not registered: " + typeName);
            return false;
        }

        try
        {
            m_vm->RegisterProperty(typeIt->second, property);
            m_typeProperties[typeName].push_back(property);

            LOG_INFO("Registered property " + property.Name + " on type " + typeName);
            return true;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Failed to register property " + property.Name + ": " + e.what());
            return false;
        }
    }

    bool ScriptingSubsystem::IsTypeRegistered(const std::string& typeName) const
    {
        return m_registeredTypes.find(typeName) != m_registeredTypes.end();
    }

    // ================================================================
    // State Query
    // ================================================================

    std::string ScriptingSubsystem::GetLastError() const
    {
        if (m_vm)
        {
            return m_vm->GetLastError();
        }
        return "No virtual machine set";
    }

    std::vector<std::string> ScriptingSubsystem::GetRegisteredTypeNames() const
    {
        std::vector<std::string> typeNames;
        for (const auto& pair : m_registeredTypes)
        {
            typeNames.push_back(pair.first);
        }
        return typeNames;
    }

} // namespace Vulkana
