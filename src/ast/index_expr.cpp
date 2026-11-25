#include "ast/index_expr.h"
#include <utility>

IndexExpr::IndexExpr(std::unique_ptr<Expression> base,
                     std::unique_ptr<Expression> index,
                     SourceLocation loc)
    : Expression(loc)
    , m_base(std::move(base))
    , m_index(std::move(index)) {}

IndexExpr::~IndexExpr() = default;

std::string IndexExpr::toString() const {
    std::string bStr = m_base ? m_base->toString() : "null";
    std::string iStr = m_index ? m_index->toString() : "null";
    return "IndexExpr(" + bStr + "[" + iStr + "])";
}
