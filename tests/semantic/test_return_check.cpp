#include <catch2/catch.hpp>

#include "semantic/checks/return_check.h"
#include "semantic/symbol_table.h"
#include "semantic/type_checker.h"
#include "ast/compound_stmt.h"
#include "ast/for_stmt.h"
#include "ast/function.h"
#include "ast/identifier_expression.h"
#include "ast/if_stmt.h"
#include "ast/literal_expression.h"
#include "ast/program.h"
#include "ast/return_stmt.h"
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

// Helper to parse a ReturnStmt from return statement source text
static std::unique_ptr<ReturnStmt> parseReturn(const std::string& src) {
    Parser p = makeParser(src);
    return p.parseReturnStatement();
}

// Helper to create a ReturnStmt with an expression parsed from string
static std::unique_ptr<ReturnStmt> makeReturn(const std::string& exprSrc, SourceLocation loc = {}) {
    if (exprSrc.empty()) {
        return std::make_unique<ReturnStmt>(nullptr, loc);
    }
    Parser p = makeParser(exprSrc);
    return std::make_unique<ReturnStmt>(p.parseExpression(), loc);
}

// =============================================================================
// DAY 51 TESTS — Return Statement Validation
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Correct Integer Return
// -----------------------------------------------------------------------------
TEST_CASE("ReturnCheck: 1 - Correct integer return", "[semantic][return_check][int]") {
    ReturnChecker checker;

    SECTION("Direct return statement validation: int calculate() { return 10; }") {
        auto ret = parseReturn("return 10;");
        REQUIRE(ret != nullptr);
        CHECK(ret->hasExpression());

        Type expectedInt(TypeKind::Int);
        CHECK(checker.checkReturn(*ret, expectedInt, "calculate"));
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
    }

    SECTION("Function AST traversal with integer return") {
        auto fn = std::make_unique<Function>("calculate", Type::getInt());
        fn->setIsDefinition(true);
        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(makeReturn("42"));
        fn->addBodyStatement(std::move(body));

        CHECK(checker.check(*fn));
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 2. Correct Floating-Point Return
// -----------------------------------------------------------------------------
TEST_CASE("ReturnCheck: 2 - Correct floating-point return", "[semantic][return_check][float]") {
    ReturnChecker checker;

    SECTION("Float function returning float literal: 3.14f") {
        auto ret = parseReturn("return 3.14f;");
        REQUIRE(ret != nullptr);

        Type expectedFloat(TypeKind::Float);
        CHECK(checker.checkReturn(*ret, expectedFloat, "get_pi"));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Double function returning double literal: 3.14") {
        auto ret = parseReturn("return 3.14;");
        REQUIRE(ret != nullptr);

        Type expectedDouble(TypeKind::Double);
        CHECK(checker.checkReturn(*ret, expectedDouble, "get_precise_pi"));
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 3. Compatible Implicit Numeric Conversion
// -----------------------------------------------------------------------------
TEST_CASE("ReturnCheck: 3 - Compatible implicit numeric conversion", "[semantic][return_check][conversion]") {
    ReturnChecker checker;

    SECTION("Float function returning int value: return 10;") {
        auto ret = parseReturn("return 10;");
        REQUIRE(ret != nullptr);

        Type expectedFloat(TypeKind::Float);
        CHECK(checker.checkReturn(*ret, expectedFloat, "calculate"));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Double function returning float value: return 2.5f;") {
        auto ret = parseReturn("return 2.5f;");
        REQUIRE(ret != nullptr);

        Type expectedDouble(TypeKind::Double);
        CHECK(checker.checkReturn(*ret, expectedDouble, "compute"));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Int function returning char literal: return 'A';") {
        auto ret = parseReturn("return 'A';");
        REQUIRE(ret != nullptr);

        Type expectedInt(TypeKind::Int);
        CHECK(checker.checkReturn(*ret, expectedInt, "to_int"));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Char function returning int value: return 65;") {
        auto ret = parseReturn("return 65;");
        REQUIRE(ret != nullptr);

        Type expectedChar(TypeKind::Char);
        CHECK(checker.checkReturn(*ret, expectedChar, "to_char"));
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 4. Incompatible Return Type
// -----------------------------------------------------------------------------
TEST_CASE("ReturnCheck: 4 - Incompatible return type", "[semantic][return_check][incompatible]") {
    ReturnChecker checker;

    SECTION("int calculate(void) { return \"hello\"; } produces incompatible diagnostic") {
        auto ret = parseReturn("return \"hello\";");
        REQUIRE(ret != nullptr);

        Type expectedInt(TypeKind::Int);
        CHECK_FALSE(checker.checkReturn(*ret, expectedInt, "calculate"));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);

        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.identifier == "calculate");
        CHECK(diag.message.find("incompatible return type in function 'calculate'") != std::string::npos);
        CHECK(diag.message.find("cannot convert string literal to 'int'") != std::string::npos);
    }

    SECTION("void function returning string literal") {
        auto ret = parseReturn("return \"error\";");
        REQUIRE(ret != nullptr);

        Type expectedVoid(TypeKind::Void);
        CHECK_FALSE(checker.checkReturn(*ret, expectedVoid, "log_msg"));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message.find("void function 'log_msg' should not return a value") != std::string::npos);
    }
}

// -----------------------------------------------------------------------------
// 5. Void Function with Empty Return
// -----------------------------------------------------------------------------
TEST_CASE("ReturnCheck: 5 - Void function with return;", "[semantic][return_check][void_empty]") {
    ReturnChecker checker;

    SECTION("void display(void) { return; } is valid") {
        auto ret = parseReturn("return;");
        REQUIRE(ret != nullptr);
        CHECK_FALSE(ret->hasExpression());

        Type expectedVoid(TypeKind::Void);
        CHECK(checker.checkReturn(*ret, expectedVoid, "display"));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Function AST with empty return in void function") {
        auto fn = std::make_unique<Function>("display", Type::getVoid());
        fn->setIsDefinition(true);
        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(parseReturn("return;"));
        fn->addBodyStatement(std::move(body));

        CHECK(checker.check(*fn));
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 6. Void Function Returning a Value
// -----------------------------------------------------------------------------
TEST_CASE("ReturnCheck: 6 - Void function with return value;", "[semantic][return_check][void_value]") {
    ReturnChecker checker;

    SECTION("void display(void) { return 10; } is diagnosed") {
        auto ret = parseReturn("return 10;");
        REQUIRE(ret != nullptr);

        Type expectedVoid(TypeKind::Void);
        CHECK_FALSE(checker.checkReturn(*ret, expectedVoid, "display"));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);

        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.identifier == "display");
        CHECK(diag.message == "void function 'display' should not return a value");
    }

    SECTION("Full Function AST with return value in void function") {
        auto fn = std::make_unique<Function>("do_work", Type::getVoid());
        fn->setIsDefinition(true);
        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(parseReturn("return 42;"));
        fn->addBodyStatement(std::move(body));

        CHECK_FALSE(checker.check(*fn));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "void function 'do_work' should not return a value");
    }
}

// -----------------------------------------------------------------------------
// 7. Non-Void Function with Empty Return
// -----------------------------------------------------------------------------
TEST_CASE("ReturnCheck: 7 - Non-void function with return;", "[semantic][return_check][nonvoid_empty]") {
    ReturnChecker checker;

    SECTION("int calculate(void) { return; } is diagnosed") {
        auto ret = parseReturn("return;");
        REQUIRE(ret != nullptr);
        CHECK_FALSE(ret->hasExpression());

        Type expectedInt(TypeKind::Int);
        CHECK_FALSE(checker.checkReturn(*ret, expectedInt, "calculate"));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);

        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.identifier == "calculate");
        CHECK(diag.message == "non-void function 'calculate' should return a value");
    }

    SECTION("float compute(void) { return; } in Function AST") {
        auto fn = std::make_unique<Function>("compute", Type::getFloat());
        fn->setIsDefinition(true);
        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(parseReturn("return;"));
        fn->addBodyStatement(std::move(body));

        CHECK_FALSE(checker.check(*fn));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "non-void function 'compute' should return a value");
    }
}

