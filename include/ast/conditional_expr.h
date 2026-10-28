#pragma once

#include "ast/expression.h"
#include <memory>
#include <string>

// =============================================================================
// ConditionalExpr — AST Node for Ternary Conditional Expressions (Day 26)
// =============================================================================
//
// Represents C's ternary conditional operator:
//   condition ? thenExpr : elseExpr
//
// Owns its condition, true expression, and false expression via std::unique_ptr<Expression>.
//
class ConditionalExpr : public Expression {
public:
    ConditionalExpr(std::unique_ptr<Expression> condition,
                    std::unique_ptr<Expression> thenExpr,
                    std::unique_ptr<Expression> elseExpr,
                    SourceLocation loc = {});
    ~ConditionalExpr() override;

    const Expression* condition() const { return m_condition.get(); }
    Expression* condition() { return m_condition.get(); }
    const Expression* getCondition() const { return m_condition.get(); }
    Expression* getCondition() { return m_condition.get(); }

    const Expression* thenExpr() const { return m_thenExpr.get(); }
    Expression* thenExpr() { return m_thenExpr.get(); }
    const Expression* getThenExpr() const { return m_thenExpr.get(); }
    Expression* getThenExpr() { return m_thenExpr.get(); }

    const Expression* elseExpr() const { return m_elseExpr.get(); }
    Expression* elseExpr() { return m_elseExpr.get(); }
    const Expression* getElseExpr() const { return m_elseExpr.get(); }
    Expression* getElseExpr() { return m_elseExpr.get(); }

    // Aliases using true/false terminology
    const Expression* trueExpr() const { return m_thenExpr.get(); }
    Expression* trueExpr() { return m_thenExpr.get(); }
    const Expression* getTrueExpr() const { return m_thenExpr.get(); }
    Expression* getTrueExpr() { return m_thenExpr.get(); }

    const Expression* falseExpr() const { return m_elseExpr.get(); }
    Expression* falseExpr() { return m_elseExpr.get(); }
    const Expression* getFalseExpr() const { return m_elseExpr.get(); }
    Expression* getFalseExpr() { return m_elseExpr.get(); }

    std::unique_ptr<Expression> takeCondition() { return std::move(m_condition); }
    std::unique_ptr<Expression> takeThenExpr() { return std::move(m_thenExpr); }
    std::unique_ptr<Expression> takeElseExpr() { return std::move(m_elseExpr); }
    std::unique_ptr<Expression> takeTrueExpr() { return std::move(m_thenExpr); }
    std::unique_ptr<Expression> takeFalseExpr() { return std::move(m_elseExpr); }

    std::string toString() const override;

private:
    std::unique_ptr<Expression> m_condition;
    std::unique_ptr<Expression> m_thenExpr;
    std::unique_ptr<Expression> m_elseExpr;
};

using ConditionalExpression = ConditionalExpr;
using TernaryExpr = ConditionalExpr;
using TernaryExpression = ConditionalExpr;
