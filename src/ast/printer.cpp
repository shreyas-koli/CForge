#include "ast/printer.h"

#include "ast/assign_expr.h"
#include "ast/binary_expr.h"
#include "ast/compound_stmt.h"
#include "ast/conditional_expr.h"
#include "ast/do_while_stmt.h"
#include "ast/for_stmt.h"
#include "ast/function.h"
#include "ast/identifier_expression.h"
#include "ast/if_stmt.h"
#include "ast/literal_expression.h"
#include "ast/program.h"
#include "ast/unary_expr.h"
#include "ast/variable_declaration.h"
#include "ast/while_stmt.h"

#include <iostream>
#include <sstream>

ASTPrinter::ASTPrinter(int indentSpaces)
    : m_indentSpaces(indentSpaces) {}

std::string ASTPrinter::makeIndent(int level) const {
    return std::string(static_cast<std::size_t>(level * m_indentSpaces), ' ');
}

std::string ASTPrinter::print(const ASTNode* node) const {
    std::ostringstream ss;
    print(ss, node);
    return ss.str();
}

void ASTPrinter::print(std::ostream& out, const ASTNode* node) const {
    printNode(out, node, 0);
}

void ASTPrinter::printNode(std::ostream& out, const ASTNode* node, int level) const {
    if (!node) {
        out << makeIndent(level) << "[null]\n";
        return;
    }

    if (auto* prog = dynamic_cast<const Program*>(node)) {
        printProgram(out, prog, level);
    } else if (auto* fn = dynamic_cast<const Function*>(node)) {
        printFunction(out, fn, level);
    } else if (auto* var = dynamic_cast<const VariableDeclaration*>(node)) {
        printVariableDeclaration(out, var, level);
    } else if (auto* compound = dynamic_cast<const CompoundStmt*>(node)) {
        printCompound(out, compound, level);
    } else if (auto* ifStmt = dynamic_cast<const IfStmt*>(node)) {
        printIf(out, ifStmt, level);
    } else if (auto* whileStmt = dynamic_cast<const WhileStmt*>(node)) {
        printWhile(out, whileStmt, level);
    } else if (auto* doWhileStmt = dynamic_cast<const DoWhileStmt*>(node)) {
        printDoWhile(out, doWhileStmt, level);
    } else if (auto* forStmt = dynamic_cast<const ForStmt*>(node)) {
        printFor(out, forStmt, level);
    } else if (auto* bin = dynamic_cast<const BinaryExpr*>(node)) {
        printBinary(out, bin, level);
    } else if (auto* assign = dynamic_cast<const AssignExpr*>(node)) {
        printAssign(out, assign, level);
    } else if (auto* cond = dynamic_cast<const ConditionalExpr*>(node)) {
        printConditional(out, cond, level);
    } else if (auto* un = dynamic_cast<const UnaryExpr*>(node)) {
        printUnary(out, un, level);
    } else if (auto* lit = dynamic_cast<const LiteralExpression*>(node)) {
        printLiteral(out, lit, level);
    } else if (auto* id = dynamic_cast<const IdentifierExpression*>(node)) {
        printIdentifier(out, id, level);
    } else {
        out << makeIndent(level) << node->toString() << "\n";
    }
}

void ASTPrinter::printProgram(std::ostream& out, const Program* prog, int level) const {
    out << makeIndent(level) << "Program\n";
    for (const auto& stmt : prog->statements()) {
        printNode(out, stmt.get(), level + 1);
    }
}

void ASTPrinter::printFunction(std::ostream& out, const Function* fn, int level) const {
    std::string kindStr = fn->isDefinition() ? "definition" : "prototype";
    out << makeIndent(level) << "Function (" << kindStr << "): "
        << fn->returnType() << " " << fn->name() << "\n";

    // Parameters
    if (fn->parameters().empty()) {
        out << makeIndent(level + 1) << "Parameters: (none)\n";
    } else {
        out << makeIndent(level + 1) << "Parameters:\n";
        for (const auto& param : fn->parameters()) {
            printNode(out, param.get(), level + 2);
        }
    }

    // Body (only definitions have a body block)
    if (fn->isDefinition()) {
        out << makeIndent(level + 1) << "Body:\n";
        if (fn->body().empty()) {
            out << makeIndent(level + 2) << "[body not parsed]\n";
        } else {
            for (const auto& stmt : fn->body()) {
                printNode(out, stmt.get(), level + 2);
            }
        }
    }
}

void ASTPrinter::printVariableDeclaration(std::ostream& out, const VariableDeclaration* var, int level) const {
    out << makeIndent(level) << "VariableDeclaration: "
        << var->type() << " " << var->name() << "\n";
    if (var->hasInitializer()) {
        out << makeIndent(level + 1) << "Initializer:\n";
        printNode(out, var->initializer(), level + 2);
    }
}

