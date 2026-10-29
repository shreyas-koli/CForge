#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/compound_stmt.h"
#include "ast/do_while_stmt.h"
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
// DAY 32 TESTS — Do-While Loop (iteration-statement: 'do' stmt 'while' '(' expr ')' ';')
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Basic Do-While Loop Parsing
// -----------------------------------------------------------------------------
TEST_CASE("DoWhileStmt: basic parsing", "[parser][do_while][stmt]") {
    SECTION("simple identifier condition 'do {} while (x);'") {
        Parser parser = makeParser("do {} while (x);");
        auto stmt = parser.parseDoWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        // Body checks
        REQUIRE(stmt->body() != nullptr);
        auto* block = dynamic_cast<const CompoundStmt*>(stmt->body());
        REQUIRE(block != nullptr);
        CHECK(block->empty());
        CHECK(stmt->getBody() == stmt->body());
        CHECK(stmt->bodyStmt() == stmt->body());
        CHECK(stmt->getBodyStmt() == stmt->body());

        // Condition checks
        REQUIRE(stmt->condition() != nullptr);
        auto* idCond = dynamic_cast<const IdentifierExpression*>(stmt->condition());
        REQUIRE(idCond != nullptr);
        CHECK(idCond->name() == "x");
        CHECK(stmt->getCondition() == stmt->condition());

        // Location of 'do' keyword
        CHECK(stmt->location().line == 1);
        CHECK(stmt->location().column == 1);

        // String representation
        CHECK(stmt->toString() == "DoWhileStmt(CompoundStmt (0 statements), IdentifierExpression(x))");
    }

    SECTION("relational condition 'do {} while (x < 10);'") {
        Parser parser = makeParser("do {} while (x < 10);");
        auto stmt = parser.parseDoWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        REQUIRE(stmt->body() != nullptr);
        auto* block = dynamic_cast<const CompoundStmt*>(stmt->body());
        REQUIRE(block != nullptr);
        CHECK(block->empty());

        REQUIRE(stmt->condition() != nullptr);
        auto* bin = dynamic_cast<const BinaryExpr*>(stmt->condition());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "<");

        auto* leftId = dynamic_cast<const IdentifierExpression*>(bin->left());
        REQUIRE(leftId != nullptr);
        CHECK(leftId->name() == "x");

        auto* rightLit = dynamic_cast<const LiteralExpression*>(bin->right());
        REQUIRE(rightLit != nullptr);
        CHECK(rightLit->kind() == LiteralKind::Integer);
        CHECK(rightLit->value() == "10");
    }

    SECTION("type alias DoWhileStatement matches DoWhileStmt") {
        Parser parser = makeParser("do {} while (1);");
        std::unique_ptr<DoWhileStatement> stmt = parser.parseDoWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK(stmt->condition() != nullptr);
    }

    SECTION("takeBody and takeCondition transfer ownership") {
        Parser parser = makeParser("do {} while (flag);");
        auto stmt = parser.parseDoWhileStatement();
        REQUIRE(stmt != nullptr);

        auto body = stmt->takeBody();
        REQUIRE(body != nullptr);
        CHECK(stmt->body() == nullptr);

        auto cond = stmt->takeCondition();
        REQUIRE(cond != nullptr);
        CHECK(stmt->condition() == nullptr);
    }

    SECTION("takeBodyStmt transfers ownership") {
        Parser parser = makeParser("do {} while (flag);");
        auto stmt = parser.parseDoWhileStatement();
        REQUIRE(stmt != nullptr);

        auto body = stmt->takeBodyStmt();
        REQUIRE(body != nullptr);
        CHECK(stmt->body() == nullptr);
    }
}