// -----------------------------------------------------------------------------
// 8. Multiple Return Statements
// -----------------------------------------------------------------------------
TEST_CASE("ReturnCheck: 8 - Multiple return statements", "[semantic][return_check][multiple]") {
    ReturnChecker checker;

    SECTION("Multiple valid return statements in same function") {
        auto fn = std::make_unique<Function>("clamp", Type::getInt());
        fn->setIsDefinition(true);

        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(std::make_unique<VarDecl>(Type::getInt(), "x", std::make_unique<LiteralExpression>(LiteralKind::Integer, "5")));

        // if (x < 0) { return 0; }
        Parser pCond = makeParser("x < 0");
        auto cond = pCond.parseExpression();
        auto ifBody = std::make_unique<CompoundStmt>();
        ifBody->addStatement(makeReturn("0"));
        body->addStatement(std::make_unique<IfStmt>(std::move(cond), std::move(ifBody), nullptr));

        // return x;
        body->addStatement(makeReturn("x"));
        fn->addBodyStatement(std::move(body));

        CHECK(checker.check(*fn));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Multiple returns with one invalid return reports exactly one diagnostic") {
        auto fn = std::make_unique<Function>("check_num", Type::getInt());
        fn->setIsDefinition(true);

        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(std::make_unique<VarDecl>(Type::getInt(), "n", std::make_unique<LiteralExpression>(LiteralKind::Integer, "10")));

        // if (n == 0) { return "zero"; }
        Parser pCond = makeParser("n == 0");
        auto cond = pCond.parseExpression();
        auto ifBody = std::make_unique<CompoundStmt>();
        ifBody->addStatement(makeReturn("\"zero\""));
        body->addStatement(std::make_unique<IfStmt>(std::move(cond), std::move(ifBody), nullptr));

        // return n;
        body->addStatement(makeReturn("n"));
        fn->addBodyStatement(std::move(body));

        CHECK_FALSE(checker.check(*fn));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message.find("cannot convert string literal to 'int'") != std::string::npos);
    }
}

