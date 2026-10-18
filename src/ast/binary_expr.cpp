#include "ast/binary_expr.h"
#include <utility>

BinaryExpr::BinaryExpr(std::string op,
                       std::unique_ptr<Expression> left,
                       std::unique_ptr<Expression> right,
                       SourceLocation loc)
    : Expression(loc)
    , m_op(std::move(op))
    , m_left(std::move(left))
    , m_right(std::move(right)) {}

BinaryExpr::~BinaryExpr() = default;

std::string BinaryExpr::toString() const {
    std::string lStr = m_left ? m_left->toString() : "null";
    std::string rStr = m_right ? m_right->toString() : "null";
    return "BinaryExpr(" + m_op + ", " + lStr + ", " + rStr + ")";
}
