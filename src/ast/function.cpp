#include "ast/function.h"
#include <utility>

Function::Function(std::string name,
                   std::shared_ptr<Type> returnType,
                   SourceLocation loc,
                   bool isDefinition,
                   ParameterListKind paramListKind,
                   StorageClass storageClass)
    : Statement(loc)
    , m_name(std::move(name))
    , m_returnType(std::move(returnType))
    , m_returnTypeName(m_returnType ? m_returnType->toString() : "")
    , m_isDefinition(isDefinition)
    , m_paramListKind(paramListKind)
    , m_storageClass(storageClass) {}

Function::Function(std::string name,
                   std::string returnType,
                   SourceLocation loc,
                   bool isDefinition,
                   ParameterListKind paramListKind,
                   StorageClass storageClass)
    : Statement(loc)
    , m_name(std::move(name))
    , m_returnType(Type::fromKeyword(returnType))
    , m_returnTypeName(std::move(returnType))
    , m_isDefinition(isDefinition)
    , m_paramListKind(paramListKind)
    , m_storageClass(storageClass) {
    if (!m_returnType && !m_returnTypeName.empty()) {
        m_returnType = getPrimitiveType(m_returnTypeName);
    }
}

Function::~Function() = default;

void Function::setReturnType(std::string returnType) {
    m_returnTypeName = std::move(returnType);
    m_returnType = Type::fromKeyword(m_returnTypeName);
    if (!m_returnType && !m_returnTypeName.empty()) {
        m_returnType = getPrimitiveType(m_returnTypeName);
    }
}

void Function::setReturnType(std::shared_ptr<Type> returnType) {
    m_returnType = std::move(returnType);
    m_returnTypeName = m_returnType ? m_returnType->toString() : "";
}

void Function::addParameter(std::unique_ptr<VariableDeclaration> param) {
    if (param) {
        m_paramListKind = ParameterListKind::Typed;
        m_parameters.push_back(std::move(param));
    }
}

void Function::addBodyStatement(std::unique_ptr<Statement> stmt) {
    if (stmt) {
        m_body.push_back(std::move(stmt));
    }
}

std::string Function::toString() const {
    std::string retStr = m_returnType ? m_returnType->toString() : m_returnTypeName;
    std::string prefix;
    if (m_storageClass != StorageClass::None) {
        prefix = std::string(storageClassToString(m_storageClass)) + " ";
    }
    std::string str = "Function: " + prefix + retStr + " " + m_name + "(";
    if (m_paramListKind == ParameterListKind::Void) {
        str += "void";
    } else {
        for (std::size_t i = 0; i < m_parameters.size(); ++i) {
            if (i > 0) {
                str += ", ";
            }
            str += m_parameters[i]->typeName();
            if (!m_parameters[i]->name().empty()) {
                str += " " + m_parameters[i]->name();
            }
        }
    }
    str += ")";
    return str;
}
