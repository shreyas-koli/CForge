#pragma once

#include "ast/expression.h"
#include "ast/identifier_expression.h"

#include <memory>
#include <string>
#include <vector>

// =============================================================================
// CallExpr — AST Node for Function Call Expressions (Day 50)
// =============================================================================
//
// Represents a function call expression, including the callee expression
// (or callee function name) and a list of argument expressions.
//
// Examples:
//   add(10, 20)
//   display()
//   add(10, add(20, 30))
//
class CallExpr : public Expression {
public:
    // Constructor taking callee name as string and argument list
    CallExpr(std::string calleeName,
             std::vector<std::unique_ptr<Expression>> args,
             SourceLocation loc = {});

    // Constructor taking arbitrary callee expression and argument list
    CallExpr(std::unique_ptr<Expression> callee,
             std::vector<std::unique_ptr<Expression>> args,
             SourceLocation loc = {});

    ~CallExpr() override = default;

    // Callee accessors
    const Expression* callee() const noexcept { return m_callee.get(); }
    const Expression* getCallee() const noexcept { return m_callee.get(); }
    const std::string& calleeName() const noexcept { return m_calleeName; }
    const std::string& getCalleeName() const noexcept { return m_calleeName; }

    // Argument accessors
    const std::vector<std::unique_ptr<Expression>>& arguments() const noexcept { return m_args; }
    const std::vector<std::unique_ptr<Expression>>& getArguments() const noexcept { return m_args; }
    std::size_t argumentCount() const noexcept { return m_args.size(); }
    std::size_t getArgumentCount() const noexcept { return m_args.size(); }

    const Expression* argument(std::size_t index) const noexcept {
        return index < m_args.size() ? m_args[index].get() : nullptr;
    }
    const Expression* getArgument(std::size_t index) const noexcept {
        return index < m_args.size() ? m_args[index].get() : nullptr;
    }

    std::string toString() const override;

private:
    std::unique_ptr<Expression> m_callee;
    std::string m_calleeName;
    std::vector<std::unique_ptr<Expression>> m_args;
};
