#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/break_stmt.h"
#include "ast/continue_stmt.h"
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
// DAY 34 TESTS — Break and Continue Jump Statements
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Basic Break and Continue Parsing
// -----------------------------------------------------------------------------
TEST_CASE("JumpStmt: basic break parsing", "[parser][break][jump]") {
    SECTION("parseBreakStatement 'break;'") {
        Parser parser = makeParser("break;");
        auto stmt = parser.parseBreakStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(stmt->location().line == 1);
        CHECK(stmt->location().column == 1);
        CHECK(stmt->toString() == "BreakStmt");
    }

    SECTION("parseStatement 'break;'") {
        Parser parser = makeParser("break;");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* breakStmt = dynamic_cast<const BreakStmt*>(stmt.get());
        REQUIRE(breakStmt != nullptr);
    }

    SECTION("type alias BreakStatement matches BreakStmt") {
        Parser parser = makeParser("break;");
        std::unique_ptr<BreakStatement> stmt = parser.parseBreakStatement();
        REQUIRE(stmt != nullptr);
    }

    SECTION("break parses without loop context (parsing-only contract)") {
        // Semantic analysis will check loop context later; parser allows jump statements
        Parser parser = makeParser("break;");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
    }
}

TEST_CASE("JumpStmt: basic continue parsing", "[parser][continue][jump]") {
    SECTION("parseContinueStatement 'continue;'") {
        Parser parser = makeParser("continue;");
        auto stmt = parser.parseContinueStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(stmt->location().line == 1);
        CHECK(stmt->location().column == 1);
        CHECK(stmt->toString() == "ContinueStmt");
    }

    SECTION("parseStatement 'continue;'") {
        Parser parser = makeParser("continue;");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* contStmt = dynamic_cast<const ContinueStmt*>(stmt.get());
        REQUIRE(contStmt != nullptr);
    }

    SECTION("type alias ContinueStatement matches ContinueStmt") {
        Parser parser = makeParser("continue;");
        std::unique_ptr<ContinueStatement> stmt = parser.parseContinueStatement();
        REQUIRE(stmt != nullptr);
    }

    SECTION("continue parses without loop context (parsing-only contract)") {
        Parser parser = makeParser("continue;");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
    }
}

// -----------------------------------------------------------------------------
// 2. Break and Continue inside Loops
// -----------------------------------------------------------------------------
TEST_CASE("JumpStmt: inside while loops", "[parser][break][continue][while]") {
    SECTION("unbraced while with break 'while (1) break;'") {
        Parser parser = makeParser("while (1) break;");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt.get());
        REQUIRE(whileStmt != nullptr);

        auto* bodyBreak = dynamic_cast<const BreakStmt*>(whileStmt->body());
        REQUIRE(bodyBreak != nullptr);
    }

    SECTION("unbraced while with continue 'while (1) continue;'") {
        Parser parser = makeParser("while (1) continue;");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt.get());
        REQUIRE(whileStmt != nullptr);

        auto* bodyCont = dynamic_cast<const ContinueStmt*>(whileStmt->body());
        REQUIRE(bodyCont != nullptr);
    }

    SECTION("braced while with break 'while (x) { break; }'") {
        Parser parser = makeParser("while (x) { break; }");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt.get());
        REQUIRE(whileStmt != nullptr);

        auto* block = dynamic_cast<const CompoundStmt*>(whileStmt->body());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 1);

        auto* innerBreak = dynamic_cast<const BreakStmt*>(block->statements()[0].get());
        REQUIRE(innerBreak != nullptr);
    }

    SECTION("braced while with continue 'while (x) { continue; }'") {
        Parser parser = makeParser("while (x) { continue; }");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt.get());
        REQUIRE(whileStmt != nullptr);

        auto* block = dynamic_cast<const CompoundStmt*>(whileStmt->body());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 1);

        auto* innerCont = dynamic_cast<const ContinueStmt*>(block->statements()[0].get());
        REQUIRE(innerCont != nullptr);
    }
}

