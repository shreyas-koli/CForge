#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/compound_stmt.h"
#include "ast/if_stmt.h"
#include "ast/while_stmt.h"
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
// DAY 31 TESTS — While Loop (iteration-statement: 'while' '(' expr ')' stmt)
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Basic While Loop Parsing
// -----------------------------------------------------------------------------
TEST_CASE("WhileStmt: basic parsing", "[parser][while][stmt]") {
    SECTION("simple identifier condition 'while (x) {}'") {
        Parser parser = makeParser("while (x) {}");
        auto stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        // Condition checks
        REQUIRE(stmt->condition() != nullptr);
        auto* idCond = dynamic_cast<const IdentifierExpression*>(stmt->condition());
        REQUIRE(idCond != nullptr);
        CHECK(idCond->name() == "x");
        CHECK(stmt->getCondition() == stmt->condition());

        // Body checks
        REQUIRE(stmt->body() != nullptr);
        auto* block = dynamic_cast<const CompoundStmt*>(stmt->body());
        REQUIRE(block != nullptr);
        CHECK(block->empty());
        CHECK(stmt->getBody() == stmt->body());
        CHECK(stmt->bodyStmt() == stmt->body());
        CHECK(stmt->getBodyStmt() == stmt->body());

        // Location of 'while' keyword
        CHECK(stmt->location().line == 1);
        CHECK(stmt->location().column == 1);

        // String representation
        CHECK(stmt->toString() == "WhileStmt(IdentifierExpression(x), CompoundStmt (0 statements))");
    }

    SECTION("type alias WhileStatement matches WhileStmt") {
        Parser parser = makeParser("while (flag) {}");
        std::unique_ptr<WhileStatement> stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK(stmt->condition() != nullptr);
    }

    SECTION("takeCondition and takeBody transfer ownership") {
        Parser parser = makeParser("while (active) {}");
        auto stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);

        auto cond = stmt->takeCondition();
        REQUIRE(cond != nullptr);
        CHECK(stmt->condition() == nullptr);

        auto body = stmt->takeBody();
        REQUIRE(body != nullptr);
        CHECK(stmt->body() == nullptr);
    }
}

// -----------------------------------------------------------------------------
// 2. While Loop Conditions with Various Expressions
// -----------------------------------------------------------------------------
TEST_CASE("WhileStmt: complex conditions", "[parser][while][expr]") {
    SECTION("relational condition 'while (x > 10) {}'") {
        Parser parser = makeParser("while (x > 10) {}");
        auto stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* bin = dynamic_cast<const BinaryExpr*>(stmt->condition());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == ">");

        auto* leftId = dynamic_cast<const IdentifierExpression*>(bin->left());
        REQUIRE(leftId != nullptr);
        CHECK(leftId->name() == "x");

        auto* rightLit = dynamic_cast<const LiteralExpression*>(bin->right());
        REQUIRE(rightLit != nullptr);
        CHECK(rightLit->kind() == LiteralKind::Integer);
        CHECK(rightLit->value() == "10");
    }

    SECTION("compound arithmetic & relational 'while (x + 1 < y * 2) {}'") {
        Parser parser = makeParser("while (x + 1 < y * 2) {}");
        auto stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* rel = dynamic_cast<const BinaryExpr*>(stmt->condition());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");

        auto* add = dynamic_cast<const BinaryExpr*>(rel->left());
        REQUIRE(add != nullptr);
        CHECK(add->op() == "+");

        auto* mul = dynamic_cast<const BinaryExpr*>(rel->right());
        REQUIRE(mul != nullptr);
        CHECK(mul->op() == "*");
    }

    SECTION("equality condition 'while (count != 0) {}'") {
        Parser parser = makeParser("while (count != 0) {}");
        auto stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* bin = dynamic_cast<const BinaryExpr*>(stmt->condition());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "!=");
    }

    SECTION("bitwise condition 'while (x & 1) {}'") {
        Parser parser = makeParser("while (x & 1) {}");
        auto stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* bin = dynamic_cast<const BinaryExpr*>(stmt->condition());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "&");
    }

    SECTION("unary condition 'while (!done) {}'") {
        Parser parser = makeParser("while (!done) {}");
        auto stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* un = dynamic_cast<const UnaryExpr*>(stmt->condition());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "!");
    }
}

