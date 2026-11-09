#include "ast/var_decl.h"
#include <utility>

VarDecl::VarDecl(std::shared_ptr<Type> type,
                 std::string name,
                 std::unique_ptr<Expression> initializer,
                 SourceLocation loc)
    : Statement(loc)
    , m_type(std::move(type))
    , m_name(std::move(name))
    , m_initializer(std::move(initializer)) {
}

VarDecl::~VarDecl() = default;

std::unique_ptr<Expression> VarDecl::takeInitializer() {
    return std::move(m_initializer);
}

void VarDecl::setInitializer(std::unique_ptr<Expression> initializer) {
    m_initializer = std::move(initializer);
}

std::string VarDecl::toString() const {
    std::string typeStr = m_type ? m_type->toString() : "<unknown>";
    std::string str = "VarDecl: " + typeStr + " " + m_name;
    if (m_initializer) {
        str += " = " + m_initializer->toString();
    }
    return str;
}
