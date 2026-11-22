#include <catch2/catch.hpp>

#include "semantic/checks/undeclared.h"
#include "semantic/symbol_table.h"
#include "ast/assign_expr.h"
#include "ast/binary_expr.h"
#include "ast/compound_stmt.h"
#include "ast/conditional_expr.h"
#include "ast/for_stmt.h"
#include "ast/function.h"
#include "ast/identifier_expression.h"
#include "ast/if_stmt.h"
#include "ast/literal_expression.h"
#include "ast/program.h"
#include "ast/unary_expr.h"
#include "ast/var_decl.h"
#include "ast/while_stmt.h"
#include "ast/do_while_stmt.h"
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
// DAY 48 TESTS — Undefined Identifier Detection
// =============================================================================

// -----------------------------------------------------------------------------
// Case 1: Undeclared Variable in an Assignment
// -----------------------------------------------------------------------------
TEST_CASE("Undeclared: Case 1 - Undeclared variable in assignment", "[semantic][undeclared][assignment]") {
    SECTION("Bare assignment to undeclared identifier 'x = 10'") {
        Parser parser = makeParser("x = 10");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        UndeclaredChecker checker;
        bool ok = checker.check(*expr);

        CHECK_FALSE(ok);
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);

        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.identifier == "x");
        CHECK(diag.message.find("use of undeclared identifier 'x'") != std::string::npos);
        CHECK_FALSE(diag.hasPreviousLocation);
    }

    SECTION("Assignment of undeclared variable to undeclared target 'x = y'") {
        Parser parser = makeParser("x = y");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        UndeclaredChecker checker;
        bool ok = checker.check(*expr);

        CHECK_FALSE(ok);
        REQUIRE(checker.errorCount() == 2);
        CHECK(checker.diagnostics()[0].identifier == "x");
        CHECK(checker.diagnostics()[1].identifier == "y");
    }

    SECTION("Compound assignment to undeclared identifier 'count += 5'") {
        Parser parser = makeParser("count += 5");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*expr));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "count");
    }

    SECTION("Declared target with undeclared right-hand value") {
        SymbolTable table;
        table.insert(Symbol::createVariable("total", Type::getInt()));

        UndeclaredChecker checker(table);
        Parser parser = makeParser("total = unknown_val");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "unknown_val");
    }
}

// -----------------------------------------------------------------------------
// Case 2: Undeclared Variable in an Expression
// -----------------------------------------------------------------------------
TEST_CASE("Undeclared: Case 2 - Undeclared variable in expressions", "[semantic][undeclared][expression]") {
    SECTION("Binary arithmetic expression with undeclared operand 'x + 5'") {
        Parser parser = makeParser("x + 5");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*expr));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "x");
    }

    SECTION("Unary expression with undeclared operand '-count'") {
        Parser parser = makeParser("-count");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*expr));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "count");
    }

    SECTION("Conditional / ternary expression with undeclared operands 'flag ? a : b'") {
        Parser parser = makeParser("flag ? a : b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*expr));
        REQUIRE(checker.errorCount() == 3);
        CHECK(checker.diagnostics()[0].identifier == "flag");
        CHECK(checker.diagnostics()[1].identifier == "a");
        CHECK(checker.diagnostics()[2].identifier == "b");
    }

    SECTION("VarDecl initializer references undeclared variable 'int y = x + 5;'") {
        Parser parser = makeParser("int y = x + 5;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);

        UndeclaredChecker checker;
        bool ok = checker.check(*decl);

        CHECK_FALSE(ok);
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "x");
        // Ensure 'y' was registered despite initializer error
        CHECK(checker.symbolTable().contains("y"));
    }
}

