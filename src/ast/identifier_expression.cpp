#include "ast/identifier_expression.h"
#include <utility>

IdentifierExpression::IdentifierExpression(std::string name, SourceLocation loc)
    : Expression(loc)
    , m_name(std::move(name)) {}

IdentifierExpression::~IdentifierExpression() = default;

std::string IdentifierExpression::toString() const {
    return "IdentifierExpression(" + m_name + ")";
}