TEST_CASE("JumpStmt: inside do-while loops", "[parser][break][continue][do_while]") {
    SECTION("unbraced do-while with break 'do break; while (1);'") {
        Parser parser = makeParser("do break; while (1);");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* doWhileStmt = dynamic_cast<const DoWhileStmt*>(stmt.get());
        REQUIRE(doWhileStmt != nullptr);

        auto* bodyBreak = dynamic_cast<const BreakStmt*>(doWhileStmt->body());
        REQUIRE(bodyBreak != nullptr);
    }

    SECTION("unbraced do-while with continue 'do continue; while (1);'") {
        Parser parser = makeParser("do continue; while (1);");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* doWhileStmt = dynamic_cast<const DoWhileStmt*>(stmt.get());
        REQUIRE(doWhileStmt != nullptr);

        auto* bodyCont = dynamic_cast<const ContinueStmt*>(doWhileStmt->body());
        REQUIRE(bodyCont != nullptr);
    }

    SECTION("braced do-while with both 'do { break; continue; } while (x);'") {
        Parser parser = makeParser("do { break; continue; } while (x);");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* doWhileStmt = dynamic_cast<const DoWhileStmt*>(stmt.get());
        REQUIRE(doWhileStmt != nullptr);

        auto* block = dynamic_cast<const CompoundStmt*>(doWhileStmt->body());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 2);

        CHECK(dynamic_cast<const BreakStmt*>(block->statements()[0].get()) != nullptr);
        CHECK(dynamic_cast<const ContinueStmt*>(block->statements()[1].get()) != nullptr);
    }
}

TEST_CASE("JumpStmt: inside for loops", "[parser][break][continue][for]") {
    SECTION("unbraced for with break 'for (;;) break;'") {
        Parser parser = makeParser("for (;;) break;");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* forStmt = dynamic_cast<const ForStmt*>(stmt.get());
        REQUIRE(forStmt != nullptr);

        auto* bodyBreak = dynamic_cast<const BreakStmt*>(forStmt->body());
        REQUIRE(bodyBreak != nullptr);
    }

    SECTION("unbraced for with continue 'for (;;) continue;'") {
        Parser parser = makeParser("for (;;) continue;");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* forStmt = dynamic_cast<const ForStmt*>(stmt.get());
        REQUIRE(forStmt != nullptr);

        auto* bodyCont = dynamic_cast<const ContinueStmt*>(forStmt->body());
        REQUIRE(bodyCont != nullptr);
    }

    SECTION("braced for with break and continue") {
        Parser parser = makeParser("for (i = 0; i < 10; i++) { break; continue; }");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* forStmt = dynamic_cast<const ForStmt*>(stmt.get());
        REQUIRE(forStmt != nullptr);

        auto* block = dynamic_cast<const CompoundStmt*>(forStmt->body());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 2);

        CHECK(dynamic_cast<const BreakStmt*>(block->statements()[0].get()) != nullptr);
        CHECK(dynamic_cast<const ContinueStmt*>(block->statements()[1].get()) != nullptr);
    }
}

// -----------------------------------------------------------------------------
// 3. Inside Compound Statements & Nested Blocks
// -----------------------------------------------------------------------------
TEST_CASE("JumpStmt: inside compound statements", "[parser][break][continue][compound]") {
    SECTION("compound with multiple jump statements '{ break; continue; break; }'") {
        Parser parser = makeParser("{ break; continue; break; }");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* block = dynamic_cast<const CompoundStmt*>(stmt.get());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 3);

        CHECK(dynamic_cast<const BreakStmt*>(block->statements()[0].get()) != nullptr);
        CHECK(dynamic_cast<const ContinueStmt*>(block->statements()[1].get()) != nullptr);
        CHECK(dynamic_cast<const BreakStmt*>(block->statements()[2].get()) != nullptr);
    }

    SECTION("deeply nested compound blocks '{ { { break; } } }'") {
        Parser parser = makeParser("{ { { break; } } }");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* b1 = dynamic_cast<const CompoundStmt*>(stmt.get());
        REQUIRE(b1 != nullptr);
        REQUIRE(b1->statementCount() == 1);

        auto* b2 = dynamic_cast<const CompoundStmt*>(b1->statements()[0].get());
        REQUIRE(b2 != nullptr);
        REQUIRE(b2->statementCount() == 1);

        auto* b3 = dynamic_cast<const CompoundStmt*>(b2->statements()[0].get());
        REQUIRE(b3 != nullptr);
        REQUIRE(b3->statementCount() == 1);

        CHECK(dynamic_cast<const BreakStmt*>(b3->statements()[0].get()) != nullptr);
    }
}

