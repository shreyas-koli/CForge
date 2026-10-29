#pragma once

#include "ast/expression.h"
#include "ast/statement.h"

#include <memory>
#include <string>

// =============================================================================
// IfStmt — AST Node for 'if' Selection Statements (Day 29 & Day 30)
// =============================================================================
//
// Represents a C 'if' statement with an optional 'else' branch:
//   selection-statement:
//       'if' '(' expression ')' statement
//       'if' '(' expression ')' statement 'else' statement
//
// Owns the condition expression, then-branch statement, and optional else-branch.
// Source location records the location of the 'if' keyword.
//
class IfStmt : public Statement {
public:
    IfStmt(std::unique_ptr<Expression> condition,
           std::unique_ptr<Statement> thenBranch,
           SourceLocation loc = {});
    IfStmt(std::unique_ptr<Expression> condition,
           std::unique_ptr<Statement> thenBranch,
           std::unique_ptr<Statement> elseBranch,
           SourceLocation loc = {});
    ~IfStmt() override;

    // Condition accessors
    const Expression* condition() const { return m_condition.get(); }
    Expression* condition() { return m_condition.get(); }
    const Expression* getCondition() const { return m_condition.get(); }
    Expression* getCondition() { return m_condition.get(); }

    // Then-branch accessors
    const Statement* thenBranch() const { return m_thenBranch.get(); }
    Statement* thenBranch() { return m_thenBranch.get(); }
    const Statement* getThenBranch() const { return m_thenBranch.get(); }
    Statement* getThenBranch() { return m_thenBranch.get(); }

    const Statement* thenStmt() const { return m_thenBranch.get(); }
    Statement* thenStmt() { return m_thenBranch.get(); }
    const Statement* getThenStmt() const { return m_thenBranch.get(); }
    Statement* getThenStmt() { return m_thenBranch.get(); }

    // Else-branch accessors (Day 30)
    bool hasElse() const { return m_elseBranch != nullptr; }
    bool hasElseBranch() const { return m_elseBranch != nullptr; }

    const Statement* elseBranch() const { return m_elseBranch.get(); }
    Statement* elseBranch() { return m_elseBranch.get(); }
    const Statement* getElseBranch() const { return m_elseBranch.get(); }
    Statement* getElseBranch() { return m_elseBranch.get(); }

    const Statement* elseStmt() const { return m_elseBranch.get(); }
    Statement* elseStmt() { return m_elseBranch.get(); }
    const Statement* getElseStmt() const { return m_elseBranch.get(); }
    Statement* getElseStmt() { return m_elseBranch.get(); }

    // Move / take accessors
    std::unique_ptr<Expression> takeCondition() { return std::move(m_condition); }
    std::unique_ptr<Statement> takeThenBranch() { return std::move(m_thenBranch); }
    std::unique_ptr<Statement> takeThenStmt() { return std::move(m_thenBranch); }
    std::unique_ptr<Statement> takeElseBranch() { return std::move(m_elseBranch); }
    std::unique_ptr<Statement> takeElseStmt() { return std::move(m_elseBranch); }

    std::string toString() const override;

private:
    std::unique_ptr<Expression> m_condition;
    std::unique_ptr<Statement> m_thenBranch;
    std::unique_ptr<Statement> m_elseBranch;
};

using IfStatement = IfStmt;

