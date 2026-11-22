#include <catch2/catch.hpp>

#include "semantic/checks/redeclaration.h"
#include "semantic/symbol_table.h"
#include "ast/compound_stmt.h"
#include "ast/function.h"
#include "ast/program.h"
#include "ast/var_decl.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "types/type.h"

#include <memory>
#include <string>
#include <vector>

// Helper to construct a Parser from a source string
static Parser makeParser(const std::string& src) {
    Lexer lexer(src);
    return Parser(lexer.tokenize());
}

// =============================================================================
// DAY 47 TESTS — Duplicate Declaration Detection
// =============================================================================

// -----------------------------------------------------------------------------
// Case A: Duplicate Global Variable Declarations — ERROR
// -----------------------------------------------------------------------------
TEST_CASE("Redeclaration: Case A - Duplicate global variable declarations", "[semantic][redeclaration][case_a]") {
    SECTION("Two identical global variable declarations in Program") {
        auto prog = std::make_unique<Program>();

        Parser p1 = makeParser("int x;");
        auto d1 = p1.parseVarDecl();
        REQUIRE(d1 != nullptr);
        prog->addStatement(std::move(d1));

        Parser p2 = makeParser("int x;");
        auto d2 = p2.parseVarDecl();
        REQUIRE(d2 != nullptr);
        prog->addStatement(std::move(d2));

        RedeclarationChecker checker;
        bool ok = checker.check(*prog);

        CHECK_FALSE(ok);
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);

        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.identifier == "x");
        CHECK(diag.message.find("duplicate declaration of 'x'") != std::string::npos);

        // Verify the symbol table retained the first declaration
        CHECK(checker.symbolTable().contains("x"));
        CHECK(checker.symbolTable().size() == 1);
    }

    SECTION("Global variables with different types but same name") {
        auto prog = std::make_unique<Program>();

        Parser p1 = makeParser("int count;");
        prog->addStatement(p1.parseVarDecl());

        Parser p2 = makeParser("float count;");
        prog->addStatement(p2.parseVarDecl());

        RedeclarationChecker checker;
        CHECK_FALSE(checker.check(*prog));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "count");

        // The first declaration (int) is preserved in the symbol table
        const Symbol* sym = checker.symbolTable().lookup("count");
        REQUIRE(sym != nullptr);
        CHECK(sym->typeKind() == TypeKind::Int);
    }

    SECTION("Three duplicates report two errors") {
        auto prog = std::make_unique<Program>();

        Parser p1 = makeParser("int x;");
        prog->addStatement(p1.parseVarDecl());

        Parser p2 = makeParser("int x;");
        prog->addStatement(p2.parseVarDecl());

        Parser p3 = makeParser("int x;");
        prog->addStatement(p3.parseVarDecl());

        RedeclarationChecker checker;
        CHECK_FALSE(checker.check(*prog));
        CHECK(checker.errorCount() == 2);
    }
}

// -----------------------------------------------------------------------------
// Case B: Duplicate Local Variable in the Same Block — ERROR
// -----------------------------------------------------------------------------
TEST_CASE("Redeclaration: Case B - Duplicate local variable in same block", "[semantic][redeclaration][case_b]") {
    SECTION("Block with two 'int x' declarations") {
        const std::string src =
            "{\n"
            "    int x = 10;\n"
            "    int x = 20;\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        RedeclarationChecker checker;
        bool ok = checker.check(*block);

        CHECK_FALSE(ok);
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);

        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.identifier == "x");
        CHECK(diag.message.find("duplicate declaration of 'x'") != std::string::npos);
        CHECK(diag.location.line == 3); // Second declaration is at line 3
    }

    SECTION("Multiple duplicate pairs in the same block") {
        const std::string src =
            "{\n"
            "    int a = 1;\n"
            "    char b = 'A';\n"
            "    int a = 2;\n"
            "    char b = 'B';\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        RedeclarationChecker checker;
        CHECK_FALSE(checker.check(*block));
        REQUIRE(checker.errorCount() == 2);
        CHECK(checker.diagnostics()[0].identifier == "a");
        CHECK(checker.diagnostics()[1].identifier == "b");
    }
}

