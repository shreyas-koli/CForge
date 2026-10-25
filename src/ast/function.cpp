#include "ast/function.h"
#include <utility>

Function::Function(std::string name,
                   std::string returnType,
                   SourceLocation loc,
                   bool isDefinition)
    : Statement(loc)
    , m_name(std::move(name))
    , m_returnType(std::move(returnType))
    , m_isDefinition(isDefinition) {}

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
    std::string str = "Function: " + m_returnType + " " + m_name + "(";
    for (std::size_t i = 0; i < m_parameters.size(); ++i) {
        if (i > 0) {
            str += ", ";
        }
        str += m_parameters[i]->type() + " " + m_parameters[i]->name();
    }
    str += ")";
    return str;
}
