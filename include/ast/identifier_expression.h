#pragma once

#include "ast/expression.h"
#include <string>

// =============================================================================
// IdentifierExpression — AST Node for Bare Identifiers
// =============================================================================
class IdentifierExpression : public Expression {
public:
    IdentifierExpression(std::string name, SourceLocation loc = {});
    ~IdentifierExpression() override;

    const std::string& name() const { return m_name; }

    std::string toString() const override;

private:
    std::string m_name;
};
