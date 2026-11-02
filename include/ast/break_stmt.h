#pragma once

#include "ast/statement.h"
#include <string>

// =============================================================================
// BreakStmt — AST Node for 'break' Jump Statements (Day 34)
// =============================================================================
//
// Represents a C 'break' statement:
//   jump-statement:
//       'break' ';'
//
// Stores the source location of the 'break' keyword.
//
class BreakStmt : public Statement {
public:
    explicit BreakStmt(SourceLocation loc = {});
    ~BreakStmt() override;

    std::string toString() const override;
};

using BreakStatement = BreakStmt;

