#pragma once

#include "ast/expression.h"
#include "ast/statement.h"

#include <memory>
#include <string>

// =============================================================================
// ForStmt — AST Node for 'for' Iteration Statements (Day 33)
// =============================================================================
//
// Represents a C 'for' loop:
//   iteration-statement:
//       'for' '(' expression? ';' expression? ';' expression? ')' statement
//
// Owns the optional initializer, condition, and post expressions, and the loop body.
//
// Omitted Clause Convention:
//   An omitted expression clause (e.g. in 'for (;;)') is represented by nullptr.
//   Callers may check hasInitializer(), hasCondition(), or hasPost() before accessing.
//
// Source location records the location of the 'for' keyword.
//
class ForStmt : public Statement {
public:
    ForStmt(std::unique_ptr<Expression> initializer,
            std::unique_ptr<Expression> condition,
            std::unique_ptr<Expression> post,
            std::unique_ptr<Statement> body,
            SourceLocation loc = {});
    ~ForStmt() override;

    // Initializer clause accessors
    bool hasInitializer() const { return m_initializer != nullptr; }
    bool hasInit() const { return m_initializer != nullptr; }
    const Expression* initializer() const { return m_initializer.get(); }
    Expression* initializer() { return m_initializer.get(); }
    const Expression* getInitializer() const { return m_initializer.get(); }
    Expression* getInitializer() { return m_initializer.get(); }
    const Expression* init() const { return m_initializer.get(); }
    Expression* init() { return m_initializer.get(); }
    const Expression* getInit() const { return m_initializer.get(); }
    Expression* getInit() { return m_initializer.get(); }
    std::unique_ptr<Expression> takeInitializer() { return std::move(m_initializer); }
    std::unique_ptr<Expression> takeInit() { return std::move(m_initializer); }

    // Condition clause accessors
    bool hasCondition() const { return m_condition != nullptr; }
    const Expression* condition() const { return m_condition.get(); }
    Expression* condition() { return m_condition.get(); }
    const Expression* getCondition() const { return m_condition.get(); }
    Expression* getCondition() { return m_condition.get(); }
    std::unique_ptr<Expression> takeCondition() { return std::move(m_condition); }

    // Post-expression clause accessors
    bool hasPost() const { return m_post != nullptr; }
    bool hasPostExpr() const { return m_post != nullptr; }
    const Expression* post() const { return m_post.get(); }
    Expression* post() { return m_post.get(); }
    const Expression* getPost() const { return m_post.get(); }
    Expression* getPost() { return m_post.get(); }
    const Expression* postExpr() const { return m_post.get(); }
    Expression* postExpr() { return m_post.get(); }
    const Expression* getPostExpr() const { return m_post.get(); }
    Expression* getPostExpr() { return m_post.get(); }
    std::unique_ptr<Expression> takePost() { return std::move(m_post); }
    std::unique_ptr<Expression> takePostExpr() { return std::move(m_post); }

    // Body accessors
    const Statement* body() const { return m_body.get(); }
    Statement* body() { return m_body.get(); }
    const Statement* getBody() const { return m_body.get(); }
    Statement* getBody() { return m_body.get(); }
    const Statement* bodyStmt() const { return m_body.get(); }
    Statement* bodyStmt() { return m_body.get(); }
    const Statement* getBodyStmt() const { return m_body.get(); }
    Statement* getBodyStmt() { return m_body.get(); }
    std::unique_ptr<Statement> takeBody() { return std::move(m_body); }
    std::unique_ptr<Statement> takeBodyStmt() { return std::move(m_body); }

    std::string toString() const override;

private:
    std::unique_ptr<Expression> m_initializer;
    std::unique_ptr<Expression> m_condition;
    std::unique_ptr<Expression> m_post;
    std::unique_ptr<Statement> m_body;
};

using ForStatement = ForStmt;