// -----------------------------------------------------------------------------
// 9. Return Statements Inside Nested Blocks
// -----------------------------------------------------------------------------
TEST_CASE("ReturnCheck: 9 - Return statements inside nested blocks", "[semantic][return_check][nested_blocks]") {
    ReturnChecker checker;

    SECTION("Deeply nested compound blocks preserve enclosing function return type") {
        auto fn = std::make_unique<Function>("nested_fn", Type::getInt());
        fn->setIsDefinition(true);

        auto inner3 = std::make_unique<CompoundStmt>();
        inner3->addStatement(std::make_unique<VarDecl>(Type::getInt(), "val", std::make_unique<LiteralExpression>(LiteralKind::Integer, "100")));
        inner3->addStatement(makeReturn("val"));

        auto inner2 = std::make_unique<CompoundStmt>();
        inner2->addStatement(std::move(inner3));

        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(std::move(inner2));
        fn->addBodyStatement(std::move(body));

        CHECK(checker.check(*fn));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Nested block with invalid return is caught") {
        auto fn = std::make_unique<Function>("nested_invalid", Type::getInt());
        fn->setIsDefinition(true);

        auto inner = std::make_unique<CompoundStmt>();
        inner->addStatement(makeReturn("\"bad\""));

        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(std::move(inner));
        fn->addBodyStatement(std::move(body));

        CHECK_FALSE(checker.check(*fn));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "nested_invalid");
    }
}

