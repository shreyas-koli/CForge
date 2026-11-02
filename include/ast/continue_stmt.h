#pragma once

#include "ast/statement.h"
#include <string>

// =============================================================================
// ContinueStmt — AST Node for 'continue' Jump Statements (Day 34)
// =============================================================================
//
// Represents a C 'continue' statement:
//   jump-statement:
//       'continue' ';'
//
// Stores the source location of the 'continue' keyword.
//
class ContinueStmt : public Statement {
public:
    explicit ContinueStmt(SourceLocation loc = {});
    ~ContinueStmt() override;

    std::string toString() const override;
};

using ContinueStatement = ContinueStmt;

