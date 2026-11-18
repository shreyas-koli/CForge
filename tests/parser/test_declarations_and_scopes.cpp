#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/var_decl.h"
#include "ast/compound_stmt.h"
#include "ast/if_stmt.h"
#include "ast/while_stmt.h"
#include "ast/do_while_stmt.h"
#include "ast/for_stmt.h"
#include "ast/break_stmt.h"
#include "ast/continue_stmt.h"
#include "ast/function.h"
#include "ast/literal_expression.h"
#include "ast/printer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "types/type.h"

#include <memory>
#include <string>

// Helper to construct a Parser from a source string
static Parser makeParser(const std::string& src) {
    Lexer lexer(src);
    return Parser(lexer.tokenize());
}

// Helper to dump AST node to string via ASTPrinter
static std::string dumpAST(const ASTNode* node) {
    ASTPrinter printer;
    return printer.print(node);
}

// =============================================================================
// DAY 43 TESTS — Declarations & Scopes (Nested Blocks)
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Declarations inside basic compound blocks
// -----------------------------------------------------------------------------
TEST_CASE("DeclarationsAndScopes: Basic declarations inside compound blocks", "[parser][decl][scope][compound]") {
    SECTION("Single primitive declaration without initializer: '{ int x; }'") {
        Parser parser = makeParser("{ int x; }");
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        REQUIRE(block->statementCount() == 1);
        auto* decl = dynamic_cast<const VarDecl*>(block->statements()[0].get());
        REQUIRE(decl != nullptr);
        CHECK(decl->name() == "x");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Int);
        CHECK_FALSE(decl->hasInitializer());
        CHECK(decl->storageClass() == StorageClass::None);
        CHECK_FALSE(decl->isConst());
        CHECK_FALSE(decl->isVolatile());
    }

    SECTION("Declaration with initializer: '{ int y = 42; }'") {
        Parser parser = makeParser("{ int y = 42; }");
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(block->statementCount() == 1);
        auto* decl = dynamic_cast<const VarDecl*>(block->statements()[0].get());
        REQUIRE(decl != nullptr);
        CHECK(decl->name() == "y");
        CHECK(decl->hasInitializer());
        auto* lit = dynamic_cast<const LiteralExpression*>(decl->initializer());
        REQUIRE(lit != nullptr);
        CHECK(lit->value() == "42");
    }

    SECTION("Various primitive types inside a block") {
        const std::string src =
            "{\n"
            "    char ch = 'A';\n"
            "    float f = 3.14;\n"
            "    double d = 2.71828;\n"
            "    void v;\n"
            "}";
        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(block->statementCount() == 4);

        auto* d0 = dynamic_cast<const VarDecl*>(block->statements()[0].get());
        REQUIRE(d0 != nullptr);
        CHECK(d0->name() == "ch");
        CHECK(d0->typeKind() == TypeKind::Char);
        CHECK(d0->hasInitializer());

        auto* d1 = dynamic_cast<const VarDecl*>(block->statements()[1].get());
        REQUIRE(d1 != nullptr);
        CHECK(d1->name() == "f");
        CHECK(d1->typeKind() == TypeKind::Float);
        CHECK(d1->hasInitializer());

        auto* d2 = dynamic_cast<const VarDecl*>(block->statements()[2].get());
        REQUIRE(d2 != nullptr);
        CHECK(d2->name() == "d");
        CHECK(d2->typeKind() == TypeKind::Double);
        CHECK(d2->hasInitializer());

        auto* d3 = dynamic_cast<const VarDecl*>(block->statements()[3].get());
        REQUIRE(d3 != nullptr);
        CHECK(d3->name() == "v");
        CHECK(d3->typeKind() == TypeKind::Void);
        CHECK_FALSE(d3->hasInitializer());
    }

    SECTION("Type qualifiers: const and volatile declarations inside block") {
        const std::string src =
            "{\n"
            "    const int c = 10;\n"
            "    volatile float vf = 1.0;\n"
            "    const volatile double cvd = 0.5;\n"
            "}";
        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(block->statementCount() == 3);

        auto* d0 = dynamic_cast<const VarDecl*>(block->statements()[0].get());
        REQUIRE(d0 != nullptr);
        CHECK(d0->name() == "c");
        CHECK(d0->isConst());
        CHECK_FALSE(d0->isVolatile());

        auto* d1 = dynamic_cast<const VarDecl*>(block->statements()[1].get());
        REQUIRE(d1 != nullptr);
        CHECK(d1->name() == "vf");
        CHECK(d1->isVolatile());
        CHECK_FALSE(d1->isConst());

        auto* d2 = dynamic_cast<const VarDecl*>(block->statements()[2].get());
        REQUIRE(d2 != nullptr);
        CHECK(d2->name() == "cvd");
        CHECK(d2->isConst());
        CHECK(d2->isVolatile());
    }

    SECTION("Storage classes: static, extern, register inside block") {
        const std::string src =
            "{\n"
            "    static int counter = 0;\n"
            "    extern int global_flag;\n"
            "    register int fast_idx = 100;\n"
            "}";
        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(block->statementCount() == 3);

        auto* d0 = dynamic_cast<const VarDecl*>(block->statements()[0].get());
        REQUIRE(d0 != nullptr);
        CHECK(d0->name() == "counter");
        CHECK(d0->isStatic());
        CHECK(d0->storageClass() == StorageClass::Static);

        auto* d1 = dynamic_cast<const VarDecl*>(block->statements()[1].get());
        REQUIRE(d1 != nullptr);
        CHECK(d1->name() == "global_flag");
        CHECK(d1->isExtern());
        CHECK(d1->storageClass() == StorageClass::Extern);

        auto* d2 = dynamic_cast<const VarDecl*>(block->statements()[2].get());
        REQUIRE(d2 != nullptr);
        CHECK(d2->name() == "fast_idx");
        CHECK(d2->isRegister());
        CHECK(d2->storageClass() == StorageClass::Register);
    }
}