// -----------------------------------------------------------------------------
// 10. Return Statements Inside Supported Conditional Statements
// -----------------------------------------------------------------------------
TEST_CASE("ReturnCheck: 10 - Return statements inside conditional statements", "[semantic][return_check][conditionals]") {
    ReturnChecker checker;

    SECTION("Both then and else branches return compatible values") {
        auto fn = std::make_unique<Function>("abs_val", Type::getInt());
        fn->setIsDefinition(true);

        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(std::make_unique<VarDecl>(Type::getInt(), "x", std::make_unique<LiteralExpression>(LiteralKind::Integer, "-5")));

        Parser pCond = makeParser("x < 0");
        auto cond = pCond.parseExpression();

        auto thenBranch = std::make_unique<CompoundStmt>();
        thenBranch->addStatement(makeReturn("-x"));

        auto elseBranch = std::make_unique<CompoundStmt>();
        elseBranch->addStatement(makeReturn("x"));

        body->addStatement(std::make_unique<IfStmt>(std::move(cond), std::move(thenBranch), std::move(elseBranch)));
        fn->addBodyStatement(std::move(body));

        CHECK(checker.check(*fn));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Else branch has incompatible return type") {
        auto fn = std::make_unique<Function>("bad_branch", Type::getInt());
        fn->setIsDefinition(true);

        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(std::make_unique<VarDecl>(Type::getInt(), "flag", std::make_unique<LiteralExpression>(LiteralKind::Integer, "1")));

        Parser pCond = makeParser("flag");
        auto cond = pCond.parseExpression();

        auto thenBranch = std::make_unique<CompoundStmt>();
        thenBranch->addStatement(makeReturn("1"));

        auto elseBranch = std::make_unique<CompoundStmt>();
        elseBranch->addStatement(makeReturn("\"false\""));

        body->addStatement(std::make_unique<IfStmt>(std::move(cond), std::move(thenBranch), std::move(elseBranch)));
        fn->addBodyStatement(std::move(body));

        CHECK_FALSE(checker.check(*fn));
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message.find("cannot convert string literal to 'int'") != std::string::npos);
    }
}

// -----------------------------------------------------------------------------
// 11. Separate Functions with Different Return Types
// -----------------------------------------------------------------------------
TEST_CASE("ReturnCheck: 11 - Separate functions with different return types", "[semantic][return_check][separate_fns]") {
    Program program;

    // Function 1: int f1(void) { return 10; }
    auto fn1 = std::make_unique<Function>("f1", Type::getInt());
    fn1->setIsDefinition(true);
    auto body1 = std::make_unique<CompoundStmt>();
    body1->addStatement(makeReturn("10"));
    fn1->addBodyStatement(std::move(body1));
    program.addStatement(std::move(fn1));

    // Function 2: void f2(void) { return; }
    auto fn2 = std::make_unique<Function>("f2", Type::getVoid());
    fn2->setIsDefinition(true);
    auto body2 = std::make_unique<CompoundStmt>();
    body2->addStatement(parseReturn("return;"));
    fn2->addBodyStatement(std::move(body2));
    program.addStatement(std::move(fn2));

    // Function 3: float f3(void) { return 3.14f; }
    auto fn3 = std::make_unique<Function>("f3", Type::getFloat());
    fn3->setIsDefinition(true);
    auto body3 = std::make_unique<CompoundStmt>();
    body3->addStatement(makeReturn("3.14f"));
    fn3->addBodyStatement(std::move(body3));
    program.addStatement(std::move(fn3));

    ReturnChecker checker;
    CHECK(checker.check(program));
    CHECK_FALSE(checker.hasErrors());
    CHECK(checker.errorCount() == 0);
}

