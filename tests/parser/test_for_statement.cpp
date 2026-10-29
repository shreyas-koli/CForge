#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/compound_stmt.h"
#include "ast/do_while_stmt.h"
#include "ast/for_stmt.h"
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
// DAY 33 TESTS — For Loop (iteration-statement: 'for' '(' expr? ';' expr? ';' expr? ')' stmt)
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Basic For Loop Parsing with All Clauses Present
// -----------------------------------------------------------------------------
TEST_CASE("ForStmt: basic full clauses", "[parser][for][stmt]") {
    SECTION("standard loop 'for (i = 0; i < 10; i++) {}'") {
        Parser parser = makeParser("for (i = 0; i < 10; i++) {}");
        auto stmt = parser.parseForStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        // Initializer checks
        REQUIRE(stmt->hasInitializer());
        REQUIRE(stmt->hasInit());
        REQUIRE(stmt->initializer() != nullptr);
        CHECK(stmt->getInitializer() == stmt->initializer());
        CHECK(stmt->init() == stmt->initializer());
        CHECK(stmt->getInit() == stmt->initializer());
        auto* initAssign = dynamic_cast<const AssignExpr*>(stmt->initializer());
        REQUIRE(initAssign != nullptr);
        CHECK(initAssign->op() == "=");

        // Condition checks
        REQUIRE(stmt->hasCondition());
        REQUIRE(stmt->condition() != nullptr);
        CHECK(stmt->getCondition() == stmt->condition());
        auto* condBin = dynamic_cast<const BinaryExpr*>(stmt->condition());
        REQUIRE(condBin != nullptr);
        CHECK(condBin->op() == "<");

        // Post expression checks
        REQUIRE(stmt->hasPost());
        REQUIRE(stmt->hasPostExpr());
        REQUIRE(stmt->post() != nullptr);
        CHECK(stmt->getPost() == stmt->post());
        CHECK(stmt->postExpr() == stmt->post());
        CHECK(stmt->getPostExpr() == stmt->post());
        auto* postUn = dynamic_cast<const UnaryExpr*>(stmt->post());
        REQUIRE(postUn != nullptr);
        CHECK(postUn->op() == "++");

        // Body checks
        REQUIRE(stmt->body() != nullptr);
        auto* block = dynamic_cast<const CompoundStmt*>(stmt->body());
        REQUIRE(block != nullptr);
        CHECK(block->empty());
        CHECK(stmt->getBody() == stmt->body());
        CHECK(stmt->bodyStmt() == stmt->body());
        CHECK(stmt->getBodyStmt() == stmt->body());

        // Source location
        CHECK(stmt->location().line == 1);
        CHECK(stmt->location().column == 1);
    }

    SECTION("type alias ForStatement matches ForStmt") {
        Parser parser = makeParser("for (;;);");
        // Semicolon alone is not supported as statement yet, so use {}
        Parser parserBlock = makeParser("for (;;) {}");
        std::unique_ptr<ForStatement> stmt = parserBlock.parseForStatement();
        REQUIRE(stmt != nullptr);
    }

    SECTION("take accessors transfer ownership") {
        Parser parser = makeParser("for (a = 1; b < 2; c++) {}");
        auto stmt = parser.parseForStatement();
        REQUIRE(stmt != nullptr);

        auto init = stmt->takeInitializer();
        REQUIRE(init != nullptr);
        CHECK(stmt->initializer() == nullptr);

        auto cond = stmt->takeCondition();
        REQUIRE(cond != nullptr);
        CHECK(stmt->condition() == nullptr);

        auto post = stmt->takePost();
        REQUIRE(post != nullptr);
        CHECK(stmt->post() == nullptr);

        auto body = stmt->takeBody();
        REQUIRE(body != nullptr);
        CHECK(stmt->body() == nullptr);
    }
}

