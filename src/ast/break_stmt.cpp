#include "ast/break_stmt.h"

BreakStmt::BreakStmt(SourceLocation loc)
    : Statement(loc) {}

BreakStmt::~BreakStmt() = default;

std::string BreakStmt::toString() const {
    return "BreakStmt";
}

