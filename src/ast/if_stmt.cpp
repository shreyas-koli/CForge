#include "ast/if_stmt.h"
#include <utility>

IfStmt::IfStmt(std::unique_ptr<Expression> condition,
               std::unique_ptr<Statement> thenBranch,
               SourceLocation loc)
    : Statement(loc)
    , m_condition(std::move(condition))
    , m_thenBranch(std::move(thenBranch))
    , m_elseBranch(nullptr) {}

IfStmt::IfStmt(std::unique_ptr<Expression> condition,
               std::unique_ptr<Statement> thenBranch,
               std::unique_ptr<Statement> elseBranch,
               SourceLocation loc)
    : Statement(loc)
    , m_condition(std::move(condition))
    , m_thenBranch(std::move(thenBranch))
    , m_elseBranch(std::move(elseBranch)) {}

IfStmt::~IfStmt() = default;

std::string IfStmt::toString() const {
    std::string condStr = m_condition ? m_condition->toString() : "null";
    std::string thenStr = m_thenBranch ? m_thenBranch->toString() : "null";
    if (m_elseBranch) {
        return "IfStmt(" + condStr + ", " + thenStr + ", " + m_elseBranch->toString() + ")";
    }
    return "IfStmt(" + condStr + ", " + thenStr + ")";
}
