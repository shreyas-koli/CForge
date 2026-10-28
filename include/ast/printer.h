#pragma once

#include "ast/ast_node.h"
#include <iosfwd>
#include <string>

// Forward declarations
class Program;
class Function;
class VariableDeclaration;
class LiteralExpression;
class IdentifierExpression;
class BinaryExpr;
class UnaryExpr;
class AssignExpr;
class ConditionalExpr;

// =============================================================================
// ASTPrinter — Formats and prints CForge ASTs in a readable tree format (Day 19)
// =============================================================================
//
// Recursively traverses all supported AST node types and produces an indented
// textual tree representation showing:
//   - Node names and child relationships
//   - Operator symbols and operand distinctions (Left / Right / Operand)
//   - Literal kinds and values
//   - Identifiers and type specifiers
//   - Function prototype vs definition status
//   - Documented stub marker [body not parsed] for skipped function bodies
//
class ASTPrinter {
public:
    explicit ASTPrinter(int indentSpaces = 2);

    // Formats any ASTNode into a readable tree string
    std::string print(const ASTNode* node) const;

    // Stream output helper
    void print(std::ostream& out, const ASTNode* node) const;

private:
    int m_indentSpaces{2};

    void printNode(std::ostream& out, const ASTNode* node, int level) const;
    void printProgram(std::ostream& out, const Program* prog, int level) const;
    void printFunction(std::ostream& out, const Function* fn, int level) const;
    void printVariableDeclaration(std::ostream& out, const VariableDeclaration* var, int level) const;
    void printLiteral(std::ostream& out, const LiteralExpression* lit, int level) const;
    void printIdentifier(std::ostream& out, const IdentifierExpression* id, int level) const;
    void printBinary(std::ostream& out, const BinaryExpr* bin, int level) const;
    void printUnary(std::ostream& out, const UnaryExpr* un, int level) const;
    void printAssign(std::ostream& out, const AssignExpr* assign, int level) const;

    std::string makeIndent(int level) const;
};

// Convenience free functions
std::string printAST(const ASTNode* node, int indentSpaces = 2);
void printAST(std::ostream& out, const ASTNode* node, int indentSpaces = 2);