// -----------------------------------------------------------------------------
// Case 3: Declared Local Variable
// -----------------------------------------------------------------------------
TEST_CASE("Undeclared: Case 3 - Declared local variable resolves successfully", "[semantic][undeclared][local]") {
    SECTION("Local variable declared before its use in subsequent declaration initializer") {
        const std::string src =
            "{\n"
            "    int x = 10;\n"
            "    int y = x + 5;\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        UndeclaredChecker checker;
        bool ok = checker.check(*block);

        CHECK(ok);
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
    }

    SECTION("Multiple declared locals chained across initializers") {
        const std::string src =
            "{\n"
            "    int a = 1;\n"
            "    int b = a * 2;\n"
            "    int c = a + b;\n"
            "    int d = (c > b) ? a : b;\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        UndeclaredChecker checker;
        CHECK(checker.check(*block));
        CHECK(checker.errorCount() == 0);
    }

    SECTION("Self-referencing declaration 'int x = x + 1;' considers x declared per C17 §6.2.1 p7") {
        Parser parser = makeParser("int x = x + 1;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);

        UndeclaredChecker checker;
        CHECK(checker.check(*decl));
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// Case 4: Declared Global Variable
// -----------------------------------------------------------------------------
TEST_CASE("Undeclared: Case 4 - Declared global variable is accessible", "[semantic][undeclared][global]") {
    SECTION("Global variable resolved from function body") {
        auto prog = std::make_unique<Program>();

        // Global declaration: int g_value = 42;
        Parser p1 = makeParser("int g_value = 42;");
        prog->addStatement(p1.parseVarDecl());

        // Function: int main(void) { int local = g_value; }
        auto fn = std::make_unique<Function>("main", Type::getInt());
        fn->setIsDefinition(true);

        Parser p2 = makeParser("{\n    int local = g_value;\n}");
        fn->addBodyStatement(p2.parseCompoundStatement());
        prog->addStatement(std::move(fn));

        UndeclaredChecker checker;
        bool ok = checker.check(*prog);

        CHECK(ok);
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
    }

    SECTION("Multiple global variables referenced in subsequent global initializer") {
        auto prog = std::make_unique<Program>();

        Parser p1 = makeParser("int a = 10;");
        prog->addStatement(p1.parseVarDecl());

        Parser p2 = makeParser("int b = 20;");
        prog->addStatement(p2.parseVarDecl());

        Parser p3 = makeParser("int c = a + b;");
        prog->addStatement(p3.parseVarDecl());

        UndeclaredChecker checker;
        CHECK(checker.check(*prog));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Global variable used before its declaration produces undeclared diagnostic") {
        auto prog = std::make_unique<Program>();

        // Use 'late_var' before it is declared
        Parser p1 = makeParser("int early = late_var;");
        prog->addStatement(p1.parseVarDecl());

        Parser p2 = makeParser("int late_var = 100;");
        prog->addStatement(p2.parseVarDecl());

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*prog));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "late_var");
    }
}

// -----------------------------------------------------------------------------
// Case 5: Variable Visible from an Outer Scope
// -----------------------------------------------------------------------------
TEST_CASE("Undeclared: Case 5 - Variable visible from outer lexical scope", "[semantic][undeclared][outer_scope]") {
    SECTION("Two-level nesting: inner block accesses outer variable") {
        const std::string src =
            "{\n"
            "    int outer = 100;\n"
            "    {\n"
            "        int inner = outer + 1;\n"
            "    }\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        UndeclaredChecker checker;
        CHECK(checker.check(*block));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Outer variable accessible after inner block completes") {
        const std::string src =
            "{\n"
            "    int outer = 50;\n"
            "    {\n"
            "        int inner = 10;\n"
            "    }\n"
            "    int after = outer * 2;\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        UndeclaredChecker checker;
        CHECK(checker.check(*block));
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// Case 6: Variable Used After Its Scope Ends
// -----------------------------------------------------------------------------
TEST_CASE("Undeclared: Case 6 - Variable used after scope ends is rejected", "[semantic][undeclared][scope_exit]") {
    SECTION("Variable declared in block used outside block") {
        const std::string src =
            "{\n"
            "    {\n"
            "        int temp = 20;\n"
            "    }\n"
            "    int leak = temp;\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        UndeclaredChecker checker;
        bool ok = checker.check(*block);

        CHECK_FALSE(ok);
        REQUIRE(checker.errorCount() == 1);

        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.identifier == "temp");
        CHECK(diag.message.find("use of undeclared identifier 'temp'") != std::string::npos);
    }

    SECTION("Variable declared in 'if' branch used in 'else' branch") {
        Parser pThen = makeParser("{\n    int branch_var = 1;\n}");
        Parser pElse = makeParser("{\n    int other = branch_var;\n}");

        auto ifStmt = std::make_unique<IfStmt>(
            std::make_unique<LiteralExpression>(LiteralKind::Integer, "1"),
            pThen.parseCompoundStatement(),
            pElse.parseCompoundStatement()
        );

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*ifStmt));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "branch_var");
    }

    SECTION("Variable declared in 'while' loop body used after the loop") {
        const std::string src =
            "{\n"
            "    while (1) {\n"
            "        int loop_local = 99;\n"
            "    }\n"
            "    int after = loop_local;\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*block));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "loop_local");
    }
}

