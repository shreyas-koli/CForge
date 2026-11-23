#include "ast/call_expr.h"
#include <sstream>

CallExpr::CallExpr(std::string calleeName,
                   std::vector<std::unique_ptr<Expression>> args,
                   SourceLocation loc)
    : Expression(loc)
    , m_callee(std::make_unique<IdentifierExpression>(calleeName, loc))
    , m_calleeName(std::move(calleeName))
    , m_args(std::move(args))
{}

CallExpr::CallExpr(std::unique_ptr<Expression> callee,
                   std::vector<std::unique_ptr<Expression>> args,
                   SourceLocation loc)
    : Expression(loc)
    , m_callee(std::move(callee))
    , m_args(std::move(args))
{
    if (const auto* id = dynamic_cast<const IdentifierExpression*>(m_callee.get())) {
        m_calleeName = id->name();
    }
}

std::string CallExpr::toString() const {
    std::ostringstream ss;
    ss << "CallExpr(" << (!m_calleeName.empty() ? m_calleeName : (m_callee ? m_callee->toString() : "")) << "(";
    for (std::size_t i = 0; i < m_args.size(); ++i) {
        if (i > 0) ss << ", ";
        if (m_args[i]) ss << m_args[i]->toString();
    }
    ss << "))";
    return ss.str();
}