// -----------------------------------------------------------------------------
// 12. Correct Source-Location Diagnostics
// -----------------------------------------------------------------------------
TEST_CASE("ReturnCheck: 12 - Correct source-location diagnostics", "[semantic][return_check][location]") {
    ReturnChecker checker;

    SECTION("Empty return in non-void function reports location of 'return'") {
        SourceLocation loc{3, 5};
        ReturnStmt ret(nullptr, loc);

        Type expectedInt(TypeKind::Int);
        CHECK_FALSE(checker.checkReturn(ret, expectedInt, "calc"));
        REQUIRE(checker.errorCount() == 1);

        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.location.line == 3);
        CHECK(diag.location.column == 5);
    }

    SECTION("Return value in void function reports location") {
        SourceLocation loc{7, 12};
        auto expr = std::make_unique<LiteralExpression>(LiteralKind::Integer, "10", SourceLocation{7, 19});
        ReturnStmt ret(std::move(expr), loc);

        Type expectedVoid(TypeKind::Void);
        CHECK_FALSE(checker.checkReturn(ret, expectedVoid, "proc"));
        REQUIRE(checker.errorCount() == 1);

        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.location.line == 7);
        CHECK(diag.location.column == 12);
    }
}

// -----------------------------------------------------------------------------
// 13. No Duplicate or Cascading Diagnostics when Expression Inference Fails
// -----------------------------------------------------------------------------
TEST_CASE("ReturnCheck: 13 - No duplicate diagnostics when expression inference fails", "[semantic][return_check][cascading]") {
    ReturnChecker checker;

    SECTION("Undeclared identifier in return expression does not produce cascading return type error") {
        auto ret = parseReturn("return undeclared_var;");
        REQUIRE(ret != nullptr);

        Type expectedInt(TypeKind::Int);
        // Returns false to signify failure, but does not emit a misleading type error
        bool ok = checker.checkReturn(*ret, expectedInt, "test_fn");
        CHECK_FALSE(ok);
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
    }
}