// -----------------------------------------------------------------------------
// Case C: Shadowing a Global Variable — ALLOWED
// -----------------------------------------------------------------------------
TEST_CASE("Redeclaration: Case C - Shadowing a global variable", "[semantic][redeclaration][case_c]") {
    SECTION("Global x = 10 and function block x = 20") {
        auto prog = std::make_unique<Program>();

        // Global: int x = 10;
        Parser pGlobal = makeParser("int x = 10;");
        prog->addStatement(pGlobal.parseVarDecl());

        // Function: int main(void) { int x = 20; return 0; }
        auto mainFn = std::make_unique<Function>("main", Type::getInt(), SourceLocation{3, 1}, true);
        mainFn->setParameterListKind(ParameterListKind::Void);

        Parser pBody = makeParser("{\n    int x = 20;\n}");
        auto bodyBlock = pBody.parseCompoundStatement();
        REQUIRE(bodyBlock != nullptr);
        mainFn->addBodyStatement(std::move(bodyBlock));
        prog->addStatement(std::move(mainFn));

        RedeclarationChecker checker;
        bool ok = checker.check(*prog);

        CHECK(ok);
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
    }

    SECTION("Global and block checked sequentially in custom scope") {
        SymbolTable table;
        RedeclarationChecker checker(table);

        Parser pGlobal = makeParser("int x = 10;");
        auto gDecl = pGlobal.parseVarDecl();
        REQUIRE(gDecl != nullptr);
        CHECK(checker.check(*gDecl));

        CHECK(table.contains("x"));
        CHECK(table.lookup("x")->typeKind() == TypeKind::Int);

        // Enter a local block scope
        Parser pBlock = makeParser("{\n    float x = 2.5;\n}");
        auto block = pBlock.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK(checker.check(*block)); // Block internally pushes and pops scope

        CHECK_FALSE(checker.hasErrors());
        // Global x is still intact after block pop
        CHECK(table.contains("x"));
        CHECK(table.lookup("x")->typeKind() == TypeKind::Int);
    }
}

// -----------------------------------------------------------------------------
// Case D: Nested Block Shadowing — ALLOWED
// -----------------------------------------------------------------------------
TEST_CASE("Redeclaration: Case D - Nested block shadowing", "[semantic][redeclaration][case_d]") {
    SECTION("Outer block x shadowed by inner block x") {
        const std::string src =
            "{\n"
            "    int x = 10;\n"
            "    {\n"
            "        int x = 20;\n"
            "    }\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        RedeclarationChecker checker;
        bool ok = checker.check(*block);

        CHECK(ok);
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
    }

    SECTION("Deeply nested block shadowing (3 levels: int -> float -> char)") {
        const std::string src =
            "{\n"
            "    int x = 1;\n"
            "    {\n"
            "        float x = 2.0;\n"
            "        {\n"
            "            char x = 'C';\n"
            "        }\n"
            "    }\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        RedeclarationChecker checker;
        CHECK(checker.check(*block));
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// Case E: Duplicate Declaration in an Inner Scope — ERROR
// -----------------------------------------------------------------------------
TEST_CASE("Redeclaration: Case E - Duplicate declaration in inner scope", "[semantic][redeclaration][case_e]") {
    SECTION("Outer x is valid; inner scope duplicates y") {
        const std::string src =
            "{\n"
            "    int x = 10;\n"
            "    {\n"
            "        int y = 20;\n"
            "        int y = 30;\n"
            "    }\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        RedeclarationChecker checker;
        bool ok = checker.check(*block);

        CHECK_FALSE(ok);
        REQUIRE(checker.errorCount() == 1);

        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.identifier == "y");
        CHECK(diag.message.find("duplicate declaration of 'y'") != std::string::npos);
        CHECK(diag.location.line == 5); // Second y is at line 5
    }

    SECTION("Shadowing outer x AND duplicating inner y") {
        const std::string src =
            "{\n"
            "    int x = 10;\n"
            "    int y = 100;\n"
            "    {\n"
            "        int x = 20;\n"      // Legal shadowing of x
            "        int z = 1;\n"
            "        int z = 2;\n"       // Duplicate of z in inner block
            "    }\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        RedeclarationChecker checker;
        CHECK_FALSE(checker.check(*block));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "z");
    }
}

