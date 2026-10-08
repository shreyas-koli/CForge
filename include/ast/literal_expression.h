#pragma once

#include "ast/expression.h"
#include <string>

// =============================================================================
// LiteralKind — Kinds of primary literal expressions
// =============================================================================
enum class LiteralKind {
    Integer,
    Float,
    Char,
    String
};

const char* literalKindName(LiteralKind kind);

// =============================================================================
// LiteralExpression — AST Node for Literal Values (Integer, Float, Char, String)
// =============================================================================
class LiteralExpression : public Expression {
public:
    LiteralExpression(LiteralKind kind, std::string value, SourceLocation loc = {});
    ~LiteralExpression() override;

    LiteralKind kind() const { return m_kind; }
    const std::string& value() const { return m_value; }

    std::string toString() const override;

private:
    LiteralKind m_kind;
    std::string m_value;
};