// -----------------------------------------------------------------------------
// Case 7: Nested Scope Visibility (Multi-Level & Sibling Blocks)
// -----------------------------------------------------------------------------
TEST_CASE("Undeclared: Case 7 - Multi-level nested scopes and sibling isolation", "[semantic][undeclared][nesting]") {
    SECTION("Three levels of nested scopes accessing all enclosing variables") {
        const std::string src =
            "{\n"
            "    int a = 1;\n"
            "    {\n"
            "        int b = 2;\n"
            "        {\n"
            "            int c = a + b + 3;\n"
            "        }\n"
            "    }\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        UndeclaredChecker checker;
        CHECK(checker.check(*block));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Sibling blocks are isolated: variable in block 1 not visible in block 2") {
        const std::string src =
            "{\n"
            "    {\n"
            "        int first = 10;\n"
            "    }\n"
            "    {\n"
            "        int second = first;\n"
            "    }\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*block));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "first");
    }

    SECTION("Deeply nested access to undeclared identifier at level 3") {
        const std::string src =
            "{\n"
            "    int a = 1;\n"
            "    {\n"
            "        int b = 2;\n"
            "        {\n"
            "            int c = a + b + missing;\n"
            "        }\n"
            "    }\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*block));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "missing");
    }
}

// -----------------------------------------------------------------------------
// Case 8: Multiple Undeclared Identifiers
// -----------------------------------------------------------------------------
TEST_CASE("Undeclared: Case 8 - Multiple undeclared identifiers detected", "[semantic][undeclared][multiple]") {
    SECTION("Single expression containing three distinct undeclared identifiers") {
        Parser parser = makeParser("a + b * c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        UndeclaredChecker checker;
        bool ok = checker.check(*expr);

        CHECK_FALSE(ok);
        REQUIRE(checker.errorCount() == 3);
        CHECK(checker.diagnostics()[0].identifier == "a");
        CHECK(checker.diagnostics()[1].identifier == "b");
        CHECK(checker.diagnostics()[2].identifier == "c");
    }

    SECTION("Chained assignment with multiple undeclared identifiers 'x = y = z'") {
        Parser parser = makeParser("x = y = z");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*expr));
        REQUIRE(checker.errorCount() == 3);
        CHECK(checker.diagnostics()[0].identifier == "x");
        CHECK(checker.diagnostics()[1].identifier == "y");
        CHECK(checker.diagnostics()[2].identifier == "z");
    }

    SECTION("Multiple declarations with undeclared initializers in same block") {
        const std::string src =
            "{\n"
            "    int x = missing_one;\n"
            "    int y = missing_two;\n"
            "    int z = missing_three;\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*block));
        REQUIRE(checker.errorCount() == 3);
        CHECK(checker.diagnostics()[0].identifier == "missing_one");
        CHECK(checker.diagnostics()[1].identifier == "missing_two");
        CHECK(checker.diagnostics()[2].identifier == "missing_three");
    }
}

// -----------------------------------------------------------------------------
// Case 9: Correct Diagnostic Identifier and Source Location
// -----------------------------------------------------------------------------
TEST_CASE("Undeclared: Case 9 - Diagnostic formatting and source location", "[semantic][undeclared][diagnostic]") {
    SECTION("Accurate line and column location in parsed expression") {
        const std::string src = "\n\n   bad_var + 10";
        Parser parser = makeParser(src);
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*expr));
        REQUIRE(checker.errorCount() == 1);

        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.identifier == "bad_var");
        CHECK(diag.location.line == 3);
        CHECK(diag.location.column == 4);
        CHECK_FALSE(diag.hasPreviousLocation);

        std::string formatted = diag.toString();
        CHECK(formatted.find("3:4: error: use of undeclared identifier 'bad_var'") != std::string::npos);
    }

    SECTION("Diagnostic accessors and toString without line numbers") {
        Diagnostic diag("val", "use of undeclared identifier 'val'", SourceLocation{0, 0});
        CHECK(diag.getIdentifier() == "val");
        CHECK(diag.getMessage() == "use of undeclared identifier 'val'");
        CHECK(diag.getLocation().line == 0);
        CHECK(diag.toString() == "error: use of undeclared identifier 'val'");
    }
}

