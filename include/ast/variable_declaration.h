#pragma once

#include "ast/statement.h"
#include "ast/expression.h"

#include <memory>
#include <string>

// =============================================================================
// VariableDeclaration — AST Node for Variable Declarations
// =============================================================================
//
// Represents variable declarations such as "int x = 10;" or "float y;".
//
class VariableDeclaration : public Statement {
public:
    VariableDeclaration(std::string name,
                         std::string type,
                         std::unique_ptr<Expression> initializer = nullptr,
                         SourceLocation loc = {});
    ~VariableDeclaration() override;

    const std::string& name() const { return m_name; }
    const std::string& type() const { return m_type; }

    bool hasInitializer() const { return m_initializer != nullptr; }
    const Expression* initializer() const { return m_initializer.get(); }
    Expression* initializer() { return m_initializer.get(); }

    std::unique_ptr<Expression> takeInitializer();
    void setInitializer(std::unique_ptr<Expression> initializer);

    std::string toString() const override;

private:
    std::string m_name;
    std::string m_type;
    std::unique_ptr<Expression> m_initializer;
};
