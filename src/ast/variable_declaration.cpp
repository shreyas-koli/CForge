#include "ast/variable_declaration.h"
#include <utility>

VariableDeclaration::VariableDeclaration(std::string name,
                                             std::string type,
                                             std::unique_ptr<Expression> initializer,
                                             SourceLocation loc)
    : Statement(loc)
    , m_name(std::move(name))
    , m_type(std::move(type))
    , m_initializer(std::move(initializer)) {}

VariableDeclaration::~VariableDeclaration() = default;

std::unique_ptr<Expression> VariableDeclaration::takeInitializer() {
    return std::move(m_initializer);
}

void VariableDeclaration::setInitializer(std::unique_ptr<Expression> initializer) {
    m_initializer = std::move(initializer);
}

std::string VariableDeclaration::toString() const {
    std::string str = "VariableDeclaration: " + m_type + " " + m_name;
    if (m_initializer) {
        str += " = " + m_initializer->toString();
    }
    return str;
}
