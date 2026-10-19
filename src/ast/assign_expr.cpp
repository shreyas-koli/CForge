#include "ast/assign_expr.h"
#include <utility>

AssignExpr::AssignExpr(std::string op,
                       std::unique_ptr<Expression> left,
                       std::unique_ptr<Expression> right,
                       SourceLocation loc)
    : Expression(loc)
    , m_op(std::move(op))
    , m_left(std::move(left))
    , m_right(std::move(right)) {}

AssignExpr::~AssignExpr() = default;

std::string AssignExpr::toString() const {
    std::string lStr = m_left ? m_left->toString() : "null";
    std::string rStr = m_right ? m_right->toString() : "null";
    return "AssignExpr(" + m_op + ", " + lStr + ", " + rStr + ")";
}
