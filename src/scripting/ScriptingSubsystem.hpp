// ===================================================================
// Vulkana Scripting Subsystem - Abstraction Layer
// ===================================================================
// Provides a VM-agnostic scripting interface for binding C++ types,
// methods, and properties to an embedded scripting runtime (Lua, C#, etc).
// Enables data-driven gameplay scripting without recompiling core binaries.
// ===================================================================

#pragma once

#include "core/Subsystem.hpp"
#include <memory>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <any>
#include <typeinfo>

namespace Vulkana
{
    // ================================================================
    // Scripting Type Information
    // ================================================================

    /// Type descriptor for a C++ type exposed to scripting
    struct FScriptType
    {
        std::string Name;
        const std::type_info* Type;
        std::string Module;

        FScriptType() : Name(""), Type(nullptr), Module("") {}

        FScriptType(const std::string& name, const std::type_info& type, 
                   const std::string& module = "")
            : Name(name), Type(&type), Module(module)
        {
        }
    };

    /// Method signature for C++ methods bound to scripting
    using ScriptMethodFunc = std::function<void(void* instance, const std::vector<std::any>& args)>;
    using ScriptPropertyGetter = std::function<std::any(void* instance)>;
    using ScriptPropertySetter = std::function<void(void* instance, const std::any& value)>;

    /// Method descriptor
    struct FScriptMethod
    {
        std::string Name;
        ScriptMethodFunc Func;
        std::vector<std::string> ParameterTypes;

        FScriptMethod(const std::string& name, ScriptMethodFunc func)
            : Name(name), Func(func)
        {
        }
    };

    /// Property descriptor
    struct FScriptProperty
    {
        std::string Name;
        std::string Type;
        ScriptPropertyGetter Getter;
        ScriptPropertySetter Setter;
        bool IsReadOnly;

        FScriptProperty(const std::string& name, const std::string& type,
                       ScriptPropertyGetter getter, ScriptPropertySetter setter = nullptr,
                       bool readOnly = false)
            : Name(name), Type(type), Getter(getter), Setter(setter), IsReadOnly(readOnly)
        {
        }
    };

    // ================================================================
    // Abstract Virtual Machine Interface
    // ================================================================

    class ScriptVM
    {
    public:
        virtual ~ScriptVM() = default;

        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;

        virtual bool ExecuteString(const std::string& code) = 0;
        virtual bool ExecuteFile(const std::string& filePath) = 0;

        virtual void RegisterType(const FScriptType& type) = 0;
        virtual void RegisterMethod(const FScriptType& type, const FScriptMethod& method) = 0;
        virtual void RegisterProperty(const FScriptType& type, const FScriptProperty& property) = 0;

        virtual bool CallFunction(const std::string& functionName, 
                                 const std::vector<std::any>& args = {}) = 0;

        virtual std::string GetLastError() const = 0;
    };

    // ================================================================
    // Scripting Subsystem - Type-Safe Binding Interface
    // ================================================================

    class ScriptingSubsystem : public TSubsystem<ScriptingSubsystem>
    {
    public:
        ScriptingSubsystem();
        virtual ~ScriptingSubsystem() override;

        // ================================================================
        // Subsystem Lifecycle
        // ================================================================

        virtual void Initialize() override;
        virtual void Tick(float dt) override;
        virtual void Shutdown() override;

        // ================================================================
        // VM Lifecycle & Management
        // ================================================================

        /// Set the virtual machine implementation for this subsystem.
        /// Transfers ownership to the subsystem.
        void SetVirtualMachine(std::unique_ptr<ScriptVM> vm);

        /// Get the active virtual machine (nullptr if not set).
        ScriptVM* GetVirtualMachine() const { return m_vm.get(); }

        /// Check if a virtual machine is active.
        bool HasVirtualMachine() const { return m_vm != nullptr; }

        // ================================================================
        // Script Execution
        // ================================================================

        /// Execute a script string in the VM.
        bool ExecuteScript(const std::string& code);

        /// Load and execute a script file from disk.
        bool ExecuteScriptFile(const std::string& filePath);

        /// Call a global function in the script environment.
        bool CallScriptFunction(const std::string& functionName, 
                               const std::vector<std::any>& args = {});

        // ================================================================
        // Type Registration Interface
        // ================================================================

        /// Register a C++ type with the scripting system.
        /// Returns true on success.
        bool RegisterScriptType(const FScriptType& type);

        /// Register a C++ method on a scripted type.
        bool RegisterScriptMethod(const std::string& typeName, 
                                 const FScriptMethod& method);

        /// Register a C++ property on a scripted type.
        bool RegisterScriptProperty(const std::string& typeName, 
                                   const FScriptProperty& property);

        /// Check if a type is registered.
        bool IsTypeRegistered(const std::string& typeName) const;

        // ================================================================
        // Templated Type Binding (Helper Methods)
        // ================================================================

        /// Template method to bind a member function with automatic wrapping.
        template <typename ClassT, typename MethodT>
        bool BindMethod(const std::string& typeName, 
                       const std::string& methodName, 
                       MethodT method)
        {
            if (!HasVirtualMachine())
            {
                LOG_ERROR("No virtual machine set");
                return false;
            }

            // Wrapper that converts method pointer to ScriptMethodFunc
            ScriptMethodFunc wrapper = [method](void* instance, 
                                               const std::vector<std::any>& args)
            {
                // This is a simplified wrapper; full implementation would
                // use template specialization for different method signatures
                ClassT* obj = static_cast<ClassT*>(instance);
                if (obj && method)
                {
                    (obj->*method)();
                }
            };

            FScriptMethod scriptMethod(methodName, wrapper);
            return RegisterScriptMethod(typeName, scriptMethod);
        }

        /// Template method to bind a member property getter.
        template <typename ClassT, typename PropertyT>
        bool BindProperty(const std::string& typeName,
                         const std::string& propertyName,
                         PropertyT ClassT::* propertyMember,
                         bool readOnly = false)
        {
            if (!HasVirtualMachine())
            {
                LOG_ERROR("No virtual machine set");
                return false;
            }

            ScriptPropertyGetter getter = [propertyMember](void* instance) -> std::any
            {
                ClassT* obj = static_cast<ClassT*>(instance);
                if (obj)
                {
                    return std::any(obj->*propertyMember);
                }
                return std::any();
            };

            ScriptPropertySetter setter = readOnly ? nullptr :
                [propertyMember](void* instance, const std::any& value)
                {
                    ClassT* obj = static_cast<ClassT*>(instance);
                    if (obj)
                    {
                        obj->*propertyMember = std::any_cast<PropertyT>(value);
                    }
                };

            FScriptProperty prop(propertyName, typeid(PropertyT).name(), 
                               getter, setter, readOnly);
            return RegisterScriptProperty(typeName, prop);
        }

        // ================================================================
        // State Query
        // ================================================================

        /// Get the last error message from the scripting system.
        std::string GetLastError() const;

        /// Get the number of registered script types.
        size_t GetRegisteredTypeCount() const { return m_registeredTypes.size(); }

        /// Get all registered script type names.
        std::vector<std::string> GetRegisteredTypeNames() const;

    private:
        std::unique_ptr<ScriptVM> m_vm;
        std::map<std::string, FScriptType> m_registeredTypes;
        std::map<std::string, std::vector<FScriptMethod>> m_typeMethods;
        std::map<std::string, std::vector<FScriptProperty>> m_typeProperties;
        bool m_isInitialized;
    };

} // namespace Vulkana