// -----------------------------------------------------------------------------
// 3. Nested While Loops and Loop Body Varieties
// -----------------------------------------------------------------------------
TEST_CASE("WhileStmt: nested loops and bodies", "[parser][while][nesting]") {
    SECTION("unbraced nested while loop 'while (a) while (b) {}'") {
        Parser parser = makeParser("while (a) while (b) {}");
        auto stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        // Outer condition
        auto* outerCond = dynamic_cast<const IdentifierExpression*>(stmt->condition());
        REQUIRE(outerCond != nullptr);
        CHECK(outerCond->name() == "a");

        // Inner while as body
        auto* innerWhile = dynamic_cast<const WhileStmt*>(stmt->body());
        REQUIRE(innerWhile != nullptr);

        auto* innerCond = dynamic_cast<const IdentifierExpression*>(innerWhile->condition());
        REQUIRE(innerCond != nullptr);
        CHECK(innerCond->name() == "b");

        auto* innerBody = dynamic_cast<const CompoundStmt*>(innerWhile->body());
        REQUIRE(innerBody != nullptr);
        CHECK(innerBody->empty());
    }

    SECTION("braced nested while loop 'while (a) { while (b) {} }'") {
        Parser parser = makeParser("while (a) { while (b) {} }");
        auto stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* outerBlock = dynamic_cast<const CompoundStmt*>(stmt->body());
        REQUIRE(outerBlock != nullptr);
        REQUIRE(outerBlock->statementCount() == 1);

        auto* innerWhile = dynamic_cast<const WhileStmt*>(outerBlock->statements()[0].get());
        REQUIRE(innerWhile != nullptr);
        CHECK(innerWhile->condition()->toString() == "IdentifierExpression(b)");

        auto* innerBlock = dynamic_cast<const CompoundStmt*>(innerWhile->body());
        REQUIRE(innerBlock != nullptr);
        CHECK(innerBlock->empty());
    }

    SECTION("while loop containing if/else in body: 'while (x) if (y) {} else {}'") {
        Parser parser = makeParser("while (x) if (y) {} else {}");
        auto stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* ifStmt = dynamic_cast<const IfStmt*>(stmt->body());
        REQUIRE(ifStmt != nullptr);
        CHECK(ifStmt->hasElse());
        CHECK(ifStmt->condition()->toString() == "IdentifierExpression(y)");
    }

    SECTION("while loop containing if/else in braced body") {
        Parser parser = makeParser("while (running) { if (ready) {} else {} }");
        auto stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* block = dynamic_cast<const CompoundStmt*>(stmt->body());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 1);

        auto* ifStmt = dynamic_cast<const IfStmt*>(block->statements()[0].get());
        REQUIRE(ifStmt != nullptr);
        CHECK(ifStmt->hasElse());
    }

    SECTION("if statement containing while loop: 'if (flag) while (n > 0) {}'") {
        Parser parser = makeParser("if (flag) while (n > 0) {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* ifStmt = dynamic_cast<const IfStmt*>(stmt.get());
        REQUIRE(ifStmt != nullptr);

        auto* whileStmt = dynamic_cast<const WhileStmt*>(ifStmt->thenBranch());
        REQUIRE(whileStmt != nullptr);
        CHECK(whileStmt->condition()->toString() == "BinaryExpr(>, IdentifierExpression(n), LiteralExpression(Integer, 0))");
    }

    SECTION("sibling while loops inside compound statement '{ while (a) {} while (b) {} }'") {
        Parser parser = makeParser("{ while (a) {} while (b) {} }");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* block = dynamic_cast<const CompoundStmt*>(stmt.get());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 2);

        auto* w1 = dynamic_cast<const WhileStmt*>(block->statements()[0].get());
        REQUIRE(w1 != nullptr);
        CHECK(w1->condition()->toString() == "IdentifierExpression(a)");

        auto* w2 = dynamic_cast<const WhileStmt*>(block->statements()[1].get());
        REQUIRE(w2 != nullptr);
        CHECK(w2->condition()->toString() == "IdentifierExpression(b)");
    }
}

