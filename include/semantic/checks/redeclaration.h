#pragma once

#include "ast/ast_node.h"
#include "ast/compound_stmt.h"
#include "ast/function.h"
#include "ast/program.h"
#include "ast/statement.h"
#include "ast/var_decl.h"
#include "ast/variable_declaration.h"
#include "lexer/token.h"
#include "semantic/symbol_table.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "semantic/diagnostics.h"

// =============================================================================
// RedeclarationChecker — Day 47 Duplicate Declaration Detection
// =============================================================================
//
// Detects invalid redeclarations within the same lexical scope while permitting
// valid shadowing of outer declarations in nested scopes.
//
// Leverages Day 46 SymbolTable scope management (pushScope, popScope,
// lookupCurrentScope) to enforce C17 scope rules:
//   - Duplicate declarations in the same scope are rejected with diagnostics.
//   - Inner scopes (blocks/functions) legally shadow outer declarations.
//   - Identifiers in separate, non-overlapping functions/blocks do not conflict.
//
class RedeclarationChecker {
public:
    // Constructs a checker managing its own internal SymbolTable
    RedeclarationChecker();

    // Constructs a checker using a shared external SymbolTable
    explicit RedeclarationChecker(SymbolTable& table);

    ~RedeclarationChecker() = default;

    // Check entry points
    bool check(const Program& program);
    bool check(const ASTNode& node);
    bool check(const Statement& stmt);
    bool check(const CompoundStmt& compound);
    bool check(const Function& fn);
    bool check(const VarDecl& varDecl);
    bool check(const VariableDeclaration& varDecl);

    // Error and diagnostic queries
    bool hasErrors() const noexcept {
        for (const auto& d : m_diagnostics) {
            if (d.isError()) return true;
        }
        return false;
    }
    std::size_t errorCount() const noexcept {
        std::size_t count = 0;
        for (const auto& d : m_diagnostics) {
            if (d.isError()) ++count;
        }
        return count;
    }
    const std::vector<Diagnostic>& diagnostics() const noexcept { return m_diagnostics; }
    const std::vector<Diagnostic>& errors() const noexcept { return m_diagnostics; }

    bool hasWarnings() const noexcept {
        for (const auto& d : m_diagnostics) {
            if (d.isWarning()) return true;
        }
        return false;
    }
    std::size_t warningCount() const noexcept {
        std::size_t count = 0;
        for (const auto& d : m_diagnostics) {
            if (d.isWarning()) ++count;
        }
        return count;
    }
    void reportWarning(const std::string& message, SourceLocation loc = {}, const std::string& id = "");
    void reportError(const std::string& message, SourceLocation loc = {}, const std::string& id = "");

    // State reset
    void clear();

    // Symbol table access
    const SymbolTable& symbolTable() const noexcept { return m_table; }
    SymbolTable& symbolTable() noexcept { return m_table; }

private:
    bool checkProgram(const Program& program);
    bool checkStatement(const Statement& stmt);
    bool checkCompoundStmt(const CompoundStmt& compound);
    bool checkFunction(const Function& fn);
    bool checkVarDecl(const VarDecl& varDecl);
    bool checkVariableDeclaration(const VariableDeclaration& varDecl);

    void reportDuplicate(const std::string& name,
                         SourceLocation loc,
                         const Symbol* existing);

    std::unique_ptr<SymbolTable> m_ownedTable;
    SymbolTable& m_table;
    std::vector<Diagnostic> m_diagnostics;
};

// =============================================================================
// Convenience Checking Functions
// =============================================================================
std::vector<Diagnostic> checkRedeclarations(const Program& program);
std::vector<Diagnostic> checkRedeclarations(const Program& program, SymbolTable& table);
std::vector<Diagnostic> checkRedeclarations(const Statement& stmt);
std::vector<Diagnostic> checkRedeclarations(const Statement& stmt, SymbolTable& table);