void ASTPrinter::printLiteral(std::ostream& out, const LiteralExpression* lit, int level) const {
    out << makeIndent(level) << "LiteralExpression ("
        << literalKindName(lit->kind()) << "): " << lit->value() << "\n";
}

void ASTPrinter::printIdentifier(std::ostream& out, const IdentifierExpression* id, int level) const {
    out << makeIndent(level) << "IdentifierExpression: " << id->name() << "\n";
}

void ASTPrinter::printBinary(std::ostream& out, const BinaryExpr* bin, int level) const {
    out << makeIndent(level) << "BinaryExpr: " << bin->op() << "\n";
    out << makeIndent(level + 1) << "Left:\n";
    printNode(out, bin->left(), level + 2);
    out << makeIndent(level + 1) << "Right:\n";
    printNode(out, bin->right(), level + 2);
}

void ASTPrinter::printAssign(std::ostream& out, const AssignExpr* assign, int level) const {
    out << makeIndent(level) << "AssignExpr: " << assign->op() << "\n";
    out << makeIndent(level + 1) << "Left:\n";
    printNode(out, assign->left(), level + 2);
    out << makeIndent(level + 1) << "Right:\n";
    printNode(out, assign->right(), level + 2);
}

void ASTPrinter::printUnary(std::ostream& out, const UnaryExpr* un, int level) const {
    out << makeIndent(level) << "UnaryExpr: " << un->op() << "\n";
    out << makeIndent(level + 1) << "Operand:\n";
    printNode(out, un->operand(), level + 2);
}

void ASTPrinter::printConditional(std::ostream& out, const ConditionalExpr* cond, int level) const {
    out << makeIndent(level) << "ConditionalExpr\n";
    out << makeIndent(level + 1) << "Condition:\n";
    printNode(out, cond->condition(), level + 2);
    out << makeIndent(level + 1) << "Then:\n";
    printNode(out, cond->thenExpr(), level + 2);
    out << makeIndent(level + 1) << "Else:\n";
    printNode(out, cond->elseExpr(), level + 2);
}

void ASTPrinter::printCompound(std::ostream& out, const CompoundStmt* compound, int level) const {
    out << makeIndent(level) << "CompoundStmt\n";
    for (const auto& stmt : compound->statements()) {
        printNode(out, stmt.get(), level + 1);
    }
}

void ASTPrinter::printIf(std::ostream& out, const IfStmt* ifStmt, int level) const {
    out << makeIndent(level) << "IfStmt\n";
    out << makeIndent(level + 1) << "Condition:\n";
    printNode(out, ifStmt->condition(), level + 2);
    out << makeIndent(level + 1) << "Then:\n";
    printNode(out, ifStmt->thenBranch(), level + 2);
    if (ifStmt->hasElse()) {
        out << makeIndent(level + 1) << "Else:\n";
        printNode(out, ifStmt->elseBranch(), level + 2);
    }
}

void ASTPrinter::printWhile(std::ostream& out, const WhileStmt* whileStmt, int level) const {
    out << makeIndent(level) << "WhileStmt\n";
    out << makeIndent(level + 1) << "Condition:\n";
    printNode(out, whileStmt->condition(), level + 2);
    out << makeIndent(level + 1) << "Body:\n";
    printNode(out, whileStmt->body(), level + 2);
}

void ASTPrinter::printDoWhile(std::ostream& out, const DoWhileStmt* doWhileStmt, int level) const {
    out << makeIndent(level) << "DoWhileStmt\n";
    out << makeIndent(level + 1) << "Body:\n";
    printNode(out, doWhileStmt->body(), level + 2);
    out << makeIndent(level + 1) << "Condition:\n";
    printNode(out, doWhileStmt->condition(), level + 2);
}

void ASTPrinter::printFor(std::ostream& out, const ForStmt* forStmt, int level) const {
    out << makeIndent(level) << "ForStmt\n";

    out << makeIndent(level + 1) << "Initializer:\n";
    if (forStmt->initializer()) {
        printNode(out, forStmt->initializer(), level + 2);
    } else {
        out << makeIndent(level + 2) << "(omitted)\n";
    }

    out << makeIndent(level + 1) << "Condition:\n";
    if (forStmt->condition()) {
        printNode(out, forStmt->condition(), level + 2);
    } else {
        out << makeIndent(level + 2) << "(omitted)\n";
    }

    out << makeIndent(level + 1) << "Post:\n";
    if (forStmt->post()) {
        printNode(out, forStmt->post(), level + 2);
    } else {
        out << makeIndent(level + 2) << "(omitted)\n";
    }

    out << makeIndent(level + 1) << "Body:\n";
    printNode(out, forStmt->body(), level + 2);
}

std::string printAST(const ASTNode* node, int indentSpaces) {
    ASTPrinter printer(indentSpaces);
    return printer.print(node);
}

void printAST(std::ostream& out, const ASTNode* node, int indentSpaces) {
    ASTPrinter printer(indentSpaces);
    printer.print(out, node);
}

