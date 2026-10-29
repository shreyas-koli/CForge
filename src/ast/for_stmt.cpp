#include "ast/for_stmt.h"
#include <utility>

ForStmt::ForStmt(std::unique_ptr<Expression> initializer,
                 std::unique_ptr<Expression> condition,
                 std::unique_ptr<Expression> post,
                 std::unique_ptr<Statement> body,
                 SourceLocation loc)
    : Statement(loc)
    , m_initializer(std::move(initializer))
    , m_condition(std::move(condition))
    , m_post(std::move(post))
    , m_body(std::move(body)) {}

ForStmt::~ForStmt() = default;

std::string ForStmt::toString() const {
    std::string initStr = m_initializer ? m_initializer->toString() : "null";
    std::string condStr = m_condition ? m_condition->toString() : "null";
    std::string postStr = m_post ? m_post->toString() : "null";
    std::string bodyStr = m_body ? m_body->toString() : "null";
    return "ForStmt(" + initStr + ", " + condStr + ", " + postStr + ", " + bodyStr + ")";
}

