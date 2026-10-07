#include "ast/function.h"
#include <utility>

Function::Function(std::string name, std::string returnType, SourceLocation loc)
    : Statement(loc)
    , m_name(std::move(name))
    , m_returnType(std::move(returnType)) {}

Function::~Function() = default;

void Function::addParameter(std::unique_ptr<VariableDeclaration> param) {
    if (param) {
        m_parameters.push_back(std::move(param));
    }
}

void Function::addBodyStatement(std::unique_ptr<Statement> stmt) {
    if (stmt) {
        m_body.push_back(std::move(stmt));
    }
}

std::string Function::toString() const {
    return "Function: " + m_returnType + " " + m_name + "()";
}
