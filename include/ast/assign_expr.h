#pragma once

#include "ast/expression.h"
#include <memory>
#include <string>

// =============================================================================
// AssignExpr — AST Node for Assignment Expressions (Day 17)
// =============================================================================
//
// Represents an assignment operation:
//   Simple assignment:   x = 10
//   Compound assignment: x += 5, x -= 2, x *= 3, x /= 4
//
// Owns its left-hand (target) and right-hand (value) operands via std::unique_ptr<Expression>.
//
class AssignExpr : public Expression {
public:
    AssignExpr(std::string op,
               std::unique_ptr<Expression> left,
               std::unique_ptr<Expression> right,
               SourceLocation loc = {});
    ~AssignExpr() override;

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

using AssignExpression = AssignExpr;
using AssignmentExpression = AssignExpr;