// -----------------------------------------------------------------------------
// 2. Declarations inside nested compound blocks
// -----------------------------------------------------------------------------
TEST_CASE("DeclarationsAndScopes: Nested compound blocks", "[parser][decl][scope][nesting]") {
    SECTION("Two-level nesting with declarations at each level") {
        const std::string src =
            "{\n"
            "    int outer = 1;\n"
            "    {\n"
            "        int inner = 2;\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto outerBlock = parser.parseCompoundStatement();
        REQUIRE(outerBlock != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        REQUIRE(outerBlock->statementCount() == 2);

        // First item: VarDecl 'outer'
        auto* outerDecl = dynamic_cast<const VarDecl*>(outerBlock->statements()[0].get());
        REQUIRE(outerDecl != nullptr);
        CHECK(outerDecl->name() == "outer");

        // Second item: inner CompoundStmt
        auto* innerBlock = dynamic_cast<const CompoundStmt*>(outerBlock->statements()[1].get());
        REQUIRE(innerBlock != nullptr);
        REQUIRE(innerBlock->statementCount() == 1);

        auto* innerDecl = dynamic_cast<const VarDecl*>(innerBlock->statements()[0].get());
        REQUIRE(innerDecl != nullptr);
        CHECK(innerDecl->name() == "inner");
    }

    SECTION("Three-level nesting: '{ int a = 1; { int b = 2; { int c = 3; } } }'") {
        const std::string src =
            "{\n"
            "    int a = 1;\n"
            "    {\n"
            "        int b = 2;\n"
            "        {\n"
            "            int c = 3;\n"
            "        }\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto l1 = parser.parseCompoundStatement();
        REQUIRE(l1 != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(l1->statementCount() == 2);
        CHECK(dynamic_cast<const VarDecl*>(l1->statements()[0].get())->name() == "a");

        auto* l2 = dynamic_cast<const CompoundStmt*>(l1->statements()[1].get());
        REQUIRE(l2 != nullptr);
        REQUIRE(l2->statementCount() == 2);
        CHECK(dynamic_cast<const VarDecl*>(l2->statements()[0].get())->name() == "b");

        auto* l3 = dynamic_cast<const CompoundStmt*>(l2->statements()[1].get());
        REQUIRE(l3 != nullptr);
        REQUIRE(l3->statementCount() == 1);
        CHECK(dynamic_cast<const VarDecl*>(l3->statements()[0].get())->name() == "c");
    }

    SECTION("Sibling nested blocks inside outer block") {
        const std::string src =
            "{\n"
            "    int header = 0;\n"
            "    {\n"
            "        int left = 10;\n"
            "    }\n"
            "    {\n"
            "        int right = 20;\n"
            "    }\n"
            "    int footer = 99;\n"
            "}";
        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(block->statementCount() == 4);

        // 0: VarDecl header
        auto* h = dynamic_cast<const VarDecl*>(block->statements()[0].get());
        REQUIRE(h != nullptr);
        CHECK(h->name() == "header");

        // 1: CompoundStmt left
        auto* b1 = dynamic_cast<const CompoundStmt*>(block->statements()[1].get());
        REQUIRE(b1 != nullptr);
        REQUIRE(b1->statementCount() == 1);
        CHECK(dynamic_cast<const VarDecl*>(b1->statements()[0].get())->name() == "left");

        // 2: CompoundStmt right
        auto* b2 = dynamic_cast<const CompoundStmt*>(block->statements()[2].get());
        REQUIRE(b2 != nullptr);
        REQUIRE(b2->statementCount() == 1);
        CHECK(dynamic_cast<const VarDecl*>(b2->statements()[0].get())->name() == "right");

        // 3: VarDecl footer
        auto* f = dynamic_cast<const VarDecl*>(block->statements()[3].get());
        REQUIRE(f != nullptr);
        CHECK(f->name() == "footer");
    }
}

// -----------------------------------------------------------------------------
// 3. Declarations inside if / else blocks
// -----------------------------------------------------------------------------
TEST_CASE("DeclarationsAndScopes: Declarations inside if and else blocks", "[parser][decl][scope][if]") {
    SECTION("Single if statement with declaration in body: 'if (x) { int a = 10; }'") {
        Parser parser = makeParser("if (x) { int a = 10; }");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* ifStmt = dynamic_cast<const IfStmt*>(stmt.get());
        REQUIRE(ifStmt != nullptr);
        CHECK_FALSE(ifStmt->hasElse());

        auto* thenBlock = dynamic_cast<const CompoundStmt*>(ifStmt->thenBranch());
        REQUIRE(thenBlock != nullptr);
        REQUIRE(thenBlock->statementCount() == 1);

        auto* decl = dynamic_cast<const VarDecl*>(thenBlock->statements()[0].get());
        REQUIRE(decl != nullptr);
        CHECK(decl->name() == "a");
        CHECK(decl->hasInitializer());
    }

    SECTION("if/else with declarations in both branches") {
        const std::string src =
            "if (flag) {\n"
            "    int branch1 = 1;\n"
            "    const char tag1 = 'T';\n"
            "} else {\n"
            "    int branch2 = 2;\n"
            "    const char tag2 = 'F';\n"
            "}";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* ifStmt = dynamic_cast<const IfStmt*>(stmt.get());
        REQUIRE(ifStmt != nullptr);
        CHECK(ifStmt->hasElse());

        auto* thenBlock = dynamic_cast<const CompoundStmt*>(ifStmt->thenBranch());
        REQUIRE(thenBlock != nullptr);
        REQUIRE(thenBlock->statementCount() == 2);
        CHECK(dynamic_cast<const VarDecl*>(thenBlock->statements()[0].get())->name() == "branch1");
        CHECK(dynamic_cast<const VarDecl*>(thenBlock->statements()[1].get())->name() == "tag1");

        auto* elseBlock = dynamic_cast<const CompoundStmt*>(ifStmt->elseBranch());
        REQUIRE(elseBlock != nullptr);
        REQUIRE(elseBlock->statementCount() == 2);
        CHECK(dynamic_cast<const VarDecl*>(elseBlock->statements()[0].get())->name() == "branch2");
        CHECK(dynamic_cast<const VarDecl*>(elseBlock->statements()[1].get())->name() == "tag2");
    }

    SECTION("if/else if/else cascade with declarations in each block") {
        const std::string src =
            "if (a) {\n"
            "    int x = 1;\n"
            "} else if (b) {\n"
            "    int y = 2;\n"
            "} else {\n"
            "    int z = 3;\n"
            "}";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* if1 = dynamic_cast<const IfStmt*>(stmt.get());
        REQUIRE(if1 != nullptr);
        auto* then1 = dynamic_cast<const CompoundStmt*>(if1->thenBranch());
        REQUIRE(then1 != nullptr);
        CHECK(dynamic_cast<const VarDecl*>(then1->statements()[0].get())->name() == "x");

        auto* if2 = dynamic_cast<const IfStmt*>(if1->elseBranch());
        REQUIRE(if2 != nullptr);
        auto* then2 = dynamic_cast<const CompoundStmt*>(if2->thenBranch());
        REQUIRE(then2 != nullptr);
        CHECK(dynamic_cast<const VarDecl*>(then2->statements()[0].get())->name() == "y");

        auto* else2 = dynamic_cast<const CompoundStmt*>(if2->elseBranch());
        REQUIRE(else2 != nullptr);
        CHECK(dynamic_cast<const VarDecl*>(else2->statements()[0].get())->name() == "z");
    }
}

// -----------------------------------------------------------------------------
// 4. Declarations inside loop bodies (while, do-while, for)
// -----------------------------------------------------------------------------
TEST_CASE("DeclarationsAndScopes: Declarations inside loop bodies", "[parser][decl][scope][loops]") {
    SECTION("while loop body with declarations: 'while (cond) { int n = 0; break; }'") {
        const std::string src =
            "while (cond) {\n"
            "    int n = 0;\n"
            "    const float step = 0.5;\n"
            "    break;\n"
            "}";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt.get());
        REQUIRE(whileStmt != nullptr);

        auto* body = dynamic_cast<const CompoundStmt*>(whileStmt->body());
        REQUIRE(body != nullptr);
        REQUIRE(body->statementCount() == 3);

        auto* d0 = dynamic_cast<const VarDecl*>(body->statements()[0].get());
        REQUIRE(d0 != nullptr);
        CHECK(d0->name() == "n");

        auto* d1 = dynamic_cast<const VarDecl*>(body->statements()[1].get());
        REQUIRE(d1 != nullptr);
        CHECK(d1->name() == "step");
        CHECK(d1->isConst());

        auto* s2 = dynamic_cast<const BreakStmt*>(body->statements()[2].get());
        REQUIRE(s2 != nullptr);
    }

    SECTION("do-while loop body with declarations: 'do { int item = 1; } while (x);'") {
        const std::string src =
            "do {\n"
            "    int item = 1;\n"
            "    continue;\n"
            "} while (x);";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* doWhileStmt = dynamic_cast<const DoWhileStmt*>(stmt.get());
        REQUIRE(doWhileStmt != nullptr);

        auto* body = dynamic_cast<const CompoundStmt*>(doWhileStmt->body());
        REQUIRE(body != nullptr);
        REQUIRE(body->statementCount() == 2);

        auto* d0 = dynamic_cast<const VarDecl*>(body->statements()[0].get());
        REQUIRE(d0 != nullptr);
        CHECK(d0->name() == "item");

        auto* s1 = dynamic_cast<const ContinueStmt*>(body->statements()[1].get());
        REQUIRE(s1 != nullptr);
    }

    SECTION("for loop body with declarations: 'for (;;) { int temp = 100; break; }'") {
        const std::string src =
            "for (;;) {\n"
            "    int temp = 100;\n"
            "    static int runCount = 0;\n"
            "    break;\n"
            "}";
        Parser parser = makeParser(src);
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* forStmt = dynamic_cast<const ForStmt*>(stmt.get());
        REQUIRE(forStmt != nullptr);

        auto* body = dynamic_cast<const CompoundStmt*>(forStmt->body());
        REQUIRE(body != nullptr);
        REQUIRE(body->statementCount() == 3);

        auto* d0 = dynamic_cast<const VarDecl*>(body->statements()[0].get());
        REQUIRE(d0 != nullptr);
        CHECK(d0->name() == "temp");

        auto* d1 = dynamic_cast<const VarDecl*>(body->statements()[1].get());
        REQUIRE(d1 != nullptr);
        CHECK(d1->name() == "runCount");
        CHECK(d1->isStatic());

        auto* s2 = dynamic_cast<const BreakStmt*>(body->statements()[2].get());
        REQUIRE(s2 != nullptr);
    }
}

// -----------------------------------------------------------------------------
// 5. Blocks mixing declarations and statements (C99/C17 arbitrary order)
// -----------------------------------------------------------------------------
TEST_CASE("DeclarationsAndScopes: Blocks mixing declarations and statements", "[parser][decl][scope][mixed]") {
    SECTION("Interleaved declarations, if-statements, while-loops, and jump-statements") {
        const std::string src =
            "{\n"
            "    int a = 1;\n"
            "    if (a) {\n"
            "        break;\n"
            "    }\n"
            "    char b = 'Z';\n"
            "    while (b) {\n"
            "        continue;\n"
            "    }\n"
            "    const double c = 3.14;\n"
            "}";
        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        REQUIRE(block->statementCount() == 5);

        // 0: int a = 1;
        auto* s0 = dynamic_cast<const VarDecl*>(block->statements()[0].get());
        REQUIRE(s0 != nullptr);
        CHECK(s0->name() == "a");

        // 1: if (a) { break; }
        auto* s1 = dynamic_cast<const IfStmt*>(block->statements()[1].get());
        REQUIRE(s1 != nullptr);

        // 2: char b = 'Z';
        auto* s2 = dynamic_cast<const VarDecl*>(block->statements()[2].get());
        REQUIRE(s2 != nullptr);
        CHECK(s2->name() == "b");

        // 3: while (b) { continue; }
        auto* s3 = dynamic_cast<const WhileStmt*>(block->statements()[3].get());
        REQUIRE(s3 != nullptr);

        // 4: const double c = 3.14;
        auto* s4 = dynamic_cast<const VarDecl*>(block->statements()[4].get());
        REQUIRE(s4 != nullptr);
        CHECK(s4->name() == "c");
        CHECK(s4->isConst());
    }

    SECTION("Statement preceding first declaration in a block") {
        const std::string src =
            "{\n"
            "    if (cond) {\n"
            "        break;\n"
            "    }\n"
            "    int trailingVar = 100;\n"
            "}";
        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(block->statementCount() == 2);
        CHECK(dynamic_cast<const IfStmt*>(block->statements()[0].get()) != nullptr);
        auto* decl = dynamic_cast<const VarDecl*>(block->statements()[1].get());
        REQUIRE(decl != nullptr);
        CHECK(decl->name() == "trailingVar");
    }
}

// -----------------------------------------------------------------------------
// 6. Multiple nested scopes containing declarations with the same name
// -----------------------------------------------------------------------------
TEST_CASE("DeclarationsAndScopes: Lexical shadowing with same-named declarations", "[parser][decl][scope][shadowing]") {
    SECTION("Shadowing across three nested levels: outer 'x', middle 'x', inner 'x'") {
        const std::string src =
            "{\n"
            "    int x = 1;\n"
            "    {\n"
            "        int x = 2;\n"
            "        {\n"
            "            int x = 3;\n"
            "        }\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto l1 = parser.parseCompoundStatement();
        REQUIRE(l1 != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        REQUIRE(l1->statementCount() == 2);
        auto* d1 = dynamic_cast<const VarDecl*>(l1->statements()[0].get());
        REQUIRE(d1 != nullptr);
        CHECK(d1->name() == "x");
        CHECK(dynamic_cast<const LiteralExpression*>(d1->initializer())->value() == "1");

        auto* l2 = dynamic_cast<const CompoundStmt*>(l1->statements()[1].get());
        REQUIRE(l2 != nullptr);
        REQUIRE(l2->statementCount() == 2);
        auto* d2 = dynamic_cast<const VarDecl*>(l2->statements()[0].get());
        REQUIRE(d2 != nullptr);
        CHECK(d2->name() == "x");
        CHECK(dynamic_cast<const LiteralExpression*>(d2->initializer())->value() == "2");

        auto* l3 = dynamic_cast<const CompoundStmt*>(l2->statements()[1].get());
        REQUIRE(l3 != nullptr);
        REQUIRE(l3->statementCount() == 1);
        auto* d3 = dynamic_cast<const VarDecl*>(l3->statements()[0].get());
        REQUIRE(d3 != nullptr);
        CHECK(d3->name() == "x");
        CHECK(dynamic_cast<const LiteralExpression*>(d3->initializer())->value() == "3");

        // Verify distinct AST node pointers across scopes
        CHECK(d1 != d2);
        CHECK(d2 != d3);
        CHECK(d1 != d3);
    }

    SECTION("Different types for same identifier in nested scopes") {
        const std::string src =
            "{\n"
            "    int data = 100;\n"
            "    {\n"
            "        char data = 'X';\n"
            "        {\n"
            "            double data = 9.99;\n"
            "        }\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto l1 = parser.parseCompoundStatement();
        REQUIRE(l1 != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* d1 = dynamic_cast<const VarDecl*>(l1->statements()[0].get());
        REQUIRE(d1 != nullptr);
        CHECK(d1->typeKind() == TypeKind::Int);

        auto* l2 = dynamic_cast<const CompoundStmt*>(l1->statements()[1].get());
        REQUIRE(l2 != nullptr);
        auto* d2 = dynamic_cast<const VarDecl*>(l2->statements()[0].get());
        REQUIRE(d2 != nullptr);
        CHECK(d2->typeKind() == TypeKind::Char);

        auto* l3 = dynamic_cast<const CompoundStmt*>(l2->statements()[1].get());
        REQUIRE(l3 != nullptr);
        auto* d3 = dynamic_cast<const VarDecl*>(l3->statements()[0].get());
        REQUIRE(d3 != nullptr);
        CHECK(d3->typeKind() == TypeKind::Double);
    }

    SECTION("Same identifier across sibling scopes") {
        const std::string src =
            "{\n"
            "    {\n"
            "        int val = 10;\n"
            "    }\n"
            "    {\n"
            "        int val = 20;\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto root = parser.parseCompoundStatement();
        REQUIRE(root != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(root->statementCount() == 2);
        auto* b1 = dynamic_cast<const CompoundStmt*>(root->statements()[0].get());
        REQUIRE(b1 != nullptr);
        auto* d1 = dynamic_cast<const VarDecl*>(b1->statements()[0].get());
        REQUIRE(d1 != nullptr);
        CHECK(d1->name() == "val");
        CHECK(dynamic_cast<const LiteralExpression*>(d1->initializer())->value() == "10");

        auto* b2 = dynamic_cast<const CompoundStmt*>(root->statements()[1].get());
        REQUIRE(b2 != nullptr);
        auto* d2 = dynamic_cast<const VarDecl*>(b2->statements()[0].get());
        REQUIRE(d2 != nullptr);
        CHECK(d2->name() == "val");
        CHECK(dynamic_cast<const LiteralExpression*>(d2->initializer())->value() == "20");

        CHECK(d1 != d2);
    }
}

// -----------------------------------------------------------------------------
// 7. Declarations inside Function bodies
// -----------------------------------------------------------------------------
TEST_CASE("DeclarationsAndScopes: Function body AST with declarations", "[parser][decl][scope][function]") {
    SECTION("Function AST node correctly stores CompoundStmt body with declarations") {
        auto fn = std::make_unique<Function>("compute", Type::getInt(), SourceLocation{1, 1}, true);
        fn->setParameterListKind(ParameterListKind::Void);

        const std::string bodySrc =
            "{\n"
            "    int result = 0;\n"
            "    const int factor = 2;\n"
            "    static int cache = 100;\n"
            "}";
        Parser parser = makeParser(bodySrc);
        auto bodyBlock = parser.parseCompoundStatement();
        REQUIRE(bodyBlock != nullptr);
        CHECK_FALSE(parser.hasError());

        fn->addBodyStatement(std::move(bodyBlock));
        REQUIRE(fn->bodyStatementCount() == 1);

        auto* storedBody = dynamic_cast<const CompoundStmt*>(fn->body()[0].get());
        REQUIRE(storedBody != nullptr);
        REQUIRE(storedBody->statementCount() == 3);

        auto* d0 = dynamic_cast<const VarDecl*>(storedBody->statements()[0].get());
        REQUIRE(d0 != nullptr);
        CHECK(d0->name() == "result");
        CHECK_FALSE(d0->isConst());

        auto* d1 = dynamic_cast<const VarDecl*>(storedBody->statements()[1].get());
        REQUIRE(d1 != nullptr);
        CHECK(d1->name() == "factor");
        CHECK(d1->isConst());

        auto* d2 = dynamic_cast<const VarDecl*>(storedBody->statements()[2].get());
        REQUIRE(d2 != nullptr);
        CHECK(d2->name() == "cache");
        CHECK(d2->isStatic());

        // Verify ASTPrinter output for function with CompoundStmt body
        std::string tree = dumpAST(fn.get());
        CHECK(tree.find("Function (definition): int compute") != std::string::npos);
        CHECK(tree.find("Body:\n") != std::string::npos);
        CHECK(tree.find("CompoundStmt\n") != std::string::npos);
        CHECK(tree.find("VarDecl: int result") != std::string::npos);
        CHECK(tree.find("VarDecl: const int factor") != std::string::npos);
        CHECK(tree.find("VarDecl: static int cache") != std::string::npos);
    }

    SECTION("Function body with nested blocks and control flow containing declarations") {
        auto fn = std::make_unique<Function>("process", Type::getVoid(), SourceLocation{1, 1}, true);
        fn->setParameterListKind(ParameterListKind::Void);

        const std::string bodySrc =
            "{\n"
            "    int active = 1;\n"
            "    if (active) {\n"
            "        int branchVar = 10;\n"
            "    }\n"
            "    while (active) {\n"
            "        int loopVar = 20;\n"
            "        break;\n"
            "    }\n"
            "}";
        Parser parser = makeParser(bodySrc);
        auto bodyBlock = parser.parseCompoundStatement();
        REQUIRE(bodyBlock != nullptr);

        fn->addBodyStatement(std::move(bodyBlock));
        REQUIRE(fn->bodyStatementCount() == 1);

        auto* storedBody = dynamic_cast<const CompoundStmt*>(fn->body()[0].get());
        REQUIRE(storedBody != nullptr);
        REQUIRE(storedBody->statementCount() == 3);

        // Statement 0: VarDecl active
        CHECK(dynamic_cast<const VarDecl*>(storedBody->statements()[0].get())->name() == "active");

        // Statement 1: IfStmt with declaration in thenBranch
        auto* ifStmt = dynamic_cast<const IfStmt*>(storedBody->statements()[1].get());
        REQUIRE(ifStmt != nullptr);
        auto* ifBody = dynamic_cast<const CompoundStmt*>(ifStmt->thenBranch());
        REQUIRE(ifBody != nullptr);
        CHECK(dynamic_cast<const VarDecl*>(ifBody->statements()[0].get())->name() == "branchVar");

        // Statement 2: WhileStmt with declaration in body
        auto* whileStmt = dynamic_cast<const WhileStmt*>(storedBody->statements()[2].get());
        REQUIRE(whileStmt != nullptr);
        auto* whileBody = dynamic_cast<const CompoundStmt*>(whileStmt->body());
        REQUIRE(whileBody != nullptr);
        CHECK(dynamic_cast<const VarDecl*>(whileBody->statements()[0].get())->name() == "loopVar");
    }
}

// -----------------------------------------------------------------------------
// 8. ASTPrinter Output Verification
// -----------------------------------------------------------------------------
TEST_CASE("DeclarationsAndScopes: ASTPrinter formatting", "[ast][printer][scope]") {
    SECTION("Block with declarations and initializers") {
        const std::string src =
            "{\n"
            "    static const int count = 5;\n"
            "}";
        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        std::string tree = dumpAST(block.get());
        std::string expected =
            "CompoundStmt\n"
            "  VarDecl: static const int count\n"
            "    Initializer:\n"
            "      LiteralExpression (Integer): 5\n";
        CHECK(tree == expected);
    }

    SECTION("Nested block indentation") {
        const std::string src =
            "{\n"
            "    int a = 1;\n"
            "    {\n"
            "        int b = 2;\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        std::string tree = dumpAST(block.get());
        std::string expected =
            "CompoundStmt\n"
            "  VarDecl: int a\n"
            "    Initializer:\n"
            "      LiteralExpression (Integer): 1\n"
            "  CompoundStmt\n"
            "    VarDecl: int b\n"
            "      Initializer:\n"
            "        LiteralExpression (Integer): 2\n";
        CHECK(tree == expected);
    }
}

// -----------------------------------------------------------------------------
// 9. Error Handling and Negative Tests
// -----------------------------------------------------------------------------
TEST_CASE("DeclarationsAndScopes: Error handling and rejection of invalid declarations", "[parser][decl][scope][errors]") {
    SECTION("Unbraced declaration directly in while statement body is rejected") {
        Parser parser = makeParser("while (1) int x;");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement, found 'int'") != std::string::npos);
    }

    SECTION("Unbraced declaration directly in if then-branch is rejected") {
        Parser parser = makeParser("if (1) int x;");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement, found 'int'") != std::string::npos);
    }

    SECTION("Unbraced declaration directly in if else-branch is rejected") {
        Parser parser = makeParser("if (1) {} else int x;");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement, found 'int'") != std::string::npos);
    }

    SECTION("Unbraced declaration directly in for loop body is rejected") {
        Parser parser = makeParser("for (;;) int x;");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement, found 'int'") != std::string::npos);
    }

    SECTION("Unbraced declaration directly in do-while loop body is rejected") {
        Parser parser = makeParser("do int x; while (1);");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement, found 'int'") != std::string::npos);
    }

    SECTION("Missing identifier in block declaration: '{ int; }'") {
        Parser parser = makeParser("{ int; }");
        auto block = parser.parseCompoundStatement();
        CHECK(block == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected identifier after type specifier") != std::string::npos);
    }

    SECTION("Missing semicolon in block declaration: '{ int x }'") {
        Parser parser = makeParser("{ int x }");
        auto block = parser.parseCompoundStatement();
        CHECK(block == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';' after variable declaration") != std::string::npos);
    }

    SECTION("Missing expression after '=' in block declaration: '{ int x = ; }'") {
        Parser parser = makeParser("{ int x = ; }");
        auto block = parser.parseCompoundStatement();
        CHECK(block == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression after '='") != std::string::npos);
    }

    SECTION("Conflicting storage classes inside block: '{ static extern int x; }'") {
        Parser parser = makeParser("{ static extern int x; }");
        auto block = parser.parseCompoundStatement();
        CHECK(block == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Multiple storage class specifiers in declaration") != std::string::npos);
    }

    SECTION("Duplicate storage classes inside block: '{ static static int x; }'") {
        Parser parser = makeParser("{ static static int x; }");
        auto block = parser.parseCompoundStatement();
        CHECK(block == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Duplicate storage class specifier 'static'") != std::string::npos);
    }

    SECTION("Missing closing brace after declaration: '{ int x = 10;'") {
        Parser parser = makeParser("{ int x = 10;");
        auto block = parser.parseCompoundStatement();
        CHECK(block == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '}' to close compound statement") != std::string::npos);
    }
}

// -----------------------------------------------------------------------------
// 10. Source Location Tracking for Block Declarations
// -----------------------------------------------------------------------------
TEST_CASE("DeclarationsAndScopes: Source location tracking for block declarations", "[parser][decl][scope][location]") {
    SECTION("Declarations in outer and nested blocks track exact line and column") {
        const std::string src =
            "{\n"
            "    int x = 10;\n"
            "    {\n"
            "        const int y = 20;\n"
            "        {\n"
            "            static double z = 30.0;\n"
            "        }\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto l1 = parser.parseCompoundStatement();
        REQUIRE(l1 != nullptr);
        CHECK_FALSE(parser.hasError());

        // Line 1: '{'
        CHECK(l1->location().line == 1);
        CHECK(l1->location().column == 1);

        // Line 2: '    int x = 10;' -> column 5
        auto* dx = dynamic_cast<const VarDecl*>(l1->statements()[0].get());
        REQUIRE(dx != nullptr);
        CHECK(dx->location().line == 2);
        CHECK(dx->location().column == 5);

        // Line 3: '    {' -> column 5
        auto* l2 = dynamic_cast<const CompoundStmt*>(l1->statements()[1].get());
        REQUIRE(l2 != nullptr);
        CHECK(l2->location().line == 3);
        CHECK(l2->location().column == 5);

        // Line 4: '        const int y = 20;' -> column 9
        auto* dy = dynamic_cast<const VarDecl*>(l2->statements()[0].get());
        REQUIRE(dy != nullptr);
        CHECK(dy->location().line == 4);
        CHECK(dy->location().column == 9);

        // Line 5: '        {' -> column 9
        auto* l3 = dynamic_cast<const CompoundStmt*>(l2->statements()[1].get());
        REQUIRE(l3 != nullptr);
        CHECK(l3->location().line == 5);
        CHECK(l3->location().column == 9);

        // Line 6: '            static double z = 30.0;' -> column 13
        auto* dz = dynamic_cast<const VarDecl*>(l3->statements()[0].get());
        REQUIRE(dz != nullptr);
        CHECK(dz->location().line == 6);
        CHECK(dz->location().column == 13);
    }
}

// -----------------------------------------------------------------------------
// 11. Nested Blocks with Binary Expression Initializers (Prompt Pattern)
// -----------------------------------------------------------------------------
TEST_CASE("DeclarationsAndScopes: Nested blocks with binary expression initializers", "[parser][decl][scope][expressions]") {
    SECTION("Nested block with binary expression: 'int z = x + y;'") {
        const std::string src =
            "{\n"
            "    int x = 10;\n"
            "    {\n"
            "        int y = 20;\n"
            "        {\n"
            "            int z = x + y;\n"
            "        }\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto l1 = parser.parseCompoundStatement();
        REQUIRE(l1 != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* l2 = dynamic_cast<const CompoundStmt*>(l1->statements()[1].get());
        REQUIRE(l2 != nullptr);
        auto* l3 = dynamic_cast<const CompoundStmt*>(l2->statements()[1].get());
        REQUIRE(l3 != nullptr);

        auto* dz = dynamic_cast<const VarDecl*>(l3->statements()[0].get());
        REQUIRE(dz != nullptr);
        CHECK(dz->name() == "z");
        CHECK(dz->hasInitializer());

        auto* bin = dynamic_cast<const BinaryExpr*>(dz->initializer());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");
        CHECK(bin->toString() == "BinaryExpr(+, IdentifierExpression(x), IdentifierExpression(y))");
    }
}

// -----------------------------------------------------------------------------
// 12. Regression Verification for Existing Grammar
// -----------------------------------------------------------------------------
TEST_CASE("DeclarationsAndScopes: Regression verification for existing grammar", "[parser][decl][scope][regression]") {
    SECTION("Top-level function declarations continue working unchanged") {
        Parser p1 = makeParser("int calculate(int a, int b);");
        auto fn1 = p1.parseFunctionDeclaration();
        REQUIRE(fn1 != nullptr);
        CHECK_FALSE(p1.hasError());
        CHECK(fn1->name() == "calculate");
        CHECK(fn1->parameterCount() == 2);

        Parser p2 = makeParser("void run(void);");
        auto fn2 = p2.parseFunctionDeclaration();
        REQUIRE(fn2 != nullptr);
        CHECK_FALSE(p2.hasError());
        CHECK(fn2->name() == "run");
        CHECK(fn2->isParameterListVoid());
    }

    SECTION("Top-level translation unit with multiple declarations") {
        const std::string src =
            "int add(int a, int b);\n"
            "float scale(float f, double d);\n";
        Parser parser = makeParser(src);
        auto prog = parser.parseProgram();
        REQUIRE(prog != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(prog->statementCount() == 2);
    }

    SECTION("Top-level variable declarations without blocks continue working") {
        Parser parser = makeParser("int global_count = 100;");
        auto var = parser.parseVarDecl();
        REQUIRE(var != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(var->name() == "global_count");
        CHECK(var->typeKind() == TypeKind::Int);
    }

    SECTION("Ordinary control flow statements without declarations continue working") {
        Parser p1 = makeParser("while (x) { break; }");
        auto s1 = p1.parseStatement();
        REQUIRE(s1 != nullptr);
        CHECK_FALSE(p1.hasError());

        Parser p2 = makeParser("if (x) { break; } else { continue; }");
        auto s2 = p2.parseStatement();
        REQUIRE(s2 != nullptr);
        CHECK_FALSE(p2.hasError());

        Parser p3 = makeParser("for (;;) { break; }");
        auto s3 = p3.parseStatement();
        REQUIRE(s3 != nullptr);
        CHECK_FALSE(p3.hasError());

        Parser p4 = makeParser("do { continue; } while (x);");
        auto s4 = p4.parseStatement();
        REQUIRE(s4 != nullptr);
        CHECK_FALSE(p4.hasError());
    }
}
