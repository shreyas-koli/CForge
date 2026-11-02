#include "ast/continue_stmt.h"

ContinueStmt::ContinueStmt(SourceLocation loc)
    : Statement(loc) {}

ContinueStmt::~ContinueStmt() = default;

std::string ContinueStmt::toString() const {
    return "ContinueStmt";
}

