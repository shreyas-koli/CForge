#include "ast/variable_declaration.h"
#include <utility>

VariableDeclaration::VariableDeclaration(std::string name,
                                         std::string type,
                                         std::unique_ptr<Expression> initializer,
                                         SourceLocation loc)
    : Statement(loc)
    , m_type(Type::fromKeyword(type))
    , m_typeName(std::move(type))
    , m_name(std::move(name))
    , m_initializer(std::move(initializer)) {
    if (!m_type && !m_typeName.empty()) {
        m_type = getPrimitiveType(m_typeName);
    }
}

VariableDeclaration::VariableDeclaration(std::shared_ptr<Type> type,
                                         std::string name,
                                         std::unique_ptr<Expression> initializer,
                                         SourceLocation loc)
    : Statement(loc)
    , m_type(std::move(type))
    , m_typeName(m_type ? m_type->toString() : "")
    , m_name(std::move(name))
    , m_initializer(std::move(initializer)) {}

VariableDeclaration::VariableDeclaration(std::string name,
                                         std::shared_ptr<Type> type,
                                         std::unique_ptr<Expression> initializer,
                                         SourceLocation loc)
    : Statement(loc)
    , m_type(std::move(type))
    , m_typeName(m_type ? m_type->toString() : "")
    , m_name(std::move(name))
    , m_initializer(std::move(initializer)) {}

VariableDeclaration::~VariableDeclaration() = default;

std::unique_ptr<Expression> VariableDeclaration::takeInitializer() {
    return std::move(m_initializer);
}

void VariableDeclaration::setInitializer(std::unique_ptr<Expression> initializer) {
    m_initializer = std::move(initializer);
}

std::string VariableDeclaration::toString() const {
    std::string str = "VariableDeclaration: " + m_typeName;
    if (!m_name.empty()) {
        str += " " + m_name;
    }
    if (m_initializer) {
        str += " = " + m_initializer->toString();
    }
    return str;
}
