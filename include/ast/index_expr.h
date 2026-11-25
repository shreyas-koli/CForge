#pragma once

#include "ast/expression.h"
#include <memory>
#include <string>

// =============================================================================
// IndexExpr — AST Node for Array Subscript Expressions (Day 55)
// =============================================================================
//
// Represents an array subscript expression:
//   base[index]
//
// Examples:
//   numbers[0]
//   numbers[5]
//   numbers[i]
//   numbers[i + 1]
//   numbers[i][j]
//
class IndexExpr : public Expression {
public:
    IndexExpr(std::unique_ptr<Expression> base,
              std::unique_ptr<Expression> index,
              SourceLocation loc = {});
    ~IndexExpr() override;

    // Base expression accessors
    const Expression* base() const noexcept { return m_base.get(); }
    Expression* base() noexcept { return m_base.get(); }
    const Expression* getBase() const noexcept { return m_base.get(); }
    Expression* getBase() noexcept { return m_base.get(); }
    const Expression& baseRef() const noexcept { return *m_base; }
    Expression& baseRef() noexcept { return *m_base; }

    // Index expression accessors
    const Expression* index() const noexcept { return m_index.get(); }
    Expression* index() noexcept { return m_index.get(); }
    const Expression* getIndex() const noexcept { return m_index.get(); }
    Expression* getIndex() noexcept { return m_index.get(); }
    const Expression& indexRef() const noexcept { return *m_index; }
    Expression& indexRef() noexcept { return *m_index; }

    // Ownership transfer helpers
    std::unique_ptr<Expression> takeBase() noexcept { return std::move(m_base); }
    std::unique_ptr<Expression> takeIndex() noexcept { return std::move(m_index); }

    std::string toString() const override;

private:
    std::unique_ptr<Expression> m_base;
    std::unique_ptr<Expression> m_index;
};

using IndexExpression = IndexExpr;
