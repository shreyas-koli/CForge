#include "ast/do_while_stmt.h"
#include <utility>

DoWhileStmt::DoWhileStmt(std::unique_ptr<Statement> body,
                         std::unique_ptr<Expression> condition,
                         SourceLocation loc)
    : Statement(loc)
    , m_body(std::move(body))
    , m_condition(std::move(condition)) {}

DoWhileStmt::~DoWhileStmt() = default;

std::string DoWhileStmt::toString() const {
    std::string bodyStr = m_body ? m_body->toString() : "null";
    std::string condStr = m_condition ? m_condition->toString() : "null";
    return "DoWhileStmt(" + bodyStr + ", " + condStr + ")";
}

