#pragma once

#include "ast/statement.h"
#include "ast/variable_declaration.h"

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
// Function — AST Node for Function Definitions / Declarations (Day 11 & Day 18)
// =============================================================================
//
// Represents a function definition or declaration (prototype), including its
// return type, name, parameter list, parameter list kind, and body statements.
//
class Function : public Statement {
public:
    Function(std::string name,
             std::string returnType,
             SourceLocation loc = {},
             bool isDefinition = false,
             ParameterListKind paramListKind = ParameterListKind::Unspecified);
    ~Function() override;

    const std::string& name() const { return m_name; }
    const std::string& getName() const { return m_name; }
    const std::string& returnType() const { return m_returnType; }
    const std::string& getReturnType() const { return m_returnType; }

    void setReturnType(std::string returnType) { m_returnType = std::move(returnType); }
    void setName(std::string name) { m_name = std::move(name); }

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
    std::string m_returnType;
    std::vector<std::unique_ptr<VariableDeclaration>> m_parameters;
    std::vector<std::unique_ptr<Statement>> m_body;
    bool m_isDefinition{false};
    ParameterListKind m_paramListKind{ParameterListKind::Unspecified};
};

// Aliases for function declarations and definitions
using FunctionDecl = Function;
using FunctionDeclaration = Function;
using FunctionDef = Function;
using FunctionDefinition = Function;
