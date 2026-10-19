#include "ast/unary_expr.h"
#include <utility>

UnaryExpr::UnaryExpr(std::string op,
                     std::unique_ptr<Expression> operand,
                     SourceLocation loc)
    : Expression(loc)
    , m_op(std::move(op))
    , m_operand(std::move(operand)) {}

UnaryExpr::~UnaryExpr() = default;

std::string UnaryExpr::toString() const {
    std::string opStr = m_operand ? m_operand->toString() : "null";
    return "UnaryExpr(" + m_op + ", " + opStr + ")";
}
