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
// DAY 35 INTEGRATION TEST SUITE — Control Flow & Statements Integration
// =============================================================================

// =============================================================================
// Test Group A — Individual Construct Regression
// =============================================================================
TEST_CASE("ControlFlowIntegration: Group A - Individual construct regression", "[control_flow][integration][regression]") {
    SECTION("CompoundStmt: empty block '{}'") {
        Parser parser = makeParser("{}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        auto* comp = dynamic_cast<const CompoundStmt*>(stmt.get());
        REQUIRE(comp != nullptr);
        CHECK(comp->empty());
    }

    SECTION("IfStmt: basic if '(a) {}'") {
        Parser parser = makeParser("if (a) {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        auto* ifStmt = dynamic_cast<const IfStmt*>(stmt.get());
        REQUIRE(ifStmt != nullptr);
        CHECK_FALSE(ifStmt->hasElse());
    }

    SECTION("IfStmt: if/else '(a) {} else {}'") {
        Parser parser = makeParser("if (a) {} else {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        auto* ifStmt = dynamic_cast<const IfStmt*>(stmt.get());
        REQUIRE(ifStmt != nullptr);
        CHECK(ifStmt->hasElse());
    }

    SECTION("WhileStmt: while '(x) {}'") {
        Parser parser = makeParser("while (x) {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt.get());
        REQUIRE(whileStmt != nullptr);
    }

    SECTION("DoWhileStmt: do-while 'do {} while (x);'") {
        Parser parser = makeParser("do {} while (x);");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        auto* doWhileStmt = dynamic_cast<const DoWhileStmt*>(stmt.get());
        REQUIRE(doWhileStmt != nullptr);
    }

    SECTION("ForStmt: for '(;;) {}'") {
        Parser parser = makeParser("for (;;) {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        auto* forStmt = dynamic_cast<const ForStmt*>(stmt.get());
        REQUIRE(forStmt != nullptr);
        CHECK_FALSE(forStmt->hasInitializer());
        CHECK_FALSE(forStmt->hasCondition());
        CHECK_FALSE(forStmt->hasPost());
    }

    SECTION("ForStmt: full clauses 'for (i = 0; i < 10; i++) {}'") {
        Parser parser = makeParser("for (i = 0; i < 10; i++) {}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        auto* forStmt = dynamic_cast<const ForStmt*>(stmt.get());
        REQUIRE(forStmt != nullptr);
        CHECK(forStmt->hasInitializer());
        CHECK(forStmt->hasCondition());
        CHECK(forStmt->hasPost());
    }

    SECTION("BreakStmt: 'break;'") {
        Parser parser = makeParser("break;");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        CHECK(dynamic_cast<const BreakStmt*>(stmt.get()) != nullptr);
    }

    SECTION("ContinueStmt: 'continue;'") {
        Parser parser = makeParser("continue;");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        CHECK(dynamic_cast<const ContinueStmt*>(stmt.get()) != nullptr);
    }
}

// =============================================================================
// Test Group B — Nested Control Flow
// =============================================================================
TEST_CASE("ControlFlowIntegration: Group B - Nested control flow combinations", "[control_flow][integration][nesting]") {
    SECTION("while with if and break") {
        const std::string src =
            "while (x < 10) {\n"
            "    if (x == 5) {\n"
            "        break;\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt.get());
        REQUIRE(whileStmt != nullptr);

        auto* bodyBlock = dynamic_cast<const CompoundStmt*>(whileStmt->body());
        REQUIRE(bodyBlock != nullptr);
        REQUIRE(bodyBlock->statementCount() == 1);

        auto* ifStmt = dynamic_cast<const IfStmt*>(bodyBlock->statements()[0].get());
        REQUIRE(ifStmt != nullptr);

        auto* ifBody = dynamic_cast<const CompoundStmt*>(ifStmt->thenBranch());
        REQUIRE(ifBody != nullptr);
        REQUIRE(ifBody->statementCount() == 1);
        CHECK(dynamic_cast<const BreakStmt*>(ifBody->statements()[0].get()) != nullptr);
    }

    SECTION("for with if/else containing continue and while loop with break") {
        const std::string src =
            "for (i = 0; i < 10; i++) {\n"
            "    if (i == 3) {\n"
            "        continue;\n"
            "    } else {\n"
            "        while (x > 0) {\n"
            "            break;\n"
            "        }\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* forStmt = dynamic_cast<const ForStmt*>(stmt.get());
        REQUIRE(forStmt != nullptr);

        auto* forBody = dynamic_cast<const CompoundStmt*>(forStmt->body());
        REQUIRE(forBody != nullptr);
        REQUIRE(forBody->statementCount() == 1);

        auto* ifStmt = dynamic_cast<const IfStmt*>(forBody->statements()[0].get());
        REQUIRE(ifStmt != nullptr);
        REQUIRE(ifStmt->hasElse());

        auto* thenBlock = dynamic_cast<const CompoundStmt*>(ifStmt->thenBranch());
        REQUIRE(thenBlock != nullptr);
        REQUIRE(thenBlock->statementCount() == 1);
        CHECK(dynamic_cast<const ContinueStmt*>(thenBlock->statements()[0].get()) != nullptr);

        auto* elseBlock = dynamic_cast<const CompoundStmt*>(ifStmt->elseBranch());
        REQUIRE(elseBlock != nullptr);
        REQUIRE(elseBlock->statementCount() == 1);

        auto* innerWhile = dynamic_cast<const WhileStmt*>(elseBlock->statements()[0].get());
        REQUIRE(innerWhile != nullptr);
        auto* whileBody = dynamic_cast<const CompoundStmt*>(innerWhile->body());
        REQUIRE(whileBody != nullptr);
        REQUIRE(whileBody->statementCount() == 1);
        CHECK(dynamic_cast<const BreakStmt*>(whileBody->statements()[0].get()) != nullptr);
    }

    SECTION("do-while with if and while loop") {
        const std::string src =
            "do {\n"
            "    if (done) {\n"
            "        break;\n"
            "    }\n"
            "    while (x > 0) {\n"
            "        continue;\n"
            "    }\n"
            "} while (running);";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* doWhileStmt = dynamic_cast<const DoWhileStmt*>(stmt.get());
        REQUIRE(doWhileStmt != nullptr);

        auto* block = dynamic_cast<const CompoundStmt*>(doWhileStmt->body());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 2);

        auto* ifStmt = dynamic_cast<const IfStmt*>(block->statements()[0].get());
        REQUIRE(ifStmt != nullptr);

        auto* whileStmt = dynamic_cast<const WhileStmt*>(block->statements()[1].get());
        REQUIRE(whileStmt != nullptr);
    }

    SECTION("sibling loops and jumps in compound statement") {
        const std::string src =
            "{\n"
            "    while (a) { break; }\n"
            "    for (;;) { continue; }\n"
            "    do { break; } while (b);\n"
            "}";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* block = dynamic_cast<const CompoundStmt*>(stmt.get());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 3);

        CHECK(dynamic_cast<const WhileStmt*>(block->statements()[0].get()) != nullptr);
        CHECK(dynamic_cast<const ForStmt*>(block->statements()[1].get()) != nullptr);
        CHECK(dynamic_cast<const DoWhileStmt*>(block->statements()[2].get()) != nullptr);
    }
}

// =============================================================================
// Test Group C — Deeply Nested Structures
// =============================================================================
TEST_CASE("ControlFlowIntegration: Group C - Deeply nested structures", "[control_flow][integration][deep]") {
    SECTION("nested if/else inside loops") {
        const std::string src =
            "while (cond) {\n"
            "    if (a) {\n"
            "        if (b) {\n"
            "            break;\n"
            "        } else {\n"
            "            continue;\n"
            "        }\n"
            "    } else {\n"
            "        if (c) {\n"
            "            continue;\n"
            "        }\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt.get());
        REQUIRE(whileStmt != nullptr);
        auto* outerIf = dynamic_cast<const IfStmt*>(dynamic_cast<const CompoundStmt*>(whileStmt->body())->statements()[0].get());
        REQUIRE(outerIf != nullptr);
        REQUIRE(outerIf->hasElse());
    }

    SECTION("loops inside deeply nested compound statements") {
        const std::string src =
            "{\n"
            "    {\n"
            "        while (a) {\n"
            "            do {\n"
            "                for (;;) {\n"
            "                    break;\n"
            "                }\n"
            "            } while (b);\n"
            "        }\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* b1 = dynamic_cast<const CompoundStmt*>(stmt.get());
        REQUIRE(b1 != nullptr);
        auto* b2 = dynamic_cast<const CompoundStmt*>(b1->statements()[0].get());
        REQUIRE(b2 != nullptr);
        auto* w = dynamic_cast<const WhileStmt*>(b2->statements()[0].get());
        REQUIRE(w != nullptr);
        auto* dw = dynamic_cast<const DoWhileStmt*>(dynamic_cast<const CompoundStmt*>(w->body())->statements()[0].get());
        REQUIRE(dw != nullptr);
        auto* f = dynamic_cast<const ForStmt*>(dynamic_cast<const CompoundStmt*>(dw->body())->statements()[0].get());
        REQUIRE(f != nullptr);
    }

    SECTION("nested for, while, and do-while combinations") {
        const std::string src =
            "for (i = 0; i < n; i++) {\n"
            "    while (j < m) {\n"
            "        do {\n"
            "            if (flag) break; else continue;\n"
            "        } while (k);\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
    }

    SECTION("multiple break and continue in different branches") {
        const std::string src =
            "for (;;) {\n"
            "    if (x) {\n"
            "        break;\n"
            "    }\n"
            "    if (y) {\n"
            "        continue;\n"
            "    }\n"
            "    if (z) {\n"
            "        break;\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* forStmt = dynamic_cast<const ForStmt*>(stmt.get());
        REQUIRE(forStmt != nullptr);
        auto* block = dynamic_cast<const CompoundStmt*>(forStmt->body());
        REQUIRE(block != nullptr);
        REQUIRE(block->statementCount() == 3);
    }
}

// =============================================================================
// Test Group D — AST Structure Verification
// =============================================================================
TEST_CASE("ControlFlowIntegration: Group D - Detailed AST structure verification", "[control_flow][integration][ast_verify]") {
    SECTION("Section 4 conceptual hierarchy verification") {
        // Concept:
        // for (i = 0; i < 10; i++) {
        //     if (i == 5) {
        //         break;
        //     }
        // }
        const std::string src =
            "for (i = 0; i < 10; i++) {\n"
            "    if (i == 5) {\n"
            "        break;\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        // Level 1: ForStmt
        auto* forStmt = dynamic_cast<const ForStmt*>(stmt.get());
        REQUIRE(forStmt != nullptr);

        // ForStmt -> Initializer: AssignExpr (=, i, 0)
        REQUIRE(forStmt->hasInitializer());
        auto* initAssign = dynamic_cast<const AssignExpr*>(forStmt->initializer());
        REQUIRE(initAssign != nullptr);
        CHECK(initAssign->op() == "=");
        auto* initLeft = dynamic_cast<const IdentifierExpression*>(initAssign->left());
        REQUIRE(initLeft != nullptr);
        CHECK(initLeft->name() == "i");
        auto* initRight = dynamic_cast<const LiteralExpression*>(initAssign->right());
        REQUIRE(initRight != nullptr);
        CHECK(initRight->value() == "0");

        // ForStmt -> Condition: BinaryExpr (<, i, 10)
        REQUIRE(forStmt->hasCondition());
        auto* condBin = dynamic_cast<const BinaryExpr*>(forStmt->condition());
        REQUIRE(condBin != nullptr);
        CHECK(condBin->op() == "<");

        // ForStmt -> Post: UnaryExpr (++, i)
        REQUIRE(forStmt->hasPost());
        auto* postUn = dynamic_cast<const UnaryExpr*>(forStmt->post());
        REQUIRE(postUn != nullptr);
        CHECK(postUn->op() == "++");

        // ForStmt -> Body: CompoundStmt
        REQUIRE(forStmt->body() != nullptr);
        auto* forBody = dynamic_cast<const CompoundStmt*>(forStmt->body());
        REQUIRE(forBody != nullptr);
        REQUIRE(forBody->statementCount() == 1);

        // CompoundStmt -> IfStmt
        auto* ifStmt = dynamic_cast<const IfStmt*>(forBody->statements()[0].get());
        REQUIRE(ifStmt != nullptr);
        CHECK_FALSE(ifStmt->hasElse());

        // IfStmt -> Condition: BinaryExpr (==, i, 5)
        REQUIRE(ifStmt->condition() != nullptr);
        auto* ifCond = dynamic_cast<const BinaryExpr*>(ifStmt->condition());
        REQUIRE(ifCond != nullptr);
        CHECK(ifCond->op() == "==");

        // IfStmt -> Then: CompoundStmt
        REQUIRE(ifStmt->thenBranch() != nullptr);
        auto* ifThen = dynamic_cast<const CompoundStmt*>(ifStmt->thenBranch());
        REQUIRE(ifThen != nullptr);
        REQUIRE(ifThen->statementCount() == 1);

        // CompoundStmt -> BreakStmt
        auto* breakStmt = dynamic_cast<const BreakStmt*>(ifThen->statements()[0].get());
        REQUIRE(breakStmt != nullptr);
    }

    SECTION("Complex multi-level control flow hierarchy verification") {
        const std::string src =
            "for (i = 0; i < 10; i++) {\n"
            "    if (i > 5) {\n"
            "        while (x > 0) {\n"
            "            if (x == 2) {\n"
            "                break;\n"
            "            } else {\n"
            "                continue;\n"
            "            }\n"
            "        }\n"
            "    } else {\n"
            "        do {\n"
            "            continue;\n"
            "        } while (running);\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        // 1. Root: ForStmt
        auto* forStmt = dynamic_cast<const ForStmt*>(stmt.get());
        REQUIRE(forStmt != nullptr);
        CHECK(forStmt->hasInitializer());
        CHECK(forStmt->hasCondition());
        CHECK(forStmt->hasPost());

        // 2. For Body: CompoundStmt
        auto* forBody = dynamic_cast<const CompoundStmt*>(forStmt->body());
        REQUIRE(forBody != nullptr);
        REQUIRE(forBody->statementCount() == 1);

        // 3. IfStmt with else
        auto* outerIf = dynamic_cast<const IfStmt*>(forBody->statements()[0].get());
        REQUIRE(outerIf != nullptr);
        REQUIRE(outerIf->hasElse());

        // 4. Then-branch: CompoundStmt containing WhileStmt
        auto* thenBlock = dynamic_cast<const CompoundStmt*>(outerIf->thenBranch());
        REQUIRE(thenBlock != nullptr);
        REQUIRE(thenBlock->statementCount() == 1);

        auto* whileStmt = dynamic_cast<const WhileStmt*>(thenBlock->statements()[0].get());
        REQUIRE(whileStmt != nullptr);
        auto* whileCond = dynamic_cast<const BinaryExpr*>(whileStmt->condition());
        REQUIRE(whileCond != nullptr);
        CHECK(whileCond->op() == ">");

        // 5. While Body: CompoundStmt containing inner IfStmt
        auto* whileBody = dynamic_cast<const CompoundStmt*>(whileStmt->body());
        REQUIRE(whileBody != nullptr);
        REQUIRE(whileBody->statementCount() == 1);

        auto* innerIf = dynamic_cast<const IfStmt*>(whileBody->statements()[0].get());
        REQUIRE(innerIf != nullptr);
        REQUIRE(innerIf->hasElse());

        // 6. Inner If branches: BreakStmt and ContinueStmt
        auto* innerThen = dynamic_cast<const CompoundStmt*>(innerIf->thenBranch());
        REQUIRE(innerThen != nullptr);
        REQUIRE(innerThen->statementCount() == 1);
        CHECK(dynamic_cast<const BreakStmt*>(innerThen->statements()[0].get()) != nullptr);

        auto* innerElse = dynamic_cast<const CompoundStmt*>(innerIf->elseBranch());
        REQUIRE(innerElse != nullptr);
        REQUIRE(innerElse->statementCount() == 1);
        CHECK(dynamic_cast<const ContinueStmt*>(innerElse->statements()[0].get()) != nullptr);

        // 7. Outer If Else-branch: CompoundStmt containing DoWhileStmt
        auto* elseBlock = dynamic_cast<const CompoundStmt*>(outerIf->elseBranch());
        REQUIRE(elseBlock != nullptr);
        REQUIRE(elseBlock->statementCount() == 1);

        auto* doWhileStmt = dynamic_cast<const DoWhileStmt*>(elseBlock->statements()[0].get());
        REQUIRE(doWhileStmt != nullptr);

        auto* doBody = dynamic_cast<const CompoundStmt*>(doWhileStmt->body());
        REQUIRE(doBody != nullptr);
        REQUIRE(doBody->statementCount() == 1);
        CHECK(dynamic_cast<const ContinueStmt*>(doBody->statements()[0].get()) != nullptr);

        auto* doCond = dynamic_cast<const IdentifierExpression*>(doWhileStmt->condition());
        REQUIRE(doCond != nullptr);
        CHECK(doCond->name() == "running");
    }
}

// =============================================================================
// Test Group E — AST Printer
// =============================================================================
TEST_CASE("ControlFlowIntegration: Group E - AST Printer output verification", "[control_flow][integration][printer]") {
    SECTION("ASTPrinter for Section 4 hierarchy") {
        const std::string src =
            "for (i = 0; i < 10; i++) {\n"
            "    if (i == 5) {\n"
            "        break;\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
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
            "    CompoundStmt\n"
            "      IfStmt\n"
            "        Condition:\n"
            "          BinaryExpr: ==\n"
            "            Left:\n"
            "              IdentifierExpression: i\n"
            "            Right:\n"
            "              LiteralExpression (Integer): 5\n"
            "        Then:\n"
            "          CompoundStmt\n"
            "            BreakStmt\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }

    SECTION("ASTPrinter for complex multi-level control flow hierarchy") {
        const std::string src =
            "for (i = 0; i < 10; i++) {\n"
            "    if (i > 5) {\n"
            "        while (x > 0) {\n"
            "            if (x == 2) {\n"
            "                break;\n"
            "            } else {\n"
            "                continue;\n"
            "            }\n"
            "        }\n"
            "    } else {\n"
            "        do {\n"
            "            continue;\n"
            "        } while (running);\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
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
            "    CompoundStmt\n"
            "      IfStmt\n"
            "        Condition:\n"
            "          BinaryExpr: >\n"
            "            Left:\n"
            "              IdentifierExpression: i\n"
            "            Right:\n"
            "              LiteralExpression (Integer): 5\n"
            "        Then:\n"
            "          CompoundStmt\n"
            "            WhileStmt\n"
            "              Condition:\n"
            "                BinaryExpr: >\n"
            "                  Left:\n"
            "                    IdentifierExpression: x\n"
            "                  Right:\n"
            "                    LiteralExpression (Integer): 0\n"
            "              Body:\n"
            "                CompoundStmt\n"
            "                  IfStmt\n"
            "                    Condition:\n"
            "                      BinaryExpr: ==\n"
            "                        Left:\n"
            "                          IdentifierExpression: x\n"
            "                        Right:\n"
            "                          LiteralExpression (Integer): 2\n"
            "                    Then:\n"
            "                      CompoundStmt\n"
            "                        BreakStmt\n"
            "                    Else:\n"
            "                      CompoundStmt\n"
            "                        ContinueStmt\n"
            "        Else:\n"
            "          CompoundStmt\n"
            "            DoWhileStmt\n"
            "              Body:\n"
            "                CompoundStmt\n"
            "                  ContinueStmt\n"
            "              Condition:\n"
            "                IdentifierExpression: running\n";
        CHECK(dumpAST(stmt.get()) == expected);
    }
}

// =============================================================================
// Test Group F — Negative and Regression Tests
// =============================================================================
TEST_CASE("ControlFlowIntegration: Group F - Negative and error diagnostics", "[control_flow][integration][error]") {
    SECTION("missing closing brace in nested block") {
        Parser parser = makeParser("while (1) { if (x) { break; }");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '}' to close compound statement") != std::string::npos);
    }

    SECTION("missing opening parenthesis in if condition") {
        Parser parser = makeParser("if x > 0) {}");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '(' after 'if'") != std::string::npos);
    }

    SECTION("missing opening parenthesis in while condition") {
        Parser parser = makeParser("while x < 10) {}");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '(' after 'while'") != std::string::npos);
    }

    SECTION("missing opening parenthesis in do-while condition") {
        Parser parser = makeParser("do {} while x < 10);");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '(' after 'while' in do-while loop") != std::string::npos);
    }

    SECTION("missing opening parenthesis in for loop") {
        Parser parser = makeParser("for i = 0; i < 10; i++) {}");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '(' after 'for'") != std::string::npos);
    }

    SECTION("missing while condition expression: 'while () {}'") {
        Parser parser = makeParser("while () {}");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression in 'while' condition") != std::string::npos);
    }

    SECTION("missing do-while condition expression: 'do {} while ();'") {
        Parser parser = makeParser("do {} while ();");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression in 'do-while' condition") != std::string::npos);
    }

    SECTION("missing first semicolon separator in for loop: 'for (i = 0 i < 10; i++) {}'") {
        Parser parser = makeParser("for (i = 0 i < 10; i++) {}");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';' after 'for' initializer") != std::string::npos);
    }

    SECTION("missing second semicolon separator in for loop: 'for (i = 0; i < 10) {}'") {
        Parser parser = makeParser("for (i = 0; i < 10) {}");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';' after 'for' condition") != std::string::npos);
    }

    SECTION("missing semicolon after break: 'while (1) { break }'") {
        Parser parser = makeParser("while (1) { break }");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';' after 'break'") != std::string::npos);
    }

    SECTION("missing semicolon after continue: 'while (1) { continue }'") {
        Parser parser = makeParser("while (1) { continue }");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';' after 'continue'") != std::string::npos);
    }

    SECTION("unexpected token in statement position: 'while (1) { return 0; }'") {
        Parser parser = makeParser("while (1) { return 0; }");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement, found 'return'") != std::string::npos);
    }

    SECTION("unexpected declaration token in statement position: 'while (1) { int x; }'") {
        Parser parser = makeParser("while (1) { int x; }");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement, found 'int'") != std::string::npos);
    }

    SECTION("graceful recovery: malformed inputs do not loop infinitely or crash") {
        std::vector<std::string> malformedInputs = {
            "{",
            "if",
            "while",
            "do",
            "for",
            "break",
            "continue",
            "if ()",
            "while ()",
            "for (",
            "for (;",
            "for (;;",
            "do while",
            "do { break;",
            "{ { { { {"
        };

        for (const auto& input : malformedInputs) {
            Parser parser = makeParser(input);
            auto stmt = parser.parseStatement();
            CHECK(stmt == nullptr);
            CHECK(parser.hasError());
        }
    }
}