// -----------------------------------------------------------------------------
// Case 10: Existing Redeclaration and Shadowing Behavior
// -----------------------------------------------------------------------------
TEST_CASE("Undeclared: Case 10 - Shadowing and duplicate declaration tolerance", "[semantic][undeclared][shadowing]") {
    SECTION("Legal inner-scope shadowing resolves inner variable") {
        const std::string src =
            "{\n"
            "    int x = 10;\n"
            "    {\n"
            "        int x = 20;\n"
            "        int y = x;\n" // resolves to inner x (20)
            "    }\n"
            "    int z = x;\n"     // resolves to outer x (10)
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        UndeclaredChecker checker;
        CHECK(checker.check(*block));
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
    }

    SECTION("Duplicate declaration in same scope does not crash; subsequent uses still resolve") {
        const std::string src =
            "{\n"
            "    int dup = 1;\n"
            "    int dup = 2;\n"
            "    int use = dup;\n"
            "}";

        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        UndeclaredChecker checker;
        // UndeclaredChecker only checks for undeclared identifiers; dup is declared
        CHECK(checker.check(*block));
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// Case 11: Functions and Parameter Scopes
// -----------------------------------------------------------------------------
TEST_CASE("Undeclared: Case 11 - Functions, parameters, and prototype scopes", "[semantic][undeclared][functions]") {
    SECTION("Function definition parameters resolve inside function body") {
        auto fn = std::make_unique<Function>("calculate", Type::getInt());
        fn->setIsDefinition(true);
        fn->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "count"));
        fn->addParameter(std::make_unique<VariableDeclaration>(Type::getFloat(), "factor"));

        Parser pBody = makeParser("{\n    float result = count * factor;\n}");
        fn->addBodyStatement(pBody.parseCompoundStatement());

        UndeclaredChecker checker;
        CHECK(checker.check(*fn));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Function parameter shadows global variable") {
        auto prog = std::make_unique<Program>();

        Parser pGlobal = makeParser("int val = 999;");
        prog->addStatement(pGlobal.parseVarDecl());

        auto fn = std::make_unique<Function>("foo", Type::getInt());
        fn->setIsDefinition(true);
        fn->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "val"));

        Parser pBody = makeParser("{\n    int res = val;\n}");
        fn->addBodyStatement(pBody.parseCompoundStatement());
        prog->addStatement(std::move(fn));

        UndeclaredChecker checker;
        CHECK(checker.check(*prog));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Function prototype parameters do NOT pollute the global scope") {
        auto prog = std::make_unique<Program>();

        // Prototype: int helper(int param_only);
        auto proto = std::make_unique<Function>("helper", Type::getInt());
        proto->setIsDefinition(false);
        proto->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "param_only"));
        prog->addStatement(std::move(proto));

        // Function: int main(void) { int leak = param_only; }
        auto mainFn = std::make_unique<Function>("main", Type::getInt());
        mainFn->setIsDefinition(true);
        Parser pMain = makeParser("{\n    int leak = param_only;\n}");
        mainFn->addBodyStatement(pMain.parseCompoundStatement());
        prog->addStatement(std::move(mainFn));

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*prog));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "param_only");
    }

    SECTION("Declared function prototype can be referenced without error") {
        auto prog = std::make_unique<Program>();

        auto proto = std::make_unique<Function>("compute", Type::getInt());
        proto->setIsDefinition(false);
        prog->addStatement(std::move(proto));

        auto mainFn = std::make_unique<Function>("main", Type::getInt());
        mainFn->setIsDefinition(true);
        Parser pMain = makeParser("{\n    int ptr = compute;\n}");
        mainFn->addBodyStatement(pMain.parseCompoundStatement());
        prog->addStatement(std::move(mainFn));

        UndeclaredChecker checker;
        CHECK(checker.check(*prog));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Undeclared function reference reports error") {
        auto mainFn = std::make_unique<Function>("main", Type::getInt());
        mainFn->setIsDefinition(true);
        Parser pMain = makeParser("{\n    int call = nonexistent_fn;\n}");
        mainFn->addBodyStatement(pMain.parseCompoundStatement());

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*mainFn));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "nonexistent_fn");
    }

    SECTION("Recursive reference inside function definition resolves") {
        auto fn = std::make_unique<Function>("factorial", Type::getInt());
        fn->setIsDefinition(true);
        Parser pBody = makeParser("{\n    int rec = factorial;\n}");
        fn->addBodyStatement(pBody.parseCompoundStatement());

        UndeclaredChecker checker;
        CHECK(checker.check(*fn));
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// Case 12: Control Flow Statement Expressions
// -----------------------------------------------------------------------------
TEST_CASE("Undeclared: Case 12 - Control flow statement expressions", "[semantic][undeclared][control_flow]") {
    SECTION("IfStmt condition with undeclared identifier") {
        Parser pCond = makeParser("unseen_flag");
        auto ifStmt = std::make_unique<IfStmt>(
            pCond.parseExpression(),
            std::make_unique<CompoundStmt>()
        );

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*ifStmt));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "unseen_flag");
    }

    SECTION("WhileStmt condition with declared vs undeclared identifier") {
        // Declared counter in symbol table
        SymbolTable table;
        table.insert(Symbol::createVariable("counter", Type::getInt()));

        UndeclaredChecker checker(table);
        Parser pWhile = makeParser("counter > 0");
        auto whileStmt = std::make_unique<WhileStmt>(
            pWhile.parseExpression(),
            std::make_unique<CompoundStmt>()
        );
        CHECK(checker.check(*whileStmt));

        // Now with undeclared identifier
        Parser pBad = makeParser("counter > limit");
        auto badWhile = std::make_unique<WhileStmt>(
            pBad.parseExpression(),
            std::make_unique<CompoundStmt>()
        );
        CHECK_FALSE(checker.check(*badWhile));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "limit");
    }

    SECTION("DoWhileStmt condition with undeclared identifier") {
        Parser pCond = makeParser("active");
        auto doWhile = std::make_unique<DoWhileStmt>(
            std::make_unique<CompoundStmt>(),
            pCond.parseExpression()
        );

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*doWhile));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "active");
    }

    SECTION("ForStmt clauses with declared and undeclared identifiers") {
        SymbolTable table;
        table.insert(Symbol::createVariable("i", Type::getInt()));

        UndeclaredChecker checker(table);

        // for (i = 0; i < max_count; i = i + 1)
        Parser pInit = makeParser("i = 0");
        Parser pCond = makeParser("i < max_count"); // max_count is undeclared
        Parser pPost = makeParser("i = i + 1");

        auto forStmt = std::make_unique<ForStmt>(
            pInit.parseExpression(),
            pCond.parseExpression(),
            pPost.parseExpression(),
            std::make_unique<CompoundStmt>()
        );

        CHECK_FALSE(checker.check(*forStmt));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "max_count");
    }
}

