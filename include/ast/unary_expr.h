#pragma once

#include "ast/expression.h"
#include <memory>
#include <string>

// =============================================================================
// UnaryExpr — AST Node for Prefix Unary Expressions (Day 16)
// =============================================================================
//
// Represents a prefix unary operation such as:
//   -x   (unary minus)
//   +x   (unary plus)
//   !x   (logical NOT)
//   ~x   (bitwise NOT)
//   ++x  (prefix increment)
//   --x  (prefix decrement)
//
// Owns its operand via std::unique_ptr<Expression>.
//
class UnaryExpr : public Expression {
public:
    UnaryExpr(std::string op,
              std::unique_ptr<Expression> operand,
              SourceLocation loc = {});
    ~UnaryExpr() override;

    const std::string& op() const { return m_op; }
    const std::string& getOp() const { return m_op; }

    const Expression* operand() const { return m_operand.get(); }
    Expression* operand() { return m_operand.get(); }
    const Expression* getOperand() const { return m_operand.get(); }
    Expression* getOperand() { return m_operand.get(); }

    std::unique_ptr<Expression> takeOperand() { return std::move(m_operand); }

    std::string toString() const override;

private:
    std::string m_op;
    std::unique_ptr<Expression> m_operand;
};

using UnaryExpression = UnaryExpr;
