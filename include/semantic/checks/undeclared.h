#pragma once

#include "ast/assign_expr.h"
#include "ast/ast_node.h"
#include "ast/binary_expr.h"
#include "ast/call_expr.h"
#include "ast/compound_stmt.h"
#include "ast/conditional_expr.h"
#include "ast/do_while_stmt.h"
#include "ast/expression.h"
#include "ast/for_stmt.h"
#include "ast/function.h"
#include "ast/identifier_expression.h"
#include "ast/if_stmt.h"
#include "ast/literal_expression.h"
#include "ast/program.h"
#include "ast/return_stmt.h"
#include "ast/statement.h"
#include "ast/unary_expr.h"
#include "ast/var_decl.h"
#include "ast/variable_declaration.h"
#include "ast/while_stmt.h"
#include "lexer/token.h"
#include "semantic/checks/redeclaration.h"
#include "semantic/symbol_table.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

// =============================================================================
// UndeclaredChecker — Day 48 Undefined Identifier Detection
// =============================================================================
//
// Detects uses of identifiers for which no matching declaration exists in any
// accessible lexical scope (innermost to outermost/global).
//
// Leverages Day 45–46 SymbolTable scope management (lookup, pushScope, popScope)
// to enforce C17 identifier visibility rules:
//   - Identifiers used in expressions must have a preceding declaration in the
//     current or an enclosing lexical scope.
//   - Variable declarations introduce names into the symbol table and their
//     initializers are validated for identifier references.
//   - When a block scope exits, its local declarations are removed from visibility;
//     any subsequent use produces an undeclared-identifier diagnostic.
//   - Declarations inside inner blocks legally shadow outer declarations without
//     error.
//   - Global declarations and previously declared functions are accessible in all
//     enclosed scopes.
//
class UndeclaredChecker {
public:
    // Constructs a checker managing its own internal SymbolTable
    UndeclaredChecker();

    // Constructs a checker using a shared external SymbolTable
    explicit UndeclaredChecker(SymbolTable& table);

    ~UndeclaredChecker() = default;

    // Check entry points
    bool check(const Program& program);
    bool check(const ASTNode& node);
    bool check(const Statement& stmt);
    bool check(const Expression& expr);
    bool check(const CompoundStmt& compound);
    bool check(const Function& fn);
    bool check(const VarDecl& varDecl);
    bool check(const VariableDeclaration& varDecl);
    bool check(const IdentifierExpression& idExpr);
    bool check(const AssignExpr& assign);
    bool check(const BinaryExpr& binary);
    bool check(const UnaryExpr& unary);
    bool check(const ConditionalExpr& cond);
    bool check(const CallExpr& call);
    bool checkCallExpr(const CallExpr& call);
    bool check(const IfStmt& ifStmt);
    bool check(const WhileStmt& whileStmt);
    bool check(const DoWhileStmt& doWhileStmt);
    bool check(const ForStmt& forStmt);

    // Error and diagnostic queries
    bool hasErrors() const noexcept { return !m_diagnostics.empty(); }
    std::size_t errorCount() const noexcept { return m_diagnostics.size(); }
    const std::vector<Diagnostic>& diagnostics() const noexcept { return m_diagnostics; }
    const std::vector<Diagnostic>& errors() const noexcept { return m_diagnostics; }

    // State reset
    void clear();

    // Symbol table access
    const SymbolTable& symbolTable() const noexcept { return m_table; }
    SymbolTable& symbolTable() noexcept { return m_table; }

private:
    bool checkProgram(const Program& program);
    bool checkStatement(const Statement& stmt);
    bool checkExpression(const Expression& expr);
    bool checkCompoundStmt(const CompoundStmt& compound);
    bool checkFunction(const Function& fn);
    bool checkVarDecl(const VarDecl& varDecl);
    bool checkVariableDeclaration(const VariableDeclaration& varDecl);
    bool checkIdentifier(const IdentifierExpression& idExpr);
    bool checkAssignExpr(const AssignExpr& assign);
    bool checkBinaryExpr(const BinaryExpr& binary);
    bool checkUnaryExpr(const UnaryExpr& unary);
    bool checkConditionalExpr(const ConditionalExpr& cond);
    bool checkIfStmt(const IfStmt& ifStmt);
    bool checkWhileStmt(const WhileStmt& whileStmt);
    bool checkDoWhileStmt(const DoWhileStmt& doWhileStmt);
    bool checkForStmt(const ForStmt& forStmt);

    void reportUndeclared(const std::string& name, SourceLocation loc);

    std::unique_ptr<SymbolTable> m_ownedTable;
    SymbolTable& m_table;
    std::vector<Diagnostic> m_diagnostics;
};

// Type aliases
using UndeclaredIdentifierChecker = UndeclaredChecker;

// =============================================================================
// Convenience Checking Functions
// =============================================================================
std::vector<Diagnostic> checkUndeclared(const Program& program);
std::vector<Diagnostic> checkUndeclared(const Program& program, SymbolTable& table);
std::vector<Diagnostic> checkUndeclared(const Statement& stmt);
std::vector<Diagnostic> checkUndeclared(const Statement& stmt, SymbolTable& table);
std::vector<Diagnostic> checkUndeclared(const Expression& expr);
std::vector<Diagnostic> checkUndeclared(const Expression& expr, SymbolTable& table);

inline std::vector<Diagnostic> checkUndeclaredIdentifiers(const Program& program) {
    return checkUndeclared(program);
}
inline std::vector<Diagnostic> checkUndeclaredIdentifiers(const Program& program, SymbolTable& table) {
    return checkUndeclared(program, table);
}
inline std::vector<Diagnostic> checkUndeclaredIdentifiers(const Statement& stmt) {
    return checkUndeclared(stmt);
}
inline std::vector<Diagnostic> checkUndeclaredIdentifiers(const Statement& stmt, SymbolTable& table) {
    return checkUndeclared(stmt, table);
}
inline std::vector<Diagnostic> checkUndeclaredIdentifiers(const Expression& expr) {
    return checkUndeclared(expr);
}
inline std::vector<Diagnostic> checkUndeclaredIdentifiers(const Expression& expr, SymbolTable& table) {
    return checkUndeclared(expr, table);
}