// -----------------------------------------------------------------------------
// 4. Nested Loops and Conditionals with Jump Statements
// -----------------------------------------------------------------------------
TEST_CASE("JumpStmt: nested loops and conditionals", "[parser][break][continue][nesting]") {
    SECTION("nested for and while loops with jumps") {
        Parser parser = makeParser("for (;;) while (1) { break; continue; }");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* forStmt = dynamic_cast<const ForStmt*>(stmt.get());
        REQUIRE(forStmt != nullptr);

        auto* whileStmt = dynamic_cast<const WhileStmt*>(forStmt->body());
        REQUIRE(whileStmt != nullptr);

        auto* block = dynamic_cast<const CompoundStmt*>(whileStmt->body());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 2);
    }

    SECTION("while with if-else having break and continue") {
        Parser parser = makeParser("while (x) { if (y) break; else continue; }");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt.get());
        REQUIRE(whileStmt != nullptr);

        auto* block = dynamic_cast<const CompoundStmt*>(whileStmt->body());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 1);

        auto* ifStmt = dynamic_cast<const IfStmt*>(block->statements()[0].get());
        REQUIRE(ifStmt != nullptr);

        auto* thenBreak = dynamic_cast<const BreakStmt*>(ifStmt->thenBranch());
        REQUIRE(thenBreak != nullptr);

        auto* elseCont = dynamic_cast<const ContinueStmt*>(ifStmt->elseBranch());
        REQUIRE(elseCont != nullptr);
    }

    SECTION("do-while with braced if-else containing jumps") {
        Parser parser = makeParser("do { if (flag) { break; } else { continue; } } while (1);");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* doWhileStmt = dynamic_cast<const DoWhileStmt*>(stmt.get());
        REQUIRE(doWhileStmt != nullptr);
    }

    SECTION("nested for loops: outer and inner each with jumps") {
        Parser parser = makeParser("for (;;) { for (;;) { break; } continue; }");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* outerFor = dynamic_cast<const ForStmt*>(stmt.get());
        REQUIRE(outerFor != nullptr);

        auto* outerBlock = dynamic_cast<const CompoundStmt*>(outerFor->body());
        REQUIRE(outerBlock != nullptr);
        REQUIRE(outerBlock->statementCount() == 2);

        auto* innerFor = dynamic_cast<const ForStmt*>(outerBlock->statements()[0].get());
        REQUIRE(innerFor != nullptr);

        auto* contStmt = dynamic_cast<const ContinueStmt*>(outerBlock->statements()[1].get());
        REQUIRE(contStmt != nullptr);
    }
}

// -----------------------------------------------------------------------------
// 5. Source Location Tracking
// -----------------------------------------------------------------------------
TEST_CASE("JumpStmt: source location tracking", "[parser][break][continue][location]") {
    SECTION("break on line 1, column 1") {
        Parser parser = makeParser("break;");
        auto stmt = parser.parseBreakStatement();
        REQUIRE(stmt != nullptr);
        CHECK(stmt->location().line == 1);
        CHECK(stmt->location().column == 1);
    }

    SECTION("break with leading whitespace and newline") {
        Parser parser = makeParser("\n   break;");
        auto stmt = parser.parseBreakStatement();
        REQUIRE(stmt != nullptr);
        CHECK(stmt->location().line == 2);
        CHECK(stmt->location().column == 4);
    }

    SECTION("continue on line 1, column 1") {
        Parser parser = makeParser("continue;");
        auto stmt = parser.parseContinueStatement();
        REQUIRE(stmt != nullptr);
        CHECK(stmt->location().line == 1);
        CHECK(stmt->location().column == 1);
    }

    SECTION("continue with leading whitespace and newline") {
        Parser parser = makeParser("\n     continue;");
        auto stmt = parser.parseContinueStatement();
        REQUIRE(stmt != nullptr);
        CHECK(stmt->location().line == 2);
        CHECK(stmt->location().column == 6);
    }
}