// -----------------------------------------------------------------------------
// Case F: Same Local Name in Separate Functions — ALLOWED
// -----------------------------------------------------------------------------
TEST_CASE("Redeclaration: Case F - Same local name in separate functions", "[semantic][redeclaration][case_f]") {
    SECTION("Two separate functions both declare local 'value'") {
        auto prog = std::make_unique<Program>();

        // Function 1: void first(void) { int value; }
        auto fn1 = std::make_unique<Function>("first", Type::getVoid(), SourceLocation{1, 1}, true);
        fn1->setParameterListKind(ParameterListKind::Void);
        Parser p1 = makeParser("{\n    int value;\n}");
        fn1->addBodyStatement(p1.parseCompoundStatement());
        prog->addStatement(std::move(fn1));

        // Function 2: void second(void) { int value; }
        auto fn2 = std::make_unique<Function>("second", Type::getVoid(), SourceLocation{5, 1}, true);
        fn2->setParameterListKind(ParameterListKind::Void);
        Parser p2 = makeParser("{\n    int value;\n}");
        fn2->addBodyStatement(p2.parseCompoundStatement());
        prog->addStatement(std::move(fn2));

        RedeclarationChecker checker;
        bool ok = checker.check(*prog);

        CHECK(ok);
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
    }

    SECTION("Three consecutive sibling compound blocks") {
        const std::string src =
            "{\n"
            "    {\n"
            "        int temp = 1;\n"
            "    }\n"
            "    {\n"
            "        int temp = 2;\n"
            "    }\n"
            "    {\n"
            "        int temp = 3;\n"
            "    }\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        RedeclarationChecker checker;
        CHECK(checker.check(*block));
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 7. Scope Cleanup and Restoration After Leaving Blocks
// -----------------------------------------------------------------------------
TEST_CASE("Redeclaration: Scope cleanup and symbol table state after check", "[semantic][redeclaration][cleanup]") {
    SECTION("Block scope exit restores outer scope visibility") {
        SymbolTable table;
        RedeclarationChecker checker(table);

        // Global variable
        Parser pG = makeParser("int x = 10;");
        checker.check(*pG.parseVarDecl());

        CHECK(table.scopeDepth() == 1);
        CHECK(table.currentScopeLevel() == 0);
        CHECK(table.contains("x"));
        CHECK(table.lookup("x")->typeKind() == TypeKind::Int);

        // Block with shadowing
        Parser pB = makeParser("{\n    float x = 3.14;\n}");
        checker.check(*pB.parseCompoundStatement());

        // Checker left the block; scope depth must return to 1
        CHECK(table.scopeDepth() == 1);
        CHECK(table.currentScopeLevel() == 0);
        CHECK(table.contains("x"));
        CHECK(table.lookup("x")->typeKind() == TypeKind::Int);
    }

    SECTION("Error in block does not corrupt outer scope stack depth") {
        SymbolTable table;
        RedeclarationChecker checker(table);

        Parser pB = makeParser("{\n    int bad = 1;\n    int bad = 2;\n}");
        checker.check(*pB.parseCompoundStatement());

        CHECK(checker.hasErrors());
        // Scope stack must be cleanly unwound even when errors occurred inside
        CHECK(table.scopeDepth() == 1);
        CHECK(table.currentScopeLevel() == 0);
    }
}

// -----------------------------------------------------------------------------
// 8. Diagnostics Formatting & Source Location Verification
// -----------------------------------------------------------------------------
TEST_CASE("Redeclaration: Diagnostic formatting and source location", "[semantic][redeclaration][diagnostic]") {
    SECTION("Diagnostic stores exact line and column of duplicate") {
        const std::string src =
            "{\n"
            "    int my_var = 1;\n"
            "    float my_var = 2.0;\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        RedeclarationChecker checker;
        checker.check(*block);

        REQUIRE(checker.errorCount() == 1);
        const auto& diag = checker.diagnostics()[0];

        CHECK(diag.identifier == "my_var");
        CHECK(diag.location.line == 3);
        CHECK(diag.previousLocation.line == 2);
        CHECK(diag.hasPreviousLocation);

        std::string str = diag.toString();
        CHECK(str.find("3:") != std::string::npos);
        CHECK(str.find("error:") != std::string::npos);
        CHECK(str.find("duplicate declaration of 'my_var'") != std::string::npos);
        CHECK(str.find("previously declared at 2:") != std::string::npos);
    }
}

// -----------------------------------------------------------------------------
// 9. Function Declarations, Definitions, and Parameter Scope
// -----------------------------------------------------------------------------
TEST_CASE("Redeclaration: Functions and parameter scope", "[semantic][redeclaration][function]") {
    SECTION("Prototype followed by definition is ALLOWED") {
        auto prog = std::make_unique<Program>();

        // Prototype: int add(int a, int b);
        Parser p1 = makeParser("int add(int a, int b);");
        prog->addStatement(p1.parseFunctionDeclaration());

        // Definition: int add(int a, int b) { return 0; }
        auto def = std::make_unique<Function>("add", Type::getInt(), SourceLocation{3, 1}, true);
        def->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "a"));
        def->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "b"));
        prog->addStatement(std::move(def));

        RedeclarationChecker checker;
        CHECK(checker.check(*prog));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Duplicate function definitions produce redefinition error") {
        auto prog = std::make_unique<Program>();

        auto def1 = std::make_unique<Function>("foo", Type::getVoid(), SourceLocation{1, 1}, true);
        def1->setParameterListKind(ParameterListKind::Void);
        prog->addStatement(std::move(def1));

        auto def2 = std::make_unique<Function>("foo", Type::getVoid(), SourceLocation{4, 1}, true);
        def2->setParameterListKind(ParameterListKind::Void);
        prog->addStatement(std::move(def2));

        RedeclarationChecker checker;
        CHECK_FALSE(checker.check(*prog));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "foo");
        CHECK(checker.diagnostics()[0].message.find("redefinition") != std::string::npos);
    }

    SECTION("Variable and function sharing same name in global scope is an error") {
        auto prog = std::make_unique<Program>();

        Parser pVar = makeParser("int calc;");
        prog->addStatement(pVar.parseVarDecl());

        Parser pFn = makeParser("int calc(void);");
        prog->addStatement(pFn.parseFunctionDeclaration());

        RedeclarationChecker checker;
        CHECK_FALSE(checker.check(*prog));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "calc");
    }

    SECTION("Duplicate function parameter name is an error") {
        auto fn = std::make_unique<Function>("f", Type::getInt(), SourceLocation{1, 1}, false);
        fn->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "p"));
        fn->addParameter(std::make_unique<VariableDeclaration>(Type::getFloat(), "p"));

        RedeclarationChecker checker;
        CHECK_FALSE(checker.check(*fn));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "p");
    }
}

