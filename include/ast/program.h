#pragma once

#include "ast/ast_node.h"
#include "ast/statement.h"

#include <memory>
#include <vector>

// =============================================================================
// Program — AST Root Node representing a C Translation Unit
// =============================================================================
//
// Contains top-level declarations and statements that make up a translation unit.
// Owns all child nodes using std::unique_ptr.
//
class Program : public ASTNode {
public:
    explicit Program(SourceLocation loc = {});
    ~Program() override;

    // Add top-level statements / declarations
    void addStatement(std::unique_ptr<Statement> stmt);

    // Accessors
    const std::vector<std::unique_ptr<Statement>>& statements() const {
        return m_statements;
    }

    std::size_t statementCount() const { return m_statements.size(); }

    std::string toString() const override;

private:
    std::vector<std::unique_ptr<Statement>> m_statements;
};