// -----------------------------------------------------------------------------
// 2. Omitted Clauses Variations
// -----------------------------------------------------------------------------
TEST_CASE("ForStmt: omitted clauses", "[parser][for][omitted]") {
    SECTION("all clauses omitted: 'for (;;) {}'") {
        Parser parser = makeParser("for (;;) {}");
        auto stmt = parser.parseForStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK_FALSE(stmt->hasInitializer());
        CHECK(stmt->initializer() == nullptr);
        CHECK_FALSE(stmt->hasCondition());
        CHECK(stmt->condition() == nullptr);
        CHECK_FALSE(stmt->hasPost());
        CHECK(stmt->post() == nullptr);

        REQUIRE(stmt->body() != nullptr);
        auto* block = dynamic_cast<const CompoundStmt*>(stmt->body());
        REQUIRE(block != nullptr);
        CHECK(block->empty());

        CHECK(stmt->toString() == "ForStmt(null, null, null, CompoundStmt (0 statements))");
    }

    SECTION("condition and post omitted: 'for (i = 0;;) {}'") {
        Parser parser = makeParser("for (i = 0;;) {}");
        auto stmt = parser.parseForStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(stmt->hasInitializer());
        CHECK(stmt->initializer() != nullptr);
        CHECK_FALSE(stmt->hasCondition());
        CHECK(stmt->condition() == nullptr);
        CHECK_FALSE(stmt->hasPost());
        CHECK(stmt->post() == nullptr);
    }

    SECTION("initializer and post omitted: 'for (; i < 10;) {}'") {
        Parser parser = makeParser("for (; i < 10;) {}");
        auto stmt = parser.parseForStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK_FALSE(stmt->hasInitializer());
        CHECK(stmt->initializer() == nullptr);
        CHECK(stmt->hasCondition());
        CHECK(stmt->condition() != nullptr);
        CHECK_FALSE(stmt->hasPost());
        CHECK(stmt->post() == nullptr);
    }

    SECTION("initializer and condition omitted: 'for (;; i++) {}'") {
        Parser parser = makeParser("for (;; i++) {}");
        auto stmt = parser.parseForStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK_FALSE(stmt->hasInitializer());
        CHECK(stmt->initializer() == nullptr);
        CHECK_FALSE(stmt->hasCondition());
        CHECK(stmt->condition() == nullptr);
        CHECK(stmt->hasPost());
        CHECK(stmt->post() != nullptr);
    }

    SECTION("only post omitted: 'for (i = 0; i < 10;) {}'") {
        Parser parser = makeParser("for (i = 0; i < 10;) {}");
        auto stmt = parser.parseForStatement();
        REQUIRE(stmt != nullptr);
        CHECK(stmt->hasInitializer());
        CHECK(stmt->hasCondition());
        CHECK_FALSE(stmt->hasPost());
    }

    SECTION("only condition omitted: 'for (i = 0;; i++) {}'") {
        Parser parser = makeParser("for (i = 0;; i++) {}");
        auto stmt = parser.parseForStatement();
        REQUIRE(stmt != nullptr);
        CHECK(stmt->hasInitializer());
        CHECK_FALSE(stmt->hasCondition());
        CHECK(stmt->hasPost());
    }

    SECTION("only initializer omitted: 'for (; i < 10; i++) {}'") {
        Parser parser = makeParser("for (; i < 10; i++) {}");
        auto stmt = parser.parseForStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(stmt->hasInitializer());
        CHECK(stmt->hasCondition());
        CHECK(stmt->hasPost());
    }
}

