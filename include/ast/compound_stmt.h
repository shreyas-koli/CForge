#pragma once

#include "ast/statement.h"

#include <memory>
#include <string>
#include <vector>

// =============================================================================
// CompoundStmt — AST Node for Compound Statements / Blocks (Day 28)
// =============================================================================
//
// Represents a compound statement (block) in C:
//   compound-statement:
//       '{' statement-list? '}'
//
//   statement-list:
//       statement*
//
// Owns its child statements via std::vector<std::unique_ptr<Statement>>.
//
class CompoundStmt : public Statement {
public:
    explicit CompoundStmt(SourceLocation loc = {});
    explicit CompoundStmt(std::vector<std::unique_ptr<Statement>> statements,
                          SourceLocation loc = {});
    ~CompoundStmt() override;

    // Statement list accessors
    const std::vector<std::unique_ptr<Statement>>& statements() const {
        return m_statements;
    }
    std::vector<std::unique_ptr<Statement>>& statements() {
        return m_statements;
    }
    const std::vector<std::unique_ptr<Statement>>& getStatements() const {
        return m_statements;
    }
    std::vector<std::unique_ptr<Statement>>& getStatements() {
        return m_statements;
    }

    std::size_t statementCount() const { return m_statements.size(); }
    std::size_t size() const { return m_statements.size(); }
    bool empty() const { return m_statements.empty(); }

    void addStatement(std::unique_ptr<Statement> stmt);

    std::string toString() const override;

private:
    std::vector<std::unique_ptr<Statement>> m_statements;
};

using BlockStmt = CompoundStmt;
using CompoundStatement = CompoundStmt;
using BlockStatement = CompoundStmt;