// -----------------------------------------------------------------------------
// 4. Source Location Tracking
// -----------------------------------------------------------------------------
TEST_CASE("WhileStmt: source location", "[parser][while][location]") {
    SECTION("while on line 1, column 1") {
        Parser parser = makeParser("while (1) {}");
        auto stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK(stmt->location().line == 1);
        CHECK(stmt->location().column == 1);
    }

    SECTION("while with leading whitespace and newline") {
        Parser parser = makeParser("\n  while (1) {}");
        auto stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK(stmt->location().line == 2);
        CHECK(stmt->location().column == 3);
    }
}

// -----------------------------------------------------------------------------
// 5. ASTPrinter Formatting
// -----------------------------------------------------------------------------
TEST_CASE("WhileStmt: ASTPrinter output", "[parser][while][printer]") {
    SECTION("simple 'while (x > 10) {}'") {
        Parser parser = makeParser("while (x > 10) {}");
        auto stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);

        std::string expected =
            "WhileStmt\n"
            "  Condition:\n"
            "    BinaryExpr: >\n"
            "      Left:\n"
            "        IdentifierExpression: x\n"
            "      Right:\n"
            "        LiteralExpression (Integer): 10\n"
            "  Body:\n"
            "    CompoundStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }

    SECTION("nested 'while (a) while (b) {}'") {
        Parser parser = makeParser("while (a) while (b) {}");
        auto stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);

        std::string expected =
            "WhileStmt\n"
            "  Condition:\n"
            "    IdentifierExpression: a\n"
            "  Body:\n"
            "    WhileStmt\n"
            "      Condition:\n"
            "        IdentifierExpression: b\n"
            "      Body:\n"
            "        CompoundStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }

    SECTION("while with if-else in body") {
        Parser parser = makeParser("while (x) if (y) {} else {}");
        auto stmt = parser.parseWhileStatement();
        REQUIRE(stmt != nullptr);

        std::string expected =
            "WhileStmt\n"
            "  Condition:\n"
            "    IdentifierExpression: x\n"
            "  Body:\n"
            "    IfStmt\n"
            "      Condition:\n"
            "        IdentifierExpression: y\n"
            "      Then:\n"
            "        CompoundStmt\n"
            "      Else:\n"
            "        CompoundStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }
}

// -----------------------------------------------------------------------------
// 6. Error Handling and Syntax Errors
// -----------------------------------------------------------------------------
TEST_CASE("WhileStmt: error handling and syntax errors", "[parser][while][error]") {
    SECTION("missing '(' after 'while': 'while x > 10) {}'") {
        Parser parser = makeParser("while x > 10) {}");
        auto stmt = parser.parseWhileStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '(' after 'while'") != std::string::npos);
    }

    SECTION("missing ')' after condition: 'while (x > 10 {}'") {
        Parser parser = makeParser("while (x > 10 {}");
        auto stmt = parser.parseWhileStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ')' after 'while' condition") != std::string::npos);
    }

    SECTION("missing condition inside parens: 'while () {}'") {
        Parser parser = makeParser("while () {}");
        auto stmt = parser.parseWhileStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression in 'while' condition") != std::string::npos);
    }

    SECTION("missing body at EOF: 'while (x)'") {
        Parser parser = makeParser("while (x)");
        auto stmt = parser.parseWhileStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement after 'while' condition") != std::string::npos);
    }

    SECTION("unclosed compound body: 'while (x) {'") {
        Parser parser = makeParser("while (x) {");
        auto stmt = parser.parseWhileStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '}' to close compound statement") != std::string::npos);
    }

    SECTION("not starting with while keyword") {
        Parser parser = makeParser("for (x; y; z) {}");
        auto stmt = parser.parseWhileStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected 'while'") != std::string::npos);
    }

    SECTION("parseStatement with unrecognized token inside while body") {
        Parser parser = makeParser("while (x) ;");
        auto stmt = parser.parseWhileStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement") != std::string::npos);
    }
}