// -----------------------------------------------------------------------------
// 14. Missing-Return Behavior (Structural Analysis)
// -----------------------------------------------------------------------------
TEST_CASE("ReturnCheck: 14 - Missing-return behavior", "[semantic][return_check][missing_return]") {
    ReturnChecker checker;

    SECTION("hasReturn detects return in top-level body") {
        auto fn = std::make_unique<Function>("has_ret", Type::getInt());
        fn->setIsDefinition(true);
        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(makeReturn("10"));
        fn->addBodyStatement(std::move(body));

        CHECK(ReturnChecker::hasReturn(*fn));
        CHECK(checker.checkMissingReturn(*fn));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("hasReturn detects return in nested block") {
        auto fn = std::make_unique<Function>("nested_ret", Type::getInt());
        fn->setIsDefinition(true);
        auto body = std::make_unique<CompoundStmt>();
        auto inner = std::make_unique<CompoundStmt>();
        inner->addStatement(makeReturn("42"));
        body->addStatement(std::move(inner));
        fn->addBodyStatement(std::move(body));

        CHECK(ReturnChecker::hasReturn(*fn));
        CHECK(checker.checkMissingReturn(*fn));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Non-void function with no return statement is detected") {
        auto fn = std::make_unique<Function>("missing_ret", Type::getInt(), SourceLocation{15, 1});
        fn->setIsDefinition(true);
        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(std::make_unique<VarDecl>(Type::getInt(), "x", std::make_unique<LiteralExpression>(LiteralKind::Integer, "10")));
        fn->addBodyStatement(std::move(body));

        CHECK_FALSE(ReturnChecker::hasReturn(*fn));
        CHECK_FALSE(checker.checkMissingReturn(*fn));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "non-void function 'missing_ret' does not return a value");
    }

    SECTION("Main function definition is permitted to omit explicit return (C99/C17 §5.1.2.2.3)") {
        auto fn = std::make_unique<Function>("main", Type::getInt());
        fn->setIsDefinition(true);
        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(std::make_unique<VarDecl>(Type::getInt(), "exit_code", std::make_unique<LiteralExpression>(LiteralKind::Integer, "0")));
        fn->addBodyStatement(std::move(body));

        CHECK_FALSE(ReturnChecker::hasReturn(*fn));
        CHECK(checker.checkMissingReturn(*fn)); // Allowed for main
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Void function definition with no return statement is valid") {
        auto fn = std::make_unique<Function>("void_no_ret", Type::getVoid());
        fn->setIsDefinition(true);
        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(std::make_unique<VarDecl>(Type::getInt(), "temp", std::make_unique<LiteralExpression>(LiteralKind::Integer, "1")));
        fn->addBodyStatement(std::move(body));

        CHECK_FALSE(ReturnChecker::hasReturn(*fn));
        CHECK(checker.checkMissingReturn(*fn)); // Valid for void
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 15. Additional Control Flow, Loops, and State Management Tests
// -----------------------------------------------------------------------------
TEST_CASE("ReturnCheck: 15 - Additional control flow, loops, and state management", "[semantic][return_check][control_flow]") {
    ReturnChecker checker;

    SECTION("Return inside while loop") {
        auto fn = std::make_unique<Function>("search_while", Type::getInt());
        fn->setIsDefinition(true);

        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(std::make_unique<VarDecl>(Type::getInt(), "i", std::make_unique<LiteralExpression>(LiteralKind::Integer, "0")));

        Parser pCond = makeParser("i < 10");
        auto whileCond = pCond.parseExpression();
        auto whileBody = std::make_unique<CompoundStmt>();
        whileBody->addStatement(makeReturn("i"));

        body->addStatement(std::make_unique<WhileStmt>(std::move(whileCond), std::move(whileBody)));
        body->addStatement(makeReturn("-1"));
        fn->addBodyStatement(std::move(body));

        CHECK(checker.check(*fn));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Return inside for loop") {
        auto fn = std::make_unique<Function>("search_for", Type::getInt());
        fn->setIsDefinition(true);

        auto body = std::make_unique<CompoundStmt>();

        Parser pInit = makeParser("0");
        Parser pCond = makeParser("5");
        Parser pPost = makeParser("1");
        auto forBody = std::make_unique<CompoundStmt>();
        forBody->addStatement(makeReturn("42"));

        body->addStatement(std::make_unique<ForStmt>(
            pInit.parseExpression(),
            pCond.parseExpression(),
            pPost.parseExpression(),
            std::move(forBody)
        ));
        body->addStatement(makeReturn("0"));
        fn->addBodyStatement(std::move(body));

        CHECK(checker.check(*fn));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Return inside do-while loop") {
        auto fn = std::make_unique<Function>("search_do_while", Type::getInt());
        fn->setIsDefinition(true);

        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(std::make_unique<VarDecl>(Type::getInt(), "x", std::make_unique<LiteralExpression>(LiteralKind::Integer, "1")));

        auto doBody = std::make_unique<CompoundStmt>();
        doBody->addStatement(makeReturn("x"));

        Parser pCond = makeParser("x < 10");
        auto doCond = pCond.parseExpression();

        body->addStatement(std::make_unique<DoWhileStmt>(std::move(doBody), std::move(doCond)));
        fn->addBodyStatement(std::move(body));

        CHECK(checker.check(*fn));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Return statement outside of any function reports error") {
        auto ret = parseReturn("return 42;");
        REQUIRE(ret != nullptr);

        CHECK_FALSE(checker.check(*ret));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "return statement outside of a function");
    }

    SECTION("Convenience checkReturns functions") {
        auto fn = std::make_unique<Function>("helper", Type::getInt());
        fn->setIsDefinition(true);
        auto body = std::make_unique<CompoundStmt>();
        body->addStatement(parseReturn("return;"));
        fn->addBodyStatement(std::move(body));

        auto diags = checkReturns(*fn);
        REQUIRE(diags.size() == 1);
        CHECK(diags[0].message == "non-void function 'helper' should return a value");
    }

    SECTION("clear() resets diagnostics and state") {
        auto ret = parseReturn("return 42;");
        checker.check(*ret);
        CHECK(checker.hasErrors());

        checker.clear();
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
    }
}
