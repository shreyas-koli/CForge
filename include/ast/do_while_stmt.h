#pragma once

#include "ast/expression.h"
#include "ast/statement.h"

#include <memory>
#include <string>

// =============================================================================
// DoWhileStmt — AST Node for 'do-while' Iteration Statements (Day 32)
// =============================================================================
//
// Represents a C 'do-while' loop:
//   iteration-statement:
//       'do' statement 'while' '(' expression ')' ';'
//
// Owns the loop body statement and the condition expression.
// Source location records the location of the 'do' keyword.
//
class DoWhileStmt : public Statement {
public:
    DoWhileStmt(std::unique_ptr<Statement> body,
                std::unique_ptr<Expression> condition,
                SourceLocation loc = {});
    ~DoWhileStmt() override;

    // Body accessors
    const Statement* body() const { return m_body.get(); }
    Statement* body() { return m_body.get(); }
    const Statement* getBody() const { return m_body.get(); }
    Statement* getBody() { return m_body.get(); }

    const Statement* bodyStmt() const { return m_body.get(); }
    Statement* bodyStmt() { return m_body.get(); }
    const Statement* getBodyStmt() const { return m_body.get(); }
    Statement* getBodyStmt() { return m_body.get(); }

    // Condition accessors
    const Expression* condition() const { return m_condition.get(); }
    Expression* condition() { return m_condition.get(); }
    const Expression* getCondition() const { return m_condition.get(); }
    Expression* getCondition() { return m_condition.get(); }

    // Move / take accessors
    std::unique_ptr<Statement> takeBody() { return std::move(m_body); }
    std::unique_ptr<Statement> takeBodyStmt() { return std::move(m_body); }
    std::unique_ptr<Expression> takeCondition() { return std::move(m_condition); }

    std::string toString() const override;

private:
    std::unique_ptr<Statement> m_body;
    std::unique_ptr<Expression> m_condition;
};

using DoWhileStatement = DoWhileStmt;

