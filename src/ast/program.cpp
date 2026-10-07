#include "ast/program.h"
#include <utility>

Program::Program(SourceLocation loc) : ASTNode(loc) {}

Program::~Program() = default;

void Program::addStatement(std::unique_ptr<Statement> stmt) {
    if (stmt) {
        m_statements.push_back(std::move(stmt));
    }
}

std::string Program::toString() const {
    return "Program (" + std::to_string(m_statements.size()) + " statements)";
}