// -----------------------------------------------------------------------------
// 10. Convenience Function checkRedeclarations()
// -----------------------------------------------------------------------------
TEST_CASE("Redeclaration: Convenience checkRedeclarations API", "[semantic][redeclaration][convenience]") {
    SECTION("Clean block returns empty vector") {
        Parser p = makeParser("{\n    int a = 1;\n    int b = 2;\n}");
        auto block = p.parseCompoundStatement();
        REQUIRE(block != nullptr);

        auto diags = checkRedeclarations(*block);
        CHECK(diags.empty());
    }

    SECTION("Duplicate block returns diagnostics vector") {
        Parser p = makeParser("{\n    int a = 1;\n    int a = 2;\n}");
        auto block = p.parseCompoundStatement();
        REQUIRE(block != nullptr);

        auto diags = checkRedeclarations(*block);
        REQUIRE(diags.size() == 1);
        CHECK(diags[0].identifier == "a");
    }
}

// -----------------------------------------------------------------------------
// 11. Function Parameters and Outermost Body Scope Sharing (C99/C17 §6.2.1 p4)
// -----------------------------------------------------------------------------
TEST_CASE("Redeclaration: Function parameters and outermost body scope sharing", "[semantic][redeclaration][func_scope]") {
    SECTION("Parameter and outermost body declaration conflict: 'int calculate(int x) { int x = 20; }'") {
        auto fn = std::make_unique<Function>("calculate", Type::getInt(), SourceLocation{1, 1}, true);
        fn->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "x", nullptr, SourceLocation{1, 19}));

        // Outermost body block containing 'int x = 20;'
        const std::string bodySrc =
            "{\n"
            "    int x = 20;\n"
            "}";
        Parser p = makeParser(bodySrc);
        auto bodyBlock = p.parseCompoundStatement();
        REQUIRE(bodyBlock != nullptr);
        fn->addBodyStatement(std::move(bodyBlock));

        RedeclarationChecker checker;
        bool ok = checker.check(*fn);

        CHECK_FALSE(ok);
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);

        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.identifier == "x");
        CHECK(diag.message.find("duplicate declaration of 'x'") != std::string::npos);
        CHECK(diag.location.line == 2);
        CHECK(diag.previousLocation.line == 1);
        CHECK(diag.hasPreviousLocation);
    }

    SECTION("Parameter shadowed inside nested block is ALLOWED: 'int calculate(int x) { { int x = 20; } }'") {
        auto fn = std::make_unique<Function>("calculate", Type::getInt(), SourceLocation{1, 1}, true);
        fn->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "x", nullptr, SourceLocation{1, 19}));

        // Outermost block containing a nested block '{ int x = 20; }'
        const std::string bodySrc =
            "{\n"
            "    {\n"
            "        int x = 20;\n"
            "    }\n"
            "}";
        Parser p = makeParser(bodySrc);
        auto bodyBlock = p.parseCompoundStatement();
        REQUIRE(bodyBlock != nullptr);
        fn->addBodyStatement(std::move(bodyBlock));

        RedeclarationChecker checker;
        bool ok = checker.check(*fn);

        CHECK(ok);
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
    }

    SECTION("Same parameter name in separate functions is ALLOWED: 'int first(int value); int second(int value);'") {
        auto prog = std::make_unique<Program>();

        // int first(int value) { int other = 1; }
        auto fn1 = std::make_unique<Function>("first", Type::getInt(), SourceLocation{1, 1}, true);
        fn1->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "value", nullptr, SourceLocation{1, 15}));
        Parser p1 = makeParser("{\n    int other = 1;\n}");
        fn1->addBodyStatement(p1.parseCompoundStatement());
        prog->addStatement(std::move(fn1));

        // int second(int value) { int other = 2; }
        auto fn2 = std::make_unique<Function>("second", Type::getInt(), SourceLocation{5, 1}, true);
        fn2->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "value", nullptr, SourceLocation{5, 16}));
        Parser p2 = makeParser("{\n    int other = 2;\n}");
        fn2->addBodyStatement(p2.parseCompoundStatement());
        prog->addStatement(std::move(fn2));

        RedeclarationChecker checker;
        bool ok = checker.check(*prog);

        CHECK(ok);
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
    }

    SECTION("Multiple parameters where second parameter is redeclared in outermost body") {
        auto fn = std::make_unique<Function>("compute", Type::getInt(), SourceLocation{1, 1}, true);
        fn->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "a", nullptr, SourceLocation{1, 17}));
        fn->addParameter(std::make_unique<VariableDeclaration>(Type::getFloat(), "b", nullptr, SourceLocation{1, 26}));

        const std::string bodySrc =
            "{\n"
            "    float b = 1.5;\n"
            "}";
        Parser p = makeParser(bodySrc);
        fn->addBodyStatement(p.parseCompoundStatement());

        RedeclarationChecker checker;
        CHECK_FALSE(checker.check(*fn));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "b");
    }
}