// -----------------------------------------------------------------------------
// 3. Nesting & Compound Bodies
// -----------------------------------------------------------------------------
TEST_CASE("ForStmt: nesting and statement hierarchy", "[parser][for][nesting]") {
    SECTION("nested for loops unbraced: 'for (;;) for (;;) {}'") {
        Parser parser = makeParser("for (;;) for (;;) {}");
        auto stmt = parser.parseForStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* innerFor = dynamic_cast<const ForStmt*>(stmt->body());
        REQUIRE(innerFor != nullptr);
        CHECK_FALSE(innerFor->hasInitializer());
        CHECK_FALSE(innerFor->hasCondition());
        CHECK_FALSE(innerFor->hasPost());
    }

    SECTION("nested for loops braced: 'for (i = 0; i < 5; i++) { for (j = 0; j < 5; j++) {} }'") {
        Parser parser = makeParser("for (i = 0; i < 5; i++) { for (j = 0; j < 5; j++) {} }");
        auto stmt = parser.parseForStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* block = dynamic_cast<const CompoundStmt*>(stmt->body());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 1);

        auto* innerFor = dynamic_cast<const ForStmt*>(block->statements()[0].get());
        REQUIRE(innerFor != nullptr);
        CHECK(innerFor->hasInitializer());
        CHECK(innerFor->hasCondition());
        CHECK(innerFor->hasPost());
    }

    SECTION("for loop inside while body: 'while (cond) for (;;) {}'") {
        Parser parser = makeParser("while (cond) for (;;) {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt.get());
        REQUIRE(whileStmt != nullptr);

        auto* forStmt = dynamic_cast<const ForStmt*>(whileStmt->body());
        REQUIRE(forStmt != nullptr);
        CHECK_FALSE(forStmt->hasCondition());
    }

    SECTION("for loop inside if/else branches") {
        Parser parser = makeParser("if (c) for (;;) {} else for (;;) {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* ifStmt = dynamic_cast<const IfStmt*>(stmt.get());
        REQUIRE(ifStmt != nullptr);

        auto* thenFor = dynamic_cast<const ForStmt*>(ifStmt->thenBranch());
        REQUIRE(thenFor != nullptr);

        auto* elseFor = dynamic_cast<const ForStmt*>(ifStmt->elseBranch());
        REQUIRE(elseFor != nullptr);
    }

    SECTION("for loop inside compound statement '{ for (;;) {} }'") {
        Parser parser = makeParser("{ for (;;) {} }");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* block = dynamic_cast<const CompoundStmt*>(stmt.get());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 1);

        auto* forStmt = dynamic_cast<const ForStmt*>(block->statements()[0].get());
        REQUIRE(forStmt != nullptr);
    }
}

// -----------------------------------------------------------------------------
// 4. Source Location Tracking
// -----------------------------------------------------------------------------
TEST_CASE("ForStmt: source location", "[parser][for][location]") {
    SECTION("for keyword on line 1, column 1") {
        Parser parser = makeParser("for (;;) {}");
        auto stmt = parser.parseForStatement();
        REQUIRE(stmt != nullptr);
        CHECK(stmt->location().line == 1);
        CHECK(stmt->location().column == 1);
    }

    SECTION("for keyword with leading whitespace and newline") {
        Parser parser = makeParser("\n  for (;;) {}");
        auto stmt = parser.parseForStatement();
        REQUIRE(stmt != nullptr);
        CHECK(stmt->location().line == 2);
        CHECK(stmt->location().column == 3);
    }
}

