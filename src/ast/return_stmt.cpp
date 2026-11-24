#include "ast/return_stmt.h"

ReturnStmt::ReturnStmt(std::unique_ptr<Expression> expr, SourceLocation loc)
    : Statement(loc)
    , m_expr(std::move(expr))
{}

ReturnStmt::ReturnStmt(SourceLocation loc)
    : Statement(loc)
    , m_expr(nullptr)
{}

ReturnStmt::~ReturnStmt() = default;

std::string ReturnStmt::toString() const {
    if (m_expr) {
        return "ReturnStmt(" + m_expr->toString() + ")";
    }
    return "ReturnStmt";
}
