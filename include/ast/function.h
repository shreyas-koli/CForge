#pragma once

#include "ast/statement.h"
#include "ast/variable_declaration.h"
#include "ast/storage_class.h"
#include "types/type.h"

#include <memory>
#include <string>
#include <vector>

// =============================================================================
// ParameterListKind — C Parameter List Specification (C17 compatibility)
// =============================================================================
//
// In C17 (ISO/IEC 9899:2018):
//   - Unspecified: "int f();" — Function declarator with no parameter info (unspecified args).
//   - Void:        "int f(void);" — Function prototype explicitly declaring zero parameters.
//   - Typed:       "int f(int a, char b);" — Function prototype declaring typed parameters.
//
enum class ParameterListKind {
    Unspecified, // "()" — C17 unspecified parameter list (takes unspecified arguments)
    Void,        // "(void)" — explicitly empty parameter list (takes zero arguments)
    Typed,       // "(...)" — one or more typed parameters (with or without identifiers)
};

// =============================================================================
// Function — AST Node for Function Definitions / Declarations (Day 11, 18, 40, 41)
// =============================================================================
//
// Represents a function definition or declaration (prototype), including its
// return type (Day 41), name, parameter list (Day 40), parameter list kind,
// and body statements.
//
class Function : public Statement {
public:
    // Preferred constructor taking std::shared_ptr<Type> (Day 41 & Day 42)
    Function(std::string name,
             std::shared_ptr<Type> returnType,
             SourceLocation loc = {},
             bool isDefinition = false,
             ParameterListKind paramListKind = ParameterListKind::Unspecified,
             StorageClass storageClass = StorageClass::None);

    // Constructor taking string return type (backward compatibility)
    Function(std::string name,
             std::string returnType,
             SourceLocation loc = {},
             bool isDefinition = false,
             ParameterListKind paramListKind = ParameterListKind::Unspecified,
             StorageClass storageClass = StorageClass::None);

    ~Function() override;

    // Function name accessors
    const std::string& name() const { return m_name; }
    const std::string& getName() const { return m_name; }
    void setName(std::string name) { m_name = std::move(name); }

    // Storage class accessors (Day 42)
    StorageClass storageClass() const { return m_storageClass; }
    StorageClass getStorageClass() const { return m_storageClass; }
    bool isStatic() const { return m_storageClass == StorageClass::Static; }
    bool isExtern() const { return m_storageClass == StorageClass::Extern; }
    void setStorageClass(StorageClass sc) { m_storageClass = sc; }

    // Return type accessors (Day 41)
    TypeRef returnType() const { return TypeRef(m_returnType.get(), m_returnTypeName); }
    const std::string& returnTypeName() const { return m_returnTypeName; }
    const std::string& getReturnTypeName() const { return m_returnTypeName; }
    const std::string& getReturnType() const { return m_returnTypeName; }

    const Type* returnTypePtr() const { return m_returnType.get(); }
    const Type* declaredReturnType() const { return m_returnType.get(); }
    const std::shared_ptr<Type>& returnTypeShared() const { return m_returnType; }
    const std::shared_ptr<Type>& getReturnTypeShared() const { return m_returnType; }
    const std::shared_ptr<Type>& getType() const { return m_returnType; }
    TypeKind returnTypeKind() const { return m_returnType ? m_returnType->kind() : TypeKind::Unknown; }

    bool isReturnVoid() const { return m_returnType && m_returnType->isVoid(); }
    bool isReturnInt() const { return m_returnType && m_returnType->isInt(); }
    bool isReturnChar() const { return m_returnType && m_returnType->isChar(); }
    bool isReturnFloat() const { return m_returnType && m_returnType->isFloat(); }
    bool isReturnDouble() const { return m_returnType && m_returnType->isDouble(); }

    void setReturnType(std::string returnType);
    void setReturnType(std::shared_ptr<Type> returnType);

    // Declaration vs Definition
    bool isDefinition() const { return m_isDefinition; }
    bool isDeclaration() const { return !m_isDefinition; }
    bool isPrototype() const { return !m_isDefinition; }
    void setIsDefinition(bool isDef) { m_isDefinition = isDef; }

    // Parameter list kind (C17: empty '()' vs explicit '(void)' vs 'typed')
    ParameterListKind parameterListKind() const { return m_paramListKind; }
    ParameterListKind getParameterListKind() const { return m_paramListKind; }
    void setParameterListKind(ParameterListKind kind) { m_paramListKind = kind; }

    bool isParameterListUnspecified() const { return m_paramListKind == ParameterListKind::Unspecified; }
    bool isParameterListVoid() const { return m_paramListKind == ParameterListKind::Void; }
    bool isParameterListTyped() const { return m_paramListKind == ParameterListKind::Typed; }
    bool hasVoidParameterList() const { return m_paramListKind == ParameterListKind::Void; }
    bool hasUnspecifiedParameters() const { return m_paramListKind == ParameterListKind::Unspecified; }

    // Parameter management
    const std::vector<std::unique_ptr<VariableDeclaration>>& parameters() const {
        return m_parameters;
    }
    const std::vector<std::unique_ptr<VariableDeclaration>>& getParameters() const {
        return m_parameters;
    }
    std::size_t parameterCount() const { return m_parameters.size(); }
    void addParameter(std::unique_ptr<VariableDeclaration> param);

    const VariableDeclaration* parameter(std::size_t index) const {
        return index < m_parameters.size() ? m_parameters[index].get() : nullptr;
    }
    const VariableDeclaration* getParameter(std::size_t index) const {
        return index < m_parameters.size() ? m_parameters[index].get() : nullptr;
    }

    // Body statements management
    const std::vector<std::unique_ptr<Statement>>& body() const { return m_body; }
    const std::vector<std::unique_ptr<Statement>>& getBody() const { return m_body; }
    std::size_t bodyStatementCount() const { return m_body.size(); }
    void addBodyStatement(std::unique_ptr<Statement> stmt);

    std::string toString() const override;

private:
    std::string m_name;
    std::shared_ptr<Type> m_returnType;
    std::string m_returnTypeName;
    std::vector<std::unique_ptr<VariableDeclaration>> m_parameters;
    std::vector<std::unique_ptr<Statement>> m_body;
    bool m_isDefinition{false};
    ParameterListKind m_paramListKind{ParameterListKind::Unspecified};
    StorageClass m_storageClass{StorageClass::None};
};

// Aliases for function declarations and definitions
using FunctionDecl = Function;
using FunctionDeclaration = Function;
using FunctionDef = Function;
using FunctionDefinition = Function;
