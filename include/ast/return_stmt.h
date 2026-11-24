#pragma once

#include "ast/expression.h"
#include "ast/statement.h"

#include <memory>
#include <string>

// =============================================================================
// ReturnStmt — AST Node for 'return' Jump Statements (Day 51)
// =============================================================================
//
// Represents a C 'return' statement:
//   jump-statement:
//       'return' expression? ';'
//
// Stores the optional return value expression and the source location of the
// 'return' keyword.
//
class ReturnStmt : public Statement {
public:
    explicit ReturnStmt(std::unique_ptr<Expression> expr = nullptr, SourceLocation loc = {});
    explicit ReturnStmt(SourceLocation loc);
    ~ReturnStmt() override;

    // Expression accessors
    bool hasExpression() const noexcept { return m_expr != nullptr; }
    bool hasValue() const noexcept { return m_expr != nullptr; }
    const Expression* expression() const noexcept { return m_expr.get(); }
    Expression* expression() noexcept { return m_expr.get(); }
    const Expression* getExpression() const noexcept { return m_expr.get(); }
    Expression* getExpression() noexcept { return m_expr.get(); }

    std::unique_ptr<Expression> takeExpression() noexcept { return std::move(m_expr); }
    void setExpression(std::unique_ptr<Expression> expr) noexcept { m_expr = std::move(expr); }

    std::string toString() const override;

private:
    std::unique_ptr<Expression> m_expr;
};

using ReturnStatement = ReturnStmt;
