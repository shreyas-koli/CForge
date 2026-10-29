#include "ast/compound_stmt.h"
#include <utility>

CompoundStmt::CompoundStmt(SourceLocation loc)
    : Statement(loc) {}

CompoundStmt::CompoundStmt(std::vector<std::unique_ptr<Statement>> statements,
                           SourceLocation loc)
    : Statement(loc)
    , m_statements(std::move(statements)) {}

CompoundStmt::~CompoundStmt() = default;

void CompoundStmt::addStatement(std::unique_ptr<Statement> stmt) {
    if (stmt) {
        m_statements.push_back(std::move(stmt));
    }
}

std::string CompoundStmt::toString() const {
    return "CompoundStmt (" + std::to_string(m_statements.size()) + " statements)";
}