// -----------------------------------------------------------------------------
// Case 13: Convenience Checking Functions and State Reset
// -----------------------------------------------------------------------------
TEST_CASE("Undeclared: Case 13 - Convenience APIs and clear()", "[semantic][undeclared][convenience]") {
    SECTION("checkUndeclared for Expression") {
        Parser parser = makeParser("x + 10");
        auto expr = parser.parseExpression();
        auto diags = checkUndeclared(*expr);
        REQUIRE(diags.size() == 1);
        CHECK(diags[0].identifier == "x");
    }

    SECTION("checkUndeclared for Statement") {
        Parser parser = makeParser("int a = missing;");
        auto decl = parser.parseVarDecl();
        auto diags = checkUndeclared(*decl);
        REQUIRE(diags.size() == 1);
        CHECK(diags[0].identifier == "missing");
    }

    SECTION("checkUndeclared for Program with external SymbolTable") {
        SymbolTable table;
        table.insert(Symbol::createVariable("declared_sym", Type::getInt()));

        auto prog = std::make_unique<Program>();
        Parser p1 = makeParser("int use1 = declared_sym;");
        prog->addStatement(p1.parseVarDecl());
        Parser p2 = makeParser("int use2 = missing_sym;");
        prog->addStatement(p2.parseVarDecl());

        auto diags = checkUndeclared(*prog, table);
        REQUIRE(diags.size() == 1);
        CHECK(diags[0].identifier == "missing_sym");
    }

    SECTION("clear() resets diagnostics and symbol table state") {
        UndeclaredChecker checker;

        Parser p1 = makeParser("x = 1");
        checker.check(*p1.parseExpression());
        CHECK(checker.hasErrors());
        CHECK(checker.errorCount() == 1);

        checker.clear();
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
        CHECK(checker.symbolTable().empty());
    }

    SECTION("check(const ASTNode&) polymorphic dispatch") {
        UndeclaredChecker checker;

        Parser p1 = makeParser("x + 5");
        auto expr = p1.parseExpression();
        const ASTNode& nodeExpr = *expr;
        CHECK_FALSE(checker.check(nodeExpr));

        Parser p2 = makeParser("int valid = 10;");
        auto decl = p2.parseVarDecl();
        const ASTNode& nodeDecl = *decl;
        UndeclaredChecker checker2;
        CHECK(checker2.check(nodeDecl));
    }
}