// -----------------------------------------------------------------------------
// 5. ASTPrinter Output Formatting
// -----------------------------------------------------------------------------
TEST_CASE("ForStmt: ASTPrinter output", "[parser][for][printer]") {
    SECTION("all clauses omitted: 'for (;;) {}'") {
        Parser parser = makeParser("for (;;) {}");
        auto stmt = parser.parseForStatement();
        REQUIRE(stmt != nullptr);

        std::string expected =
            "ForStmt\n"
            "  Initializer:\n"
            "    (omitted)\n"
            "  Condition:\n"
            "    (omitted)\n"
            "  Post:\n"
            "    (omitted)\n"
            "  Body:\n"
            "    CompoundStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }

    SECTION("all clauses present: 'for (i = 0; i < 10; i++) {}'") {
        Parser parser = makeParser("for (i = 0; i < 10; i++) {}");
        auto stmt = parser.parseForStatement();
        REQUIRE(stmt != nullptr);

        std::string expected =
            "ForStmt\n"
            "  Initializer:\n"
            "    AssignExpr: =\n"
            "      Left:\n"
            "        IdentifierExpression: i\n"
            "      Right:\n"
            "        LiteralExpression (Integer): 0\n"
            "  Condition:\n"
            "    BinaryExpr: <\n"
            "      Left:\n"
            "        IdentifierExpression: i\n"
            "      Right:\n"
            "        LiteralExpression (Integer): 10\n"
            "  Post:\n"
            "    UnaryExpr: ++\n"
            "      Operand:\n"
            "        IdentifierExpression: i\n"
            "  Body:\n"
            "    CompoundStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }

    SECTION("partially omitted: 'for (; i < 10;) {}'") {
        Parser parser = makeParser("for (; i < 10;) {}");
        auto stmt = parser.parseForStatement();
        REQUIRE(stmt != nullptr);

        std::string expected =
            "ForStmt\n"
            "  Initializer:\n"
            "    (omitted)\n"
            "  Condition:\n"
            "    BinaryExpr: <\n"
            "      Left:\n"
            "        IdentifierExpression: i\n"
            "      Right:\n"
            "        LiteralExpression (Integer): 10\n"
            "  Post:\n"
            "    (omitted)\n"
            "  Body:\n"
            "    CompoundStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }
}

// -----------------------------------------------------------------------------
// 6. Error Handling and Syntax Errors
// -----------------------------------------------------------------------------
TEST_CASE("ForStmt: error handling and syntax errors", "[parser][for][error]") {
    SECTION("missing '(' after 'for': 'for ;;) {}'") {
        Parser parser = makeParser("for ;;) {}");
        auto stmt = parser.parseForStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '(' after 'for'") != std::string::npos);
    }

    SECTION("missing first semicolon: 'for (i = 0 i < 10; i++) {}'") {
        Parser parser = makeParser("for (i = 0 i < 10; i++) {}");
        auto stmt = parser.parseForStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';' after 'for' initializer") != std::string::npos);
    }

    SECTION("missing second semicolon: 'for (i = 0; i < 10) {}'") {
        Parser parser = makeParser("for (i = 0; i < 10) {}");
        auto stmt = parser.parseForStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';' after 'for' condition") != std::string::npos);
    }

    SECTION("missing ')' after clauses: 'for (i = 0; i < 10; i++ {}'") {
        Parser parser = makeParser("for (i = 0; i < 10; i++ {}");
        auto stmt = parser.parseForStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ')' after 'for' clauses") != std::string::npos);
    }

    SECTION("missing body at EOF: 'for (;;)'") {
        Parser parser = makeParser("for (;;)");
        auto stmt = parser.parseForStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement in 'for' loop body") != std::string::npos);
    }

    SECTION("not starting with 'for' keyword") {
        Parser parser = makeParser("while (1) {}");
        auto stmt = parser.parseForStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected 'for'") != std::string::npos);
    }
}

// -----------------------------------------------------------------------------
// 7. Non-Regression: existing while, do-while, and if/else remain unchanged
// -----------------------------------------------------------------------------
TEST_CASE("ForStmt: regression check for other statements", "[parser][for][nonregression]") {
    SECTION("while (x) {} still works") {
        Parser parser = makeParser("while (x) {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK(dynamic_cast<const WhileStmt*>(stmt.get()) != nullptr);
    }

    SECTION("do {} while (x); still works") {
        Parser parser = makeParser("do {} while (x);");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK(dynamic_cast<const DoWhileStmt*>(stmt.get()) != nullptr);
    }

    SECTION("if (x) {} else {} still works") {
        Parser parser = makeParser("if (x) {} else {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK(dynamic_cast<const IfStmt*>(stmt.get()) != nullptr);
    }
}

