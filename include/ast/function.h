#pragma once

#include "ast/statement.h"
#include "ast/variable_declaration.h"

#include <memory>
#include <string>
#include <vector>

// =============================================================================
// Function — AST Node for Function Definitions / Declarations
// =============================================================================
//
// Represents a function definition or declaration, including its name,
// return type, parameter list, and body statements.
//
class Function : public Statement {
public:
    Function(std::string name,
             std::string returnType,
             SourceLocation loc = {});
    ~Function() override;

    const std::string& name() const { return m_name; }
    const std::string& returnType() const { return m_returnType; }

    void setReturnType(std::string returnType) { m_returnType = std::move(returnType); }
    void setName(std::string name) { m_name = std::move(name); }

    // Parameter management
    const std::vector<std::unique_ptr<VariableDeclaration>>& parameters() const {
        return m_parameters;
    }
    void addParameter(std::unique_ptr<VariableDeclaration> param);

    // Body statements management
    const std::vector<std::unique_ptr<Statement>>& body() const { return m_body; }
    void addBodyStatement(std::unique_ptr<Statement> stmt);

    std::string toString() const override;

private:
    std::string m_name;
    std::string m_returnType;
    std::vector<std::unique_ptr<VariableDeclaration>> m_parameters;
    std::vector<std::unique_ptr<Statement>> m_body;
};
