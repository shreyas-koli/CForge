#pragma once

#include "ast/expression.h"
#include "ast/statement.h"

#include <memory>
#include <string>

// =============================================================================
// WhileStmt — AST Node for 'while' Iteration Statements (Day 31)
// =============================================================================
//
// Represents a C 'while' loop:
//   iteration-statement:
//       'while' '(' expression ')' statement
//
// Owns the condition expression and the loop body statement.
// Source location records the location of the 'while' keyword.
//
class WhileStmt : public Statement {
public:
    WhileStmt(std::unique_ptr<Expression> condition,
              std::unique_ptr<Statement> body,
              SourceLocation loc = {});
    ~WhileStmt() override;

    // Condition accessors
    const Expression* condition() const { return m_condition.get(); }
    Expression* condition() { return m_condition.get(); }
    const Expression* getCondition() const { return m_condition.get(); }
    Expression* getCondition() { return m_condition.get(); }

    // Body accessors
    const Statement* body() const { return m_body.get(); }
    Statement* body() { return m_body.get(); }
    const Statement* getBody() const { return m_body.get(); }
    Statement* getBody() { return m_body.get(); }

    const Statement* bodyStmt() const { return m_body.get(); }
    Statement* bodyStmt() { return m_body.get(); }
    const Statement* getBodyStmt() const { return m_body.get(); }
    Statement* getBodyStmt() { return m_body.get(); }

    // Move / take accessors
    std::unique_ptr<Expression> takeCondition() { return std::move(m_condition); }
    std::unique_ptr<Statement> takeBody() { return std::move(m_body); }
    std::unique_ptr<Statement> takeBodyStmt() { return std::move(m_body); }

    std::string toString() const override;

private:
    std::unique_ptr<Expression> m_condition;
    std::unique_ptr<Statement> m_body;
};

using WhileStatement = WhileStmt;

