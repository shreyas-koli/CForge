#pragma once

#include "ast/ast_node.h"
#include "ast/compound_stmt.h"
#include "ast/do_while_stmt.h"
#include "ast/expression.h"
#include "ast/for_stmt.h"
#include "ast/function.h"
#include "ast/if_stmt.h"
#include "ast/program.h"
#include "ast/return_stmt.h"
#include "ast/statement.h"
#include "ast/var_decl.h"
#include "ast/variable_declaration.h"
#include "ast/while_stmt.h"
#include "semantic/diagnostics.h"
#include "semantic/symbol_table.h"
#include "semantic/type_checker.h"
#include "types/conversions.h"
#include "types/type.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

// =============================================================================
// ReturnChecker — Day 51 Return Statement Validation
// =============================================================================
//
// Validates return statements against the declared return type of their enclosing
// functions in the AST:
//   1. Tracks the enclosing function context during AST traversal.
//   2. Enforces void vs non-void return rules:
//      - Void functions cannot return values (Rule D), but 'return;' is valid (Rule E).
//      - Non-void functions must return a value (Rule F); empty 'return;' is diagnosed.
//   3. Infers the type of return expressions using Day 49 TypeChecker.
//   4. Validates return value type compatibility using Day 39 implicit conversions:
//      - Identical types are accepted (Rule A).
//      - Compatible implicit numeric conversions are accepted (Rule B).
//      - Incompatible types and string literal returns to non-string types are diagnosed (Rule C).
//   5. Avoids cascading diagnostics when expression inference or identifier lookup fails.
//   6. Handles nested compound statements, conditionals, and loops while maintaining
//      the correct enclosing function context.
//   7. Provides structural missing-return analysis for non-void function definitions.
//
class ReturnChecker {
public:
    // Constructs a checker managing its own internal SymbolTable
    ReturnChecker();

    // Constructs a checker using a shared external SymbolTable
    explicit ReturnChecker(SymbolTable& table);

    ~ReturnChecker() = default;

    // Check entry points
    bool check(const Program& program);
    bool check(const ASTNode& node);
    bool check(const Function& fn);
    bool check(const Statement& stmt);
    bool check(const CompoundStmt& compound);
    bool check(const IfStmt& ifStmt);
    bool check(const WhileStmt& whileStmt);
    bool check(const DoWhileStmt& doWhileStmt);
    bool check(const ForStmt& forStmt);
    bool check(const ReturnStmt& ret);
    bool check(const VarDecl& varDecl);
    bool check(const VariableDeclaration& varDecl);

    // Direct return validation helpers
    bool checkReturn(const ReturnStmt& ret,
                     const Type& expectedReturnType,
                     const std::string& fnName = "");
    bool checkReturn(const ReturnStmt& ret,
                     const std::shared_ptr<Type>& expectedReturnType,
                     const std::string& fnName = "");
    bool checkReturn(const ReturnStmt& ret,
                     const Function& fn);

    // Missing-return detection (structural)
    static bool hasReturn(const Function& fn);
    static bool hasReturn(const Statement& stmt);
    static bool hasReturn(const CompoundStmt& compound);
    bool checkMissingReturn(const Function& fn);

    // State management
    void clear();

    // Symbol table access
    const SymbolTable& symbolTable() const noexcept { return m_table; }
    SymbolTable& symbolTable() noexcept { return m_table; }

    // Enclosing function access
    const Function* currentFunction() const noexcept { return m_currentFunction; }

    // Diagnostic queries
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

private:
    bool checkProgram(const Program& program);
    bool checkFunction(const Function& fn);
    bool checkStatement(const Statement& stmt);
    bool checkCompoundStmt(const CompoundStmt& compound);
    bool checkIfStmt(const IfStmt& ifStmt);
    bool checkWhileStmt(const WhileStmt& whileStmt);
    bool checkDoWhileStmt(const DoWhileStmt& doWhileStmt);
    bool checkForStmt(const ForStmt& forStmt);
    bool checkVarDecl(const VarDecl& varDecl);
    bool checkVariableDeclaration(const VariableDeclaration& varDecl);
    bool checkReturnStmt(const ReturnStmt& ret);

    bool validateReturn(const ReturnStmt& ret,
                        const Type& expectedType,
                        const std::string& fnName);

    std::unique_ptr<SymbolTable> m_ownedTable;
    SymbolTable& m_table;
    TypeChecker m_typeChecker;
    const Function* m_currentFunction{nullptr};
    std::vector<Diagnostic> m_diagnostics;
};

// =============================================================================
// Convenience Checking Functions
// =============================================================================
std::vector<Diagnostic> checkReturns(const Program& program);
std::vector<Diagnostic> checkReturns(const Program& program, SymbolTable& table);
std::vector<Diagnostic> checkReturns(const Function& fn);
std::vector<Diagnostic> checkReturns(const Function& fn, SymbolTable& table);
std::vector<Diagnostic> checkReturns(const ReturnStmt& ret, const Type& expectedType, const std::string& fnName = "");
std::vector<Diagnostic> checkReturns(const ReturnStmt& ret, const Type& expectedType, SymbolTable& table, const std::string& fnName = "");
