#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/compound_stmt.h"
#include "ast/if_stmt.h"
#include "ast/printer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

#include <memory>
#include <string>

// Helper to construct a Parser for a source string
static Parser makeParser(const std::string& src) {
    Lexer lexer(src);
    return Parser(lexer.tokenize());
}

// Helper to print AST node to string via ASTPrinter
static std::string dumpAST(const ASTNode* node) {
    ASTPrinter printer;
    return printer.print(node);
}

// =============================================================================
// DAY 29 TESTS — If Statement (selection-statement: 'if' '(' expr ')' stmt)
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Basic If Statement Parsing
// -----------------------------------------------------------------------------
TEST_CASE("IfStmt: basic parsing", "[parser][if][stmt]") {
    SECTION("simple identifier condition 'if (x) {}'") {
        Parser parser = makeParser("if (x) {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        // Condition checks
        REQUIRE(stmt->condition() != nullptr);
        auto* idCond = dynamic_cast<const IdentifierExpression*>(stmt->condition());
        REQUIRE(idCond != nullptr);
        CHECK(idCond->name() == "x");
        CHECK(stmt->getCondition() == stmt->condition());

        // Then-branch checks
        REQUIRE(stmt->thenBranch() != nullptr);
        auto* block = dynamic_cast<const CompoundStmt*>(stmt->thenBranch());
        REQUIRE(block != nullptr);
        CHECK(block->empty());
        CHECK(stmt->getThenBranch() == stmt->thenBranch());
        CHECK(stmt->thenStmt() == stmt->thenBranch());
        CHECK(stmt->getThenStmt() == stmt->thenBranch());

        // Location of 'if'
        CHECK(stmt->location().line == 1);
        CHECK(stmt->location().column == 1);

        // String representation
        CHECK(stmt->toString() == "IfStmt(IdentifierExpression(x), CompoundStmt (0 statements))");
    }

    SECTION("relational condition 'if (x > 10) {}'") {
        Parser parser = makeParser("if (x > 10) {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* ifStmt = dynamic_cast<IfStmt*>(stmt.get());
        REQUIRE(ifStmt != nullptr);

        auto* bin = dynamic_cast<const BinaryExpr*>(ifStmt->condition());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == ">");
        CHECK(bin->left()->toString() == "IdentifierExpression(x)");
        CHECK(bin->right()->toString() == "LiteralExpression(Integer, 10)");

        auto* block = dynamic_cast<const CompoundStmt*>(ifStmt->thenBranch());
        REQUIRE(block != nullptr);
        CHECK(block->empty());
    }

    SECTION("literal condition 'if (1) {}'") {
        Parser parser = makeParser("if (1) {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* ifStmt = dynamic_cast<IfStmt*>(stmt.get());
        REQUIRE(ifStmt != nullptr);

        auto* lit = dynamic_cast<const LiteralExpression*>(ifStmt->condition());
        REQUIRE(lit != nullptr);
        CHECK(lit->value() == "1");
    }
}

// -----------------------------------------------------------------------------
// 2. Conditions with Various Expression Types
// -----------------------------------------------------------------------------
TEST_CASE("IfStmt: complex conditions", "[parser][if][expr]") {
    SECTION("arithmetic and relational 'if (x + 1 > y * 2) {}'") {
        Parser parser = makeParser("if (x + 1 > y * 2) {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* rel = dynamic_cast<const BinaryExpr*>(stmt->condition());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == ">");

        auto* add = dynamic_cast<const BinaryExpr*>(rel->left());
        REQUIRE(add != nullptr);
        CHECK(add->op() == "+");

        auto* mul = dynamic_cast<const BinaryExpr*>(rel->right());
        REQUIRE(mul != nullptr);
        CHECK(mul->op() == "*");
    }

    SECTION("equality condition 'if (a == b) {}'") {
        Parser parser = makeParser("if (a == b) {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* eq = dynamic_cast<const BinaryExpr*>(stmt->condition());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");
    }

    SECTION("bitwise and shift condition 'if (a << 1 & 0xFF) {}'") {
        Parser parser = makeParser("if (a << 1 & 0xFF) {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bitAnd = dynamic_cast<const BinaryExpr*>(stmt->condition());
        REQUIRE(bitAnd != nullptr);
        CHECK(bitAnd->op() == "&");

        auto* sh = dynamic_cast<const BinaryExpr*>(bitAnd->left());
        REQUIRE(sh != nullptr);
        CHECK(sh->op() == "<<");
    }

    SECTION("ternary condition 'if (cond ? a : b) {}'") {
        Parser parser = makeParser("if (cond ? a : b) {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* ternary = dynamic_cast<const ConditionalExpr*>(stmt->condition());
        REQUIRE(ternary != nullptr);
        CHECK(ternary->condition()->toString() == "IdentifierExpression(cond)");
        CHECK(ternary->thenExpr()->toString() == "IdentifierExpression(a)");
        CHECK(ternary->elseExpr()->toString() == "IdentifierExpression(b)");
    }
}

// -----------------------------------------------------------------------------
// 3. Nested If Statements
// -----------------------------------------------------------------------------
TEST_CASE("IfStmt: nested if statements", "[parser][if][nesting]") {
    SECTION("nested without block 'if (a) if (b) {}'") {
        Parser parser = makeParser("if (a) if (b) {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(stmt->condition()->toString() == "IdentifierExpression(a)");

        auto* innerIf = dynamic_cast<const IfStmt*>(stmt->thenBranch());
        REQUIRE(innerIf != nullptr);
        CHECK(innerIf->condition()->toString() == "IdentifierExpression(b)");

        auto* innerBlock = dynamic_cast<const CompoundStmt*>(innerIf->thenBranch());
        REQUIRE(innerBlock != nullptr);
        CHECK(innerBlock->empty());
    }

    SECTION("nested inside compound block 'if (a) { if (b) {} }'") {
        Parser parser = makeParser("if (a) { if (b) {} }");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* outerBlock = dynamic_cast<const CompoundStmt*>(stmt->thenBranch());
        REQUIRE(outerBlock != nullptr);
        REQUIRE(outerBlock->statementCount() == 1);

        auto* innerIf = dynamic_cast<const IfStmt*>(outerBlock->statements()[0].get());
        REQUIRE(innerIf != nullptr);
        CHECK(innerIf->condition()->toString() == "IdentifierExpression(b)");

        auto* innerBlock = dynamic_cast<const CompoundStmt*>(innerIf->thenBranch());
        REQUIRE(innerBlock != nullptr);
        CHECK(innerBlock->empty());
    }

    SECTION("three-level nesting 'if (a) if (b) if (c) {}'") {
        Parser parser = makeParser("if (a) if (b) if (c) {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* if2 = dynamic_cast<const IfStmt*>(stmt->thenBranch());
        REQUIRE(if2 != nullptr);
        CHECK(if2->condition()->toString() == "IdentifierExpression(b)");

        auto* if3 = dynamic_cast<const IfStmt*>(if2->thenBranch());
        REQUIRE(if3 != nullptr);
        CHECK(if3->condition()->toString() == "IdentifierExpression(c)");

        auto* block = dynamic_cast<const CompoundStmt*>(if3->thenBranch());
        REQUIRE(block != nullptr);
        CHECK(block->empty());
    }

    SECTION("sibling if statements inside block '{ if (x) {} if (y) {} }'") {
        Parser parser = makeParser("{ if (x) {} if (y) {} }");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* block = dynamic_cast<CompoundStmt*>(stmt.get());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 2);

        auto* if1 = dynamic_cast<const IfStmt*>(block->statements()[0].get());
        REQUIRE(if1 != nullptr);
        CHECK(if1->condition()->toString() == "IdentifierExpression(x)");

        auto* if2 = dynamic_cast<const IfStmt*>(block->statements()[1].get());
        REQUIRE(if2 != nullptr);
        CHECK(if2->condition()->toString() == "IdentifierExpression(y)");
    }
}

// -----------------------------------------------------------------------------
// 4. Source Location Tracking
// -----------------------------------------------------------------------------
TEST_CASE("IfStmt: source location", "[parser][if][location]") {
    SECTION("tracked at 'if' keyword on line 2 column 5") {
        Parser parser = makeParser("\n    if (x) {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);
        CHECK(stmt->location().line == 2);
        CHECK(stmt->location().column == 5);
    }
}

// -----------------------------------------------------------------------------
// 5. ASTPrinter Formatting
// -----------------------------------------------------------------------------
TEST_CASE("IfStmt: ASTPrinter output", "[parser][if][printer]") {
    SECTION("simple 'if (x > 10) {}'") {
        Parser parser = makeParser("if (x > 10) {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);

        std::string expected =
            "IfStmt\n"
            "  Condition:\n"
            "    BinaryExpr: >\n"
            "      Left:\n"
            "        IdentifierExpression: x\n"
            "      Right:\n"
            "        LiteralExpression (Integer): 10\n"
            "  Then:\n"
            "    CompoundStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }

    SECTION("nested 'if (a) if (b) {}'") {
        Parser parser = makeParser("if (a) if (b) {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);

        std::string expected =
            "IfStmt\n"
            "  Condition:\n"
            "    IdentifierExpression: a\n"
            "  Then:\n"
            "    IfStmt\n"
            "      Condition:\n"
            "        IdentifierExpression: b\n"
            "      Then:\n"
            "        CompoundStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }
}

// -----------------------------------------------------------------------------
// 6. Error Handling and Negative Tests
// -----------------------------------------------------------------------------
TEST_CASE("IfStmt: error handling and syntax errors", "[parser][if][error]") {
    SECTION("missing '(' after 'if': 'if x > 10) {}'") {
        Parser parser = makeParser("if x > 10) {}");
        auto stmt = parser.parseIfStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '(' after 'if'") != std::string::npos);
    }

    SECTION("missing ')' after condition: 'if (x > 10 {}'") {
        Parser parser = makeParser("if (x > 10 {}");
        auto stmt = parser.parseIfStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ')' after 'if' condition") != std::string::npos);
    }

    SECTION("missing condition inside parens: 'if () {}'") {
        Parser parser = makeParser("if () {}");
        auto stmt = parser.parseIfStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression in 'if' condition") != std::string::npos);
    }

    SECTION("missing then-branch at EOF: 'if (x)'") {
        Parser parser = makeParser("if (x)");
        auto stmt = parser.parseIfStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement after 'if' condition") != std::string::npos);
    }

    SECTION("trailing at EOF after 'if': 'if'") {
        Parser parser = makeParser("if");
        auto stmt = parser.parseIfStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '(' after 'if'") != std::string::npos);
    }

    SECTION("malformed condition: 'if (x +) {}'") {
        Parser parser = makeParser("if (x +) {}");
        auto stmt = parser.parseIfStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression after '+'") != std::string::npos);
    }

    SECTION("malformed then-branch block: 'if (x) {'") {
        Parser parser = makeParser("if (x) {");
        auto stmt = parser.parseIfStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '}' to close compound statement") != std::string::npos);
    }

    SECTION("unexpected trailing tokens after valid if statement") {
        Parser parser = makeParser("if (x) {} extra");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK_FALSE(parser.isAtEnd());
        CHECK(parser.peek().lexeme == "extra");
    }
}

// =============================================================================
// DAY 30 TESTS — If / Else Statements & Dangling-Else Resolution
// =============================================================================

// -----------------------------------------------------------------------------
// 7. If / Else Basic Parsing
// -----------------------------------------------------------------------------
TEST_CASE("IfStmt: if-else basic parsing", "[parser][if][else]") {
    SECTION("'if (x) {} else {}' parses both branches") {
        Parser parser = makeParser("if (x) {} else {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        // Condition
        REQUIRE(stmt->condition() != nullptr);
        auto* idCond = dynamic_cast<const IdentifierExpression*>(stmt->condition());
        REQUIRE(idCond != nullptr);
        CHECK(idCond->name() == "x");

        // Then-branch
        REQUIRE(stmt->thenBranch() != nullptr);
        auto* thenBlock = dynamic_cast<const CompoundStmt*>(stmt->thenBranch());
        REQUIRE(thenBlock != nullptr);
        CHECK(thenBlock->empty());

        // Else-branch
        CHECK(stmt->hasElse());
        CHECK(stmt->hasElseBranch());
        REQUIRE(stmt->elseBranch() != nullptr);
        auto* elseBlock = dynamic_cast<const CompoundStmt*>(stmt->elseBranch());
        REQUIRE(elseBlock != nullptr);
        CHECK(elseBlock->empty());

        // Getter accessors
        CHECK(stmt->getElseBranch() == stmt->elseBranch());
        CHECK(stmt->elseStmt() == stmt->elseBranch());
        CHECK(stmt->getElseStmt() == stmt->elseBranch());

        // String representation
        CHECK(stmt->toString() ==
              "IfStmt(IdentifierExpression(x), CompoundStmt (0 statements), CompoundStmt (0 statements))");
    }

    SECTION("'if (x) {}' still parses with no else branch") {
        Parser parser = makeParser("if (x) {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK_FALSE(stmt->hasElse());
        CHECK_FALSE(stmt->hasElseBranch());
        CHECK(stmt->elseBranch() == nullptr);
        CHECK(stmt->getElseBranch() == nullptr);
        CHECK(stmt->elseStmt() == nullptr);
        CHECK(stmt->getElseStmt() == nullptr);

        CHECK(stmt->toString() == "IfStmt(IdentifierExpression(x), CompoundStmt (0 statements))");
    }

    SECTION("both branches with nested statements") {
        Parser parser = makeParser("if (a > 0) { {} } else { {} }");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* ifStmt = dynamic_cast<IfStmt*>(stmt.get());
        REQUIRE(ifStmt != nullptr);
        CHECK(ifStmt->hasElse());

        auto* thenBlock = dynamic_cast<const CompoundStmt*>(ifStmt->thenBranch());
        REQUIRE(thenBlock != nullptr);
        CHECK(thenBlock->statementCount() == 1);

        auto* elseBlock = dynamic_cast<const CompoundStmt*>(ifStmt->elseBranch());
        REQUIRE(elseBlock != nullptr);
        CHECK(elseBlock->statementCount() == 1);
    }
}

// -----------------------------------------------------------------------------
// 8. If / Else-If Chains
// -----------------------------------------------------------------------------
TEST_CASE("IfStmt: if-else-if chains", "[parser][if][else_if]") {
    SECTION("'if (x) {} else if (y) {}' parses as nested IfStmt in else branch") {
        Parser parser = makeParser("if (x) {} else if (y) {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* outerIf = dynamic_cast<IfStmt*>(stmt.get());
        REQUIRE(outerIf != nullptr);
        CHECK(outerIf->hasElse());
        CHECK(outerIf->condition()->toString() == "IdentifierExpression(x)");

        auto* innerIf = dynamic_cast<const IfStmt*>(outerIf->elseBranch());
        REQUIRE(innerIf != nullptr);
        CHECK(innerIf->condition()->toString() == "IdentifierExpression(y)");
        CHECK_FALSE(innerIf->hasElse());
    }

    SECTION("three-way chain 'if (x) {} else if (y) {} else {}'") {
        Parser parser = makeParser("if (x) {} else if (y) {} else {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* if1 = dynamic_cast<IfStmt*>(stmt.get());
        REQUIRE(if1 != nullptr);
        CHECK(if1->hasElse());
        CHECK(if1->condition()->toString() == "IdentifierExpression(x)");

        auto* if2 = dynamic_cast<const IfStmt*>(if1->elseBranch());
        REQUIRE(if2 != nullptr);
        CHECK(if2->hasElse());
        CHECK(if2->condition()->toString() == "IdentifierExpression(y)");

        auto* finalElse = dynamic_cast<const CompoundStmt*>(if2->elseBranch());
        REQUIRE(finalElse != nullptr);
        CHECK(finalElse->empty());
    }

    SECTION("four-way chain 'if (a) {} else if (b) {} else if (c) {} else {}'") {
        Parser parser = makeParser("if (a) {} else if (b) {} else if (c) {} else {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* ifA = dynamic_cast<IfStmt*>(stmt.get());
        REQUIRE(ifA != nullptr);

        auto* ifB = dynamic_cast<const IfStmt*>(ifA->elseBranch());
        REQUIRE(ifB != nullptr);
        CHECK(ifB->condition()->toString() == "IdentifierExpression(b)");

        auto* ifC = dynamic_cast<const IfStmt*>(ifB->elseBranch());
        REQUIRE(ifC != nullptr);
        CHECK(ifC->condition()->toString() == "IdentifierExpression(c)");

        auto* finalBlock = dynamic_cast<const CompoundStmt*>(ifC->elseBranch());
        REQUIRE(finalBlock != nullptr);
        CHECK(finalBlock->empty());
    }
}

// -----------------------------------------------------------------------------
// 9. Dangling-Else Resolution
// -----------------------------------------------------------------------------
TEST_CASE("IfStmt: dangling-else resolution", "[parser][if][dangling_else]") {
    SECTION("unbraced: 'if (a) if (b) {} else {}' binds else to inner 'if (b)'") {
        // C standard rule: 'else' associates with the nearest preceding 'if'
        // AST shape:
        // IfStmt (a)
        //   Then: IfStmt (b)
        //           Then: CompoundStmt
        //           Else: CompoundStmt
        //   Else: (none)
        Parser parser = makeParser("if (a) if (b) {} else {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        // Outer if (a)
        CHECK(stmt->condition()->toString() == "IdentifierExpression(a)");
        CHECK_FALSE(stmt->hasElse());
        CHECK(stmt->elseBranch() == nullptr);

        // Inner if (b) owns the else
        auto* innerIf = dynamic_cast<const IfStmt*>(stmt->thenBranch());
        REQUIRE(innerIf != nullptr);
        CHECK(innerIf->condition()->toString() == "IdentifierExpression(b)");
        CHECK(innerIf->hasElse());

        auto* innerThen = dynamic_cast<const CompoundStmt*>(innerIf->thenBranch());
        REQUIRE(innerThen != nullptr);
        CHECK(innerThen->empty());

        auto* innerElse = dynamic_cast<const CompoundStmt*>(innerIf->elseBranch());
        REQUIRE(innerElse != nullptr);
        CHECK(innerElse->empty());
    }

    SECTION("braced: 'if (a) { if (b) {} } else {}' binds else to outer 'if (a)'") {
        // Braces force inner 'if (b)' to close before 'else'
        // AST shape:
        // IfStmt (a)
        //   Then: CompoundStmt -> [ IfStmt (b, Then: CompoundStmt, Else: none) ]
        //   Else: CompoundStmt
        Parser parser = makeParser("if (a) { if (b) {} } else {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        // Outer if (a) owns the else
        CHECK(stmt->condition()->toString() == "IdentifierExpression(a)");
        CHECK(stmt->hasElse());

        auto* outerElse = dynamic_cast<const CompoundStmt*>(stmt->elseBranch());
        REQUIRE(outerElse != nullptr);
        CHECK(outerElse->empty());

        // Inner if (b) inside compound block has NO else
        auto* outerThenBlock = dynamic_cast<const CompoundStmt*>(stmt->thenBranch());
        REQUIRE(outerThenBlock != nullptr);
        REQUIRE(outerThenBlock->statementCount() == 1);

        auto* innerIf = dynamic_cast<const IfStmt*>(outerThenBlock->statements()[0].get());
        REQUIRE(innerIf != nullptr);
        CHECK(innerIf->condition()->toString() == "IdentifierExpression(b)");
        CHECK_FALSE(innerIf->hasElse());
    }

    SECTION("unbraced outer else: 'if (a) if (b) {} else {} else {}'") {
        // First else binds to inner 'if (b)', second else binds to outer 'if (a)'
        // AST shape:
        // IfStmt (a)
        //   Then: IfStmt (b)
        //           Then: CompoundStmt
        //           Else: CompoundStmt
        //   Else: CompoundStmt
        Parser parser = makeParser("if (a) if (b) {} else {} else {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        // Outer if (a)
        CHECK(stmt->condition()->toString() == "IdentifierExpression(a)");
        CHECK(stmt->hasElse());
        auto* outerElse = dynamic_cast<const CompoundStmt*>(stmt->elseBranch());
        REQUIRE(outerElse != nullptr);

        // Inner if (b)
        auto* innerIf = dynamic_cast<const IfStmt*>(stmt->thenBranch());
        REQUIRE(innerIf != nullptr);
        CHECK(innerIf->condition()->toString() == "IdentifierExpression(b)");
        CHECK(innerIf->hasElse());
        auto* innerElse = dynamic_cast<const CompoundStmt*>(innerIf->elseBranch());
        REQUIRE(innerElse != nullptr);
    }
}

// -----------------------------------------------------------------------------
// 10. ASTPrinter Output for If / Else
// -----------------------------------------------------------------------------
TEST_CASE("IfStmt: ASTPrinter output with else", "[parser][if][else][printer]") {
    SECTION("simple 'if (x) {} else {}'") {
        Parser parser = makeParser("if (x) {} else {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);

        std::string expected =
            "IfStmt\n"
            "  Condition:\n"
            "    IdentifierExpression: x\n"
            "  Then:\n"
            "    CompoundStmt\n"
            "  Else:\n"
            "    CompoundStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }

    SECTION("chained 'if (x) {} else if (y) {}'") {
        Parser parser = makeParser("if (x) {} else if (y) {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);

        std::string expected =
            "IfStmt\n"
            "  Condition:\n"
            "    IdentifierExpression: x\n"
            "  Then:\n"
            "    CompoundStmt\n"
            "  Else:\n"
            "    IfStmt\n"
            "      Condition:\n"
            "        IdentifierExpression: y\n"
            "      Then:\n"
            "        CompoundStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }

    SECTION("dangling-else printer output 'if (a) if (b) {} else {}'") {
        Parser parser = makeParser("if (a) if (b) {} else {}");
        auto stmt = parser.parseIfStatement();
        REQUIRE(stmt != nullptr);

        std::string expected =
            "IfStmt\n"
            "  Condition:\n"
            "    IdentifierExpression: a\n"
            "  Then:\n"
            "    IfStmt\n"
            "      Condition:\n"
            "        IdentifierExpression: b\n"
            "      Then:\n"
            "        CompoundStmt\n"
            "      Else:\n"
            "        CompoundStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }
}

// -----------------------------------------------------------------------------
// 11. Error Handling and Negative Tests for Else
// -----------------------------------------------------------------------------
TEST_CASE("IfStmt: else branch error handling", "[parser][if][else][error]") {
    SECTION("missing statement after 'else' at EOF: 'if (x) {} else'") {
        Parser parser = makeParser("if (x) {} else");
        auto stmt = parser.parseIfStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement after 'else'") != std::string::npos);
    }

    SECTION("unclosed compound statement in else: 'if (x) {} else {'") {
        Parser parser = makeParser("if (x) {} else {");
        auto stmt = parser.parseIfStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '}' to close compound statement") != std::string::npos);
    }

    SECTION("unexpected punctuation after 'else': 'if (x) {} else )'") {
        Parser parser = makeParser("if (x) {} else )");
        auto stmt = parser.parseIfStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement, found ')'") != std::string::npos);
    }
}