// -----------------------------------------------------------------------------
// 2. Loop Bodies with Various Supported Statements
// -----------------------------------------------------------------------------
TEST_CASE("DoWhileStmt: loop bodies with supported statements", "[parser][do_while][body]") {
    SECTION("body containing while loop: 'do while (y) {} while (x);'") {
        Parser parser = makeParser("do while (y) {} while (x);");
        auto stmt = parser.parseDoWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* innerWhile = dynamic_cast<const WhileStmt*>(stmt->body());
        REQUIRE(innerWhile != nullptr);
        CHECK(innerWhile->condition()->toString() == "IdentifierExpression(y)");

        auto* outerCond = dynamic_cast<const IdentifierExpression*>(stmt->condition());
        REQUIRE(outerCond != nullptr);
        CHECK(outerCond->name() == "x");
    }

    SECTION("body containing if/else statement: 'do if (a) {} else {} while (b);'") {
        Parser parser = makeParser("do if (a) {} else {} while (b);");
        auto stmt = parser.parseDoWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* ifStmt = dynamic_cast<const IfStmt*>(stmt->body());
        REQUIRE(ifStmt != nullptr);
        CHECK(ifStmt->hasElse());
        CHECK(ifStmt->condition()->toString() == "IdentifierExpression(a)");

        auto* outerCond = dynamic_cast<const IdentifierExpression*>(stmt->condition());
        REQUIRE(outerCond != nullptr);
        CHECK(outerCond->name() == "b");
    }

    SECTION("body containing compound statement with multiple statements") {
        Parser parser = makeParser("do { while (a) {} if (b) {} } while (c);");
        auto stmt = parser.parseDoWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* block = dynamic_cast<const CompoundStmt*>(stmt->body());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 2);

        auto* innerWhile = dynamic_cast<const WhileStmt*>(block->statements()[0].get());
        REQUIRE(innerWhile != nullptr);

        auto* innerIf = dynamic_cast<const IfStmt*>(block->statements()[1].get());
        REQUIRE(innerIf != nullptr);
    }
}

// -----------------------------------------------------------------------------
// 3. Nested Do-While Loops & Enclosing Statements
// -----------------------------------------------------------------------------
TEST_CASE("DoWhileStmt: nesting and hierarchy", "[parser][do_while][nesting]") {
    SECTION("nested do-while loops unbraced: 'do do {} while (b); while (a);'") {
        Parser parser = makeParser("do do {} while (b); while (a);");
        auto stmt = parser.parseDoWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* innerDoWhile = dynamic_cast<const DoWhileStmt*>(stmt->body());
        REQUIRE(innerDoWhile != nullptr);
        CHECK(innerDoWhile->condition()->toString() == "IdentifierExpression(b)");

        auto* outerCond = dynamic_cast<const IdentifierExpression*>(stmt->condition());
        REQUIRE(outerCond != nullptr);
        CHECK(outerCond->name() == "a");
    }

    SECTION("nested do-while loops braced: 'do { do {} while (b); } while (a);'") {
        Parser parser = makeParser("do { do {} while (b); } while (a);");
        auto stmt = parser.parseDoWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* block = dynamic_cast<const CompoundStmt*>(stmt->body());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 1);

        auto* innerDoWhile = dynamic_cast<const DoWhileStmt*>(block->statements()[0].get());
        REQUIRE(innerDoWhile != nullptr);
        CHECK(innerDoWhile->condition()->toString() == "IdentifierExpression(b)");
    }

    SECTION("do-while inside a while loop: 'while (x) do {} while (y);'") {
        Parser parser = makeParser("while (x) do {} while (y);");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt.get());
        REQUIRE(whileStmt != nullptr);

        auto* doWhileStmt = dynamic_cast<const DoWhileStmt*>(whileStmt->body());
        REQUIRE(doWhileStmt != nullptr);
        CHECK(doWhileStmt->condition()->toString() == "IdentifierExpression(y)");
    }

    SECTION("do-while inside an if/else: 'if (c) do {} while (x); else do {} while (y);'") {
        Parser parser = makeParser("if (c) do {} while (x); else do {} while (y);");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* ifStmt = dynamic_cast<const IfStmt*>(stmt.get());
        REQUIRE(ifStmt != nullptr);

        auto* thenDo = dynamic_cast<const DoWhileStmt*>(ifStmt->thenBranch());
        REQUIRE(thenDo != nullptr);
        CHECK(thenDo->condition()->toString() == "IdentifierExpression(x)");

        auto* elseDo = dynamic_cast<const DoWhileStmt*>(ifStmt->elseBranch());
        REQUIRE(elseDo != nullptr);
        CHECK(elseDo->condition()->toString() == "IdentifierExpression(y)");
    }
}

