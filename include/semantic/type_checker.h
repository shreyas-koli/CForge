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
#include "types/conversions.h"
#include "types/type.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

// =============================================================================
// TypeChecker — Day 49 Expression Type Checking & Inference
// =============================================================================
//
// Implements expression type inference and type compatibility checking for CForge.
//
// Capabilities:
//   - Infers resulting Type for all supported expression AST nodes:
//     literals, identifiers, binary operations, unary operations,
//     assignments, and conditional expressions.
//   - Applies C17 §6.3.1.8 Usual Arithmetic Conversions for binary arithmetic.
//   - Reuses Day 39 type conversion APIs (isImplicitlyConvertible, isStrictlyImplicit)
//     to validate assignments and initializations.
//   - Detects invalid operations (e.g. arithmetic on void, floating-point modulo/bitwise,
//     assignment to const-qualified types, incompatible conditional branches).
//   - Suppresses cascading errors for unresolvable expressions.
//   - Traverses statements, compound blocks, functions, and programs.
//
class TypeChecker {
public:
    // Constructs a checker managing its own internal SymbolTable
    TypeChecker();

    // Constructs a checker using a shared external SymbolTable
    explicit TypeChecker(SymbolTable& table);

    ~TypeChecker() = default;

    // Type inference entry points
    std::shared_ptr<Type> inferType(const Expression& expr);
    std::shared_ptr<Type> inferType(const ASTNode& node);
    std::shared_ptr<Type> typeOf(const Expression& expr) { return inferType(expr); }

    // AST validation entry points
    bool check(const Program& program);
    bool check(const ASTNode& node);
    bool check(const Statement& stmt);
    bool check(const Expression& expr);
    bool check(const CompoundStmt& compound);
    bool check(const Function& fn);
    bool check(const VarDecl& varDecl);
    bool check(const VariableDeclaration& varDecl);
    bool check(const IfStmt& ifStmt);
    bool check(const WhileStmt& whileStmt);
    bool check(const DoWhileStmt& doWhileStmt);
    bool check(const ForStmt& forStmt);

    // Expression-specific inference methods
    std::shared_ptr<Type> inferLiteral(const LiteralExpression& lit);
    std::shared_ptr<Type> inferIdentifier(const IdentifierExpression& id);
    std::shared_ptr<Type> inferBinary(const BinaryExpr& bin);
    std::shared_ptr<Type> inferUnary(const UnaryExpr& unary);
    std::shared_ptr<Type> inferAssign(const AssignExpr& assign);
    std::shared_ptr<Type> inferConditional(const ConditionalExpr& cond);
    std::shared_ptr<Type> inferCall(const CallExpr& call);
    bool checkCall(const CallExpr& call);

    // Common arithmetic type computation (C17 §6.3.1.8 Usual Arithmetic Conversions)
    static std::shared_ptr<Type> commonType(const Type& t1, const Type& t2);
    static std::shared_ptr<Type> commonType(const std::shared_ptr<Type>& t1,
                                            const std::shared_ptr<Type>& t2);

    // Conversion helper queries reusing Day 39 APIs
    static bool isAssignable(const Type& source, const Type& destination);
    static bool isAssignable(const std::shared_ptr<Type>& source,
                             const std::shared_ptr<Type>& destination);
    static bool isStrictlyImplicitConversion(const Type& source, const Type& destination);
    static bool isStrictlyImplicitConversion(const std::shared_ptr<Type>& source,
                                             const std::shared_ptr<Type>& destination);

    // Error and diagnostic queries
    bool hasErrors() const noexcept { return !m_diagnostics.empty(); }
    std::size_t errorCount() const noexcept { return m_diagnostics.size(); }
    const std::vector<Diagnostic>& diagnostics() const noexcept { return m_diagnostics; }
    const std::vector<Diagnostic>& errors() const noexcept { return m_diagnostics; }

    // State management
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
    bool checkIfStmt(const IfStmt& ifStmt);
    bool checkWhileStmt(const WhileStmt& whileStmt);
    bool checkDoWhileStmt(const DoWhileStmt& doWhileStmt);
    bool checkForStmt(const ForStmt& forStmt);

    void reportError(const std::string& message, SourceLocation loc, const std::string& id = "");

    std::unique_ptr<SymbolTable> m_ownedTable;
    SymbolTable& m_table;
    std::vector<Diagnostic> m_diagnostics;
};

// =============================================================================
// Convenience Checking Functions
// =============================================================================
std::shared_ptr<Type> inferExpressionType(const Expression& expr);
std::shared_ptr<Type> inferExpressionType(const Expression& expr, SymbolTable& table);

std::vector<Diagnostic> checkTypes(const Expression& expr);
std::vector<Diagnostic> checkTypes(const Expression& expr, SymbolTable& table);
std::vector<Diagnostic> checkTypes(const Statement& stmt);
std::vector<Diagnostic> checkTypes(const Statement& stmt, SymbolTable& table);
std::vector<Diagnostic> checkTypes(const Program& program);
std::vector<Diagnostic> checkTypes(const Program& program, SymbolTable& table);
