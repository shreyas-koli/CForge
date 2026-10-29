#include "ast/while_stmt.h"
#include <utility>

WhileStmt::WhileStmt(std::unique_ptr<Expression> condition,
                     std::unique_ptr<Statement> body,
                     SourceLocation loc)
    : Statement(loc)
    , m_condition(std::move(condition))
    , m_body(std::move(body)) {}

WhileStmt::~WhileStmt() = default;

std::string WhileStmt::toString() const {
    std::string condStr = m_condition ? m_condition->toString() : "null";
    std::string bodyStr = m_body ? m_body->toString() : "null";
    return "WhileStmt(" + condStr + ", " + bodyStr + ")";
}

