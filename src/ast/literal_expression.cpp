#include "ast/literal_expression.h"
#include <utility>

const char* literalKindName(LiteralKind kind) {
    switch (kind) {
        case LiteralKind::Integer: return "Integer";
        case LiteralKind::Float:   return "Float";
        case LiteralKind::Char:    return "Char";
        case LiteralKind::String:  return "String";
    }
    return "Unknown";
}

LiteralExpression::LiteralExpression(LiteralKind kind, std::string value, SourceLocation loc)
    : Expression(loc)
    , m_kind(kind)
    , m_value(std::move(value)) {}

LiteralExpression::~LiteralExpression() = default;

std::string LiteralExpression::toString() const {
    return std::string("LiteralExpression(") + literalKindName(m_kind) + ", " + m_value + ")";
}
