#include "ast/conditional_expr.h"
#include <utility>

ConditionalExpr::ConditionalExpr(std::unique_ptr<Expression> condition,
                                 std::unique_ptr<Expression> thenExpr,
                                 std::unique_ptr<Expression> elseExpr,
                                 SourceLocation loc)
    : Expression(loc)
    , m_condition(std::move(condition))
    , m_thenExpr(std::move(thenExpr))
    , m_elseExpr(std::move(elseExpr)) {}

ConditionalExpr::~ConditionalExpr() = default;

std::string ConditionalExpr::toString() const {
    std::string condStr = m_condition ? m_condition->toString() : "null";
    std::string thenStr = m_thenExpr ? m_thenExpr->toString() : "null";
    std::string elseStr = m_elseExpr ? m_elseExpr->toString() : "null";
    return "ConditionalExpr(" + condStr + ", " + thenStr + ", " + elseStr + ")";
}
