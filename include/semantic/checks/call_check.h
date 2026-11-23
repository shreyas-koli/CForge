#pragma once

#include "ast/ast_node.h"
#include "ast/call_expr.h"
#include "ast/compound_stmt.h"
#include "ast/expression.h"
#include "ast/function.h"
#include "ast/program.h"
#include "ast/statement.h"
#include "ast/var_decl.h"
#include "ast/variable_declaration.h"
#include "semantic/checks/redeclaration.h"
#include "semantic/symbol_table.h"
#include "semantic/type_checker.h"
#include "types/conversions.h"
#include "types/type.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

// =============================================================================
// CallChecker — Day 50 Function-Call Validation
// =============================================================================
//
// Validates function calls against declared function signatures in the SymbolTable:
//   1. Resolves the called function using the SymbolTable.
//   2. Detects undeclared functions and calls to non-function objects (variables).
//   3. Validates argument counts (too few / too many / zero-parameter / void parameter list).
//   4. Validates argument types using Day 39 implicit conversion rules and Day 49
//      expression type inference.
//   5. Handles nested function calls recursively.
//   6. Avoids cascading diagnostics when argument type inference or identifier lookup fails.
//
class CallChecker {
public:
    CallChecker();
    explicit CallChecker(SymbolTable& table);
    ~CallChecker() = default;

    // Check entry points
    bool check(const Program& program);
    bool check(const ASTNode& node);
    bool check(const Statement& stmt);
    bool check(const Expression& expr);
    bool check(const CompoundStmt& compound);
    bool check(const Function& fn);
    bool check(const VarDecl& varDecl);
    bool check(const VariableDeclaration& varDecl);
    bool check(const CallExpr& call);

    // Direct function call validation helper
    bool checkCall(const CallExpr& call);
    bool checkCall(const std::string& calleeName,
                   const std::vector<std::unique_ptr<Expression>>& args,
                   SourceLocation loc = {});
    bool checkCall(const std::string& calleeName,
                   const std::vector<const Expression*>& args,
                   SourceLocation loc = {});

    // State management
    void clear();

    // Symbol table access
    const SymbolTable& symbolTable() const noexcept { return m_table; }
    SymbolTable& symbolTable() noexcept { return m_table; }

    // Diagnostic queries
    bool hasErrors() const noexcept { return !m_diagnostics.empty(); }
    std::size_t errorCount() const noexcept { return m_diagnostics.size(); }
    const std::vector<Diagnostic>& diagnostics() const noexcept { return m_diagnostics; }
    const std::vector<Diagnostic>& errors() const noexcept { return m_diagnostics; }

private:
    bool checkProgram(const Program& program);
    bool checkStatement(const Statement& stmt);
    bool checkCompoundStmt(const CompoundStmt& compound);
    bool checkFunction(const Function& fn);
    bool checkVarDecl(const VarDecl& varDecl);
    bool checkVariableDeclaration(const VariableDeclaration& varDecl);
    bool checkExpression(const Expression& expr);

    bool validateCall(const std::string& calleeName,
                      const std::vector<const Expression*>& args,
                      SourceLocation loc);

    void reportError(const std::string& message, SourceLocation loc, const std::string& id = "");

    std::unique_ptr<SymbolTable> m_ownedTable;
    SymbolTable& m_table;
    TypeChecker m_typeChecker;
    std::vector<Diagnostic> m_diagnostics;
};

// =============================================================================
// Convenience Checking Functions
// =============================================================================
std::vector<Diagnostic> checkCalls(const Expression& expr);
std::vector<Diagnostic> checkCalls(const Expression& expr, SymbolTable& table);
std::vector<Diagnostic> checkCalls(const Program& program);
std::vector<Diagnostic> checkCalls(const Program& program, SymbolTable& table);