// -----------------------------------------------------------------------------
// 4. Source Location Tracking
// -----------------------------------------------------------------------------
TEST_CASE("DoWhileStmt: source location", "[parser][do_while][location]") {
    SECTION("do keyword on line 1, column 1") {
        Parser parser = makeParser("do {} while (1);");
        auto stmt = parser.parseDoWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK(stmt->location().line == 1);
        CHECK(stmt->location().column == 1);
    }

    SECTION("do keyword with leading whitespace and newline") {
        Parser parser = makeParser("\n   do {} while (1);");
        auto stmt = parser.parseDoWhileStatement();
        REQUIRE(stmt != nullptr);
        CHECK(stmt->location().line == 2);
        CHECK(stmt->location().column == 4);
    }
}

// -----------------------------------------------------------------------------
// 5. ASTPrinter Formatting
// -----------------------------------------------------------------------------
TEST_CASE("DoWhileStmt: ASTPrinter output", "[parser][do_while][printer]") {
    SECTION("simple 'do {} while (x < 10);'") {
        Parser parser = makeParser("do {} while (x < 10);");
        auto stmt = parser.parseDoWhileStatement();
        REQUIRE(stmt != nullptr);

        std::string expected =
            "DoWhileStmt\n"
            "  Body:\n"
            "    CompoundStmt\n"
            "  Condition:\n"
            "    BinaryExpr: <\n"
            "      Left:\n"
            "        IdentifierExpression: x\n"
            "      Right:\n"
            "        LiteralExpression (Integer): 10\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }

    SECTION("nested 'do do {} while (b); while (a);'") {
        Parser parser = makeParser("do do {} while (b); while (a);");
        auto stmt = parser.parseDoWhileStatement();
        REQUIRE(stmt != nullptr);

        std::string expected =
            "DoWhileStmt\n"
            "  Body:\n"
            "    DoWhileStmt\n"
            "      Body:\n"
            "        CompoundStmt\n"
            "      Condition:\n"
            "        IdentifierExpression: b\n"
            "  Condition:\n"
            "    IdentifierExpression: a\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }
}

// -----------------------------------------------------------------------------
// 6. Error Handling and Negative Tests
// -----------------------------------------------------------------------------
TEST_CASE("DoWhileStmt: error handling and syntax errors", "[parser][do_while][error]") {
    SECTION("missing body at EOF: 'do'") {
        Parser parser = makeParser("do");
        auto stmt = parser.parseDoWhileStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement") != std::string::npos);
    }

    SECTION("missing 'while' keyword after body: 'do {}'") {
        Parser parser = makeParser("do {}");
        auto stmt = parser.parseDoWhileStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected 'while' after 'do' body") != std::string::npos);
    }

    SECTION("missing '(' after 'while': 'do {} while x < 10);'") {
        Parser parser = makeParser("do {} while x < 10);");
        auto stmt = parser.parseDoWhileStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '(' after 'while' in do-while loop") != std::string::npos);
    }

    SECTION("missing condition expression inside parens: 'do {} while ();'") {
        Parser parser = makeParser("do {} while ();");
        auto stmt = parser.parseDoWhileStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression in 'do-while' condition") != std::string::npos);
    }

    SECTION("missing ')' after condition: 'do {} while (x < 10;'") {
        Parser parser = makeParser("do {} while (x < 10;");
        auto stmt = parser.parseDoWhileStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ')' after 'do-while' condition") != std::string::npos);
    }

    SECTION("missing trailing ';' after ')': 'do {} while (x < 10)'") {
        Parser parser = makeParser("do {} while (x < 10)");
        auto stmt = parser.parseDoWhileStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';' after 'do-while' statement") != std::string::npos);
    }

    SECTION("not starting with 'do' keyword") {
        Parser parser = makeParser("while (x) {}");
        auto stmt = parser.parseDoWhileStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected 'do'") != std::string::npos);
    }
}

// -----------------------------------------------------------------------------
// 7. Non-Regression: Ordinary while loop parsing remains unchanged
// -----------------------------------------------------------------------------
TEST_CASE("DoWhileStmt: ordinary while loop regression check", "[parser][while][nonregression]") {
    SECTION("ordinary while (x) {} parses as WhileStmt") {
        Parser parser = makeParser("while (x) {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt.get());
        REQUIRE(whileStmt != nullptr);
        CHECK(whileStmt->condition()->toString() == "IdentifierExpression(x)");
    }
}