// -----------------------------------------------------------------------------
// 6. ASTPrinter Output Formatting
// -----------------------------------------------------------------------------
TEST_CASE("JumpStmt: ASTPrinter output", "[parser][break][continue][printer]") {
    SECTION("bare break statement") {
        Parser parser = makeParser("break;");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);

        std::string expected = "BreakStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }

    SECTION("bare continue statement") {
        Parser parser = makeParser("continue;");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);

        std::string expected = "ContinueStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }

    SECTION("while with break in body") {
        Parser parser = makeParser("while (1) break;");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);

        std::string expected =
            "WhileStmt\n"
            "  Condition:\n"
            "    LiteralExpression (Integer): 1\n"
            "  Body:\n"
            "    BreakStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }

    SECTION("for with continue in block") {
        Parser parser = makeParser("for (;;) { continue; }");
        auto stmt = parser.parseStatement();
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
            "    CompoundStmt\n"
            "      ContinueStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }

    SECTION("if-else with break and continue") {
        Parser parser = makeParser("if (x) break; else continue;");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);

        std::string expected =
            "IfStmt\n"
            "  Condition:\n"
            "    IdentifierExpression: x\n"
            "  Then:\n"
            "    BreakStmt\n"
            "  Else:\n"
            "    ContinueStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }
}

// -----------------------------------------------------------------------------
// 7. Error Handling and Syntax Errors
// -----------------------------------------------------------------------------
TEST_CASE("JumpStmt: error handling and syntax errors", "[parser][break][continue][error]") {
    SECTION("missing semicolon after break at EOF: 'break'") {
        Parser parser = makeParser("break");
        auto stmt = parser.parseBreakStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';' after 'break'") != std::string::npos);
    }

    SECTION("missing semicolon after break with identifier following: 'break x;'") {
        Parser parser = makeParser("break x;");
        auto stmt = parser.parseBreakStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';' after 'break'") != std::string::npos);
    }

    SECTION("missing semicolon after continue at EOF: 'continue'") {
        Parser parser = makeParser("continue");
        auto stmt = parser.parseContinueStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';' after 'continue'") != std::string::npos);
    }

    SECTION("missing semicolon after continue with identifier following: 'continue x;'") {
        Parser parser = makeParser("continue x;");
        auto stmt = parser.parseContinueStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';' after 'continue'") != std::string::npos);
    }

    SECTION("parseBreakStatement when token is not break") {
        Parser parser = makeParser("continue;");
        auto stmt = parser.parseBreakStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected 'break'") != std::string::npos);
    }

    SECTION("parseContinueStatement when token is not continue") {
        Parser parser = makeParser("break;");
        auto stmt = parser.parseContinueStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected 'continue'") != std::string::npos);
    }

    SECTION("case sensitivity: 'Break;' is not a keyword") {
        Parser parser = makeParser("Break;");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement, found 'Break'") != std::string::npos);
    }

    SECTION("case sensitivity: 'CONTINUE;' is not a keyword") {
        Parser parser = makeParser("CONTINUE;");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement, found 'CONTINUE'") != std::string::npos);
    }
}

// -----------------------------------------------------------------------------
// 8. Non-Regression: existing control-flow statements remain unaffected
// -----------------------------------------------------------------------------
TEST_CASE("JumpStmt: non-regression for existing control flow", "[parser][regression]") {
    SECTION("if/else without jumps still works") {
        Parser parser = makeParser("if (a) {} else {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK(dynamic_cast<const IfStmt*>(stmt.get()) != nullptr);
        CHECK_FALSE(parser.hasError());
    }

    SECTION("while loop without jumps still works") {
        Parser parser = makeParser("while (x > 0) {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK(dynamic_cast<const WhileStmt*>(stmt.get()) != nullptr);
        CHECK_FALSE(parser.hasError());
    }

    SECTION("do-while loop without jumps still works") {
        Parser parser = makeParser("do {} while (x < 10);");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK(dynamic_cast<const DoWhileStmt*>(stmt.get()) != nullptr);
        CHECK_FALSE(parser.hasError());
    }

    SECTION("for loop without jumps still works") {
        Parser parser = makeParser("for (i = 0; i < 10; i++) {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK(dynamic_cast<const ForStmt*>(stmt.get()) != nullptr);
        CHECK_FALSE(parser.hasError());
    }
}

