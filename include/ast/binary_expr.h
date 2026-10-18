#pragma once

#include "ast/expression.h"
#include <memory>
#include <string>

// =============================================================================
// BinaryExpr — AST Node for Binary Operator Expressions (Day 14)
// =============================================================================
//
// Represents a binary operation such as "a + b", "x * y", etc.
// Owns its left and right operands via std::unique_ptr<Expression>.
//
class BinaryExpr : public Expression {
public:
    BinaryExpr(std::string op,
               std::unique_ptr<Expression> left,
               std::unique_ptr<Expression> right,
               SourceLocation loc = {});
    ~BinaryExpr() override;

    const std::string& op() const { return m_op; }
    const std::string& getOp() const { return m_op; }

    const Expression* left() const { return m_left.get(); }
    Expression* left() { return m_left.get(); }
    const Expression* getLeft() const { return m_left.get(); }
    Expression* getLeft() { return m_left.get(); }

    const Expression* right() const { return m_right.get(); }
    Expression* right() { return m_right.get(); }
    const Expression* getRight() const { return m_right.get(); }
    Expression* getRight() { return m_right.get(); }

    std::unique_ptr<Expression> takeLeft() { return std::move(m_left); }
    std::unique_ptr<Expression> takeRight() { return std::move(m_right); }

    std::string toString() const override;

private:
    std::string m_op;
    std::unique_ptr<Expression> m_left;
    std::unique_ptr<Expression> m_right;
};

using BinaryExpression = BinaryExpr;
