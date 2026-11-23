#include <catch2/catch.hpp>

#include "semantic/checks/call_check.h"
#include "semantic/checks/undeclared.h"
#include "semantic/symbol_table.h"
#include "semantic/type_checker.h"
#include "ast/call_expr.h"
#include "ast/compound_stmt.h"
#include "ast/function.h"
#include "ast/identifier_expression.h"
#include "ast/literal_expression.h"
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
// DAY 50 TESTS — Function-Call Validation
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Valid Function Call with Matching Argument Count and Types
// -----------------------------------------------------------------------------
TEST_CASE("CallCheck: 1 - Valid function call with matching argument count and types", "[semantic][call_check][valid]") {
    SymbolTable table;
    table.insert(Symbol::createFunction(
        "add",
        Type::getInt(),
        {Type::getInt(), Type::getInt()},
        false,
        ParameterListKind::Typed
    ));

    CallChecker checker(table);

    SECTION("add(10, 20) via parser is accepted") {
        Parser p = makeParser("add(10, 20)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        const auto* call = dynamic_cast<const CallExpr*>(expr.get());
        REQUIRE(call != nullptr);
        CHECK(call->calleeName() == "add");
        CHECK(call->argumentCount() == 2);

        CHECK(checker.check(*expr));
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
    }

    SECTION("Programmatic CallExpr is accepted") {
        std::vector<std::unique_ptr<Expression>> args;
        args.push_back(std::make_unique<LiteralExpression>(LiteralKind::Integer, "5"));
        args.push_back(std::make_unique<LiteralExpression>(LiteralKind::Integer, "15"));
        CallExpr call("add", std::move(args));

        CHECK(checker.checkCall(call));
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 2. Too Few Arguments
// -----------------------------------------------------------------------------
TEST_CASE("CallCheck: 2 - Too few arguments to function", "[semantic][call_check][too_few]") {
    SymbolTable table;
    table.insert(Symbol::createFunction(
        "add",
        Type::getInt(),
        {Type::getInt(), Type::getInt()},
        false,
        ParameterListKind::Typed
    ));

    CallChecker checker(table);

    SECTION("add(10) generates too few arguments diagnostic") {
        Parser p = makeParser("add(10)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "too few arguments to function 'add'");
        CHECK(checker.diagnostics()[0].identifier == "add");
    }

    SECTION("add() with 0 arguments generates too few arguments diagnostic") {
        Parser p = makeParser("add()");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "too few arguments to function 'add'");
    }
}

// -----------------------------------------------------------------------------
// 3. Too Many Arguments
// -----------------------------------------------------------------------------
TEST_CASE("CallCheck: 3 - Too many arguments to function", "[semantic][call_check][too_many]") {
    SymbolTable table;
    table.insert(Symbol::createFunction(
        "add",
        Type::getInt(),
        {Type::getInt(), Type::getInt()},
        false,
        ParameterListKind::Typed
    ));

    CallChecker checker(table);

    SECTION("add(10, 20, 30) generates too many arguments diagnostic") {
        Parser p = makeParser("add(10, 20, 30)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "too many arguments to function 'add'");
        CHECK(checker.diagnostics()[0].identifier == "add");
    }

    SECTION("add(1, 2, 3, 4, 5) generates too many arguments diagnostic") {
        Parser p = makeParser("add(1, 2, 3, 4, 5)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        CHECK(checker.diagnostics()[0].message == "too many arguments to function 'add'");
    }
}

// -----------------------------------------------------------------------------
// 4. Zero-Parameter Function Called Correctly
// -----------------------------------------------------------------------------
TEST_CASE("CallCheck: 4 - Zero-parameter function called correctly", "[semantic][call_check][zero_param]") {
    SymbolTable table;
    table.insert(Symbol::createFunction(
        "display",
        Type::getVoid(),
        {},
        false,
        ParameterListKind::Void
    ));

    CallChecker checker(table);

    SECTION("display() is valid") {
        Parser p = makeParser("display()");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        const auto* call = dynamic_cast<const CallExpr*>(expr.get());
        REQUIRE(call != nullptr);
        CHECK(call->argumentCount() == 0);

        CHECK(checker.check(*expr));
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
    }
}

// -----------------------------------------------------------------------------
// 5. Arguments Supplied to a (void) Function
// -----------------------------------------------------------------------------
TEST_CASE("CallCheck: 5 - Arguments supplied to a (void) function are rejected", "[semantic][call_check][void_args]") {
    SymbolTable table;
    table.insert(Symbol::createFunction(
        "display",
        Type::getVoid(),
        {},
        false,
        ParameterListKind::Void
    ));

    CallChecker checker(table);

    SECTION("display(10) generates too many arguments diagnostic") {
        Parser p = makeParser("display(10)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "too many arguments to function 'display'");
        CHECK(checker.diagnostics()[0].identifier == "display");
    }

    SECTION("display(10, 20) generates too many arguments diagnostic") {
        Parser p = makeParser("display(10, 20)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        CHECK(checker.diagnostics()[0].message == "too many arguments to function 'display'");
    }
}

// -----------------------------------------------------------------------------
// 6. Valid Implicit Numeric Argument Conversion
// -----------------------------------------------------------------------------
TEST_CASE("CallCheck: 6 - Valid implicit numeric argument conversions", "[semantic][call_check][conversion]") {
    SymbolTable table;
    table.insert(Symbol::createFunction(
        "add",
        Type::getInt(),
        {Type::getInt(), Type::getInt()},
        false,
        ParameterListKind::Typed
    ));
    table.insert(Symbol::createFunction(
        "scale",
        Type::getFloat(),
        {Type::getFloat(), Type::getDouble()},
        false,
        ParameterListKind::Typed
    ));

    CallChecker checker(table);

    SECTION("add(10, 2.5) follows existing conversion rules (double -> int is implicit)") {
        Parser p = makeParser("add(10, 2.5)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK(checker.check(*expr));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("scale('A', 42) converts char to float and int to double") {
        Parser p = makeParser("scale('A', 42)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK(checker.check(*expr));
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 7. Incompatible Argument Type
// -----------------------------------------------------------------------------
TEST_CASE("CallCheck: 7 - Incompatible argument type is diagnosed", "[semantic][call_check][incompatible_type]") {
    SymbolTable table;
    table.insert(Symbol::createFunction(
        "add",
        Type::getInt(),
        {Type::getInt(), Type::getInt()},
        false,
        ParameterListKind::Typed
    ));
    table.insert(Symbol::createFunction(
        "void_fn",
        Type::getVoid(),
        {},
        false,
        ParameterListKind::Void
    ));

    CallChecker checker(table);

    SECTION("add(10, \"hello\") generates incompatible type diagnostic for string argument") {
        Parser p = makeParser("add(10, \"hello\")");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() >= 1);
        CHECK(checker.diagnostics()[0].message.find("incompatible type for argument 2 of 'add'") != std::string::npos);
        CHECK(checker.diagnostics()[0].identifier == "add");
    }

    SECTION("add(10, void_fn()) generates incompatible type diagnostic for void argument") {
        Parser p = makeParser("add(10, void_fn())");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() >= 1);
        CHECK(checker.diagnostics()[0].message.find("incompatible type for argument 2 of 'add'") != std::string::npos);
        CHECK(checker.diagnostics()[0].message.find("cannot convert 'void' to 'int'") != std::string::npos);
    }
}

// -----------------------------------------------------------------------------
// 8. Multiple Incompatible Arguments
// -----------------------------------------------------------------------------
TEST_CASE("CallCheck: 8 - Multiple incompatible arguments produce distinct diagnostics", "[semantic][call_check][multi_incompat]") {
    SymbolTable table;
    table.insert(Symbol::createFunction(
        "two_params",
        Type::getInt(),
        {Type::getInt(), Type::getFloat()},
        false,
        ParameterListKind::Typed
    ));
    table.insert(Symbol::createFunction(
        "get_void",
        Type::getVoid(),
        {},
        false,
        ParameterListKind::Void
    ));

    CallChecker checker(table);

    SECTION("two_params(\"first\", get_void()) reports errors for both arguments") {
        Parser p = makeParser("two_params(\"first\", get_void())");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 2);

        CHECK(checker.diagnostics()[0].message.find("incompatible type for argument 1 of 'two_params'") != std::string::npos);
        CHECK(checker.diagnostics()[1].message.find("incompatible type for argument 2 of 'two_params'") != std::string::npos);
    }
}

// -----------------------------------------------------------------------------
// 9. Undeclared Function Name
// -----------------------------------------------------------------------------
TEST_CASE("CallCheck: 9 - Undeclared function name is detected without misleading argument errors", "[semantic][call_check][undeclared]") {
    CallChecker checker;

    SECTION("unknown_fn(10, 20) reports undeclared identifier error only") {
        Parser p = makeParser("unknown_fn(10, 20)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "use of undeclared identifier 'unknown_fn'");
        CHECK(checker.diagnostics()[0].identifier == "unknown_fn");
    }

    SECTION("nonexistent_fn() reports undeclared identifier error only") {
        Parser p = makeParser("nonexistent_fn()");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "use of undeclared identifier 'nonexistent_fn'");
    }
}

// -----------------------------------------------------------------------------
// 10. Calling a Variable as If It Were a Function
// -----------------------------------------------------------------------------
TEST_CASE("CallCheck: 10 - Calling a variable as if it were a function is rejected", "[semantic][call_check][var_call]") {
    SymbolTable table;
    table.insert(Symbol::createVariable("x", Type::getInt()));
    table.insert(Symbol::createVariable("pi", Type::getDouble()));

    CallChecker checker(table);

    SECTION("x(20) reports called object is not a function") {
        Parser p = makeParser("x(20)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "called object 'x' is not a function");
        CHECK(checker.diagnostics()[0].identifier == "x");
    }

    SECTION("pi() reports called object is not a function without argument errors") {
        Parser p = makeParser("pi()");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "called object 'pi' is not a function");
        CHECK(checker.diagnostics()[0].identifier == "pi");
    }
}

// -----------------------------------------------------------------------------
// 11. Function Prototype Followed by Definition
// -----------------------------------------------------------------------------
TEST_CASE("CallCheck: 11 - Function prototype followed by definition resolves correctly", "[semantic][call_check][proto_def]") {
    const std::string src =
        "int compute(int x);\n"
        "int compute(int x) {\n"
        "    int res = x * 2;\n"
        "    return res;\n"
        "}";

    Parser pProg = makeParser(src);
    auto prog = pProg.parseProgram();
    REQUIRE(prog != nullptr);

    SymbolTable table;
    table.populate(*prog);

    CallChecker checker(table);

    SECTION("compute(42) resolves and validates against the function signature") {
        Parser pCall = makeParser("compute(42)");
        auto call = pCall.parseExpression();
        REQUIRE(call != nullptr);

        CHECK(checker.check(*call));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("compute() with 0 arguments reports too few arguments") {
        Parser pBad = makeParser("compute()");
        auto call = pBad.parseExpression();
        REQUIRE(call != nullptr);

        CHECK_FALSE(checker.check(*call));
        CHECK(checker.hasErrors());
        CHECK(checker.diagnostics()[0].message == "too few arguments to function 'compute'");
    }
}

// -----------------------------------------------------------------------------
// 12. Different Function Signatures
// -----------------------------------------------------------------------------
TEST_CASE("CallCheck: 12 - Diverse function signatures validation", "[semantic][call_check][signatures]") {
    SymbolTable table;
    // float transform(char tag, int id, float factor, double scale);
    table.insert(Symbol::createFunction(
        "transform",
        Type::getFloat(),
        {Type::getChar(), Type::getInt(), Type::getFloat(), Type::getDouble()},
        false,
        ParameterListKind::Typed
    ));
    // void reset_all(void);
    table.insert(Symbol::createFunction(
        "reset_all",
        Type::getVoid(),
        {},
        false,
        ParameterListKind::Void
    ));

    CallChecker checker(table);

    SECTION("Matching all 4 arguments of different types is valid") {
        Parser p = makeParser("transform('T', 100, 1.5, 3.14)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK(checker.check(*expr));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Argument count error on 4-parameter function") {
        Parser p = makeParser("transform('T', 100)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.diagnostics()[0].message == "too few arguments to function 'transform'");
    }

    SECTION("Incompatible argument on 3rd parameter") {
        Parser p = makeParser("transform('T', 100, \"bad\", 3.14)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.diagnostics()[0].message.find("incompatible type for argument 3 of 'transform'") != std::string::npos);
    }
}

// -----------------------------------------------------------------------------
// 13. Nested Function Calls
// -----------------------------------------------------------------------------
TEST_CASE("CallCheck: 13 - Nested function calls", "[semantic][call_check][nested]") {
    SymbolTable table;
    table.insert(Symbol::createFunction(
        "add",
        Type::getInt(),
        {Type::getInt(), Type::getInt()},
        false,
        ParameterListKind::Typed
    ));

    CallChecker checker(table);

    SECTION("add(10, add(20, 30)) is valid") {
        Parser p = makeParser("add(10, add(20, 30))");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK(checker.check(*expr));
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
    }

    SECTION("Deeply nested add(add(1, 2), add(3, 4)) is valid") {
        Parser p = makeParser("add(add(1, 2), add(3, 4))");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK(checker.check(*expr));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Inner call with too few arguments add(10, add(20)) reports error") {
        Parser p = makeParser("add(10, add(20))");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "too few arguments to function 'add'");
    }

    SECTION("Inner call to undeclared function add(10, missing(20)) reports error") {
        Parser p = makeParser("add(10, missing(20))");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "use of undeclared identifier 'missing'");
    }
}

// -----------------------------------------------------------------------------
// 14. Diagnostic Message and Source Location
// -----------------------------------------------------------------------------
TEST_CASE("CallCheck: 14 - Diagnostic message and source location", "[semantic][call_check][location]") {
    SymbolTable table;
    table.insert(Symbol::createFunction(
        "compute",
        Type::getInt(),
        {Type::getInt(), Type::getInt()},
        false,
        ParameterListKind::Typed
    ));

    CallChecker checker(table);

    SECTION("Location tracking in CallExpr reflects source position") {
        Parser p = makeParser("compute(1)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        REQUIRE(checker.errorCount() == 1);
        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.identifier == "compute");
        CHECK(diag.location.line == 1);
        CHECK(diag.location.column == 1);
        CHECK(diag.message == "too few arguments to function 'compute'");

        std::string diagStr = diag.toString();
        CHECK(diagStr.find("1:1:") != std::string::npos);
        CHECK(diagStr.find("too few arguments to function 'compute'") != std::string::npos);
    }
}

// -----------------------------------------------------------------------------
// 15. Program and Statement Integration Tests
// -----------------------------------------------------------------------------
TEST_CASE("CallCheck: 15 - Program, CompoundStmt, and VarDecl integration", "[semantic][call_check][integration]") {
    SECTION("Variable declaration initialized with valid call passes") {
        SymbolTable table;
        table.insert(Symbol::createFunction(
            "add",
            Type::getInt(),
            {Type::getInt(), Type::getInt()},
            false,
            ParameterListKind::Typed
        ));

        CallChecker checker(table);
        Parser p = makeParser("int result = add(10, 20);");
        auto varDecl = p.parseVarDecl();
        REQUIRE(varDecl != nullptr);

        CHECK(checker.check(*varDecl));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Variable declaration initialized with invalid call fails") {
        SymbolTable table;
        table.insert(Symbol::createFunction(
            "add",
            Type::getInt(),
            {Type::getInt(), Type::getInt()},
            false,
            ParameterListKind::Typed
        ));

        CallChecker checker(table);
        Parser p = makeParser("int result = add(10);");
        auto varDecl = p.parseVarDecl();
        REQUIRE(varDecl != nullptr);

        CHECK_FALSE(checker.check(*varDecl));
        CHECK(checker.hasErrors());
        CHECK(checker.diagnostics()[0].message == "too few arguments to function 'add'");
    }

    SECTION("Compound block with variable shadowed and called as variable") {
        SymbolTable table;
        table.insert(Symbol::createFunction(
            "foo",
            Type::getInt(),
            {Type::getInt()},
            false,
            ParameterListKind::Typed
        ));

        const std::string blockSrc =
            "{\n"
            "    int foo = 100;\n"
            "    int y = foo(5);\n"
            "}";

        Parser p = makeParser(blockSrc);
        auto block = p.parseCompoundStatement();
        REQUIRE(block != nullptr);

        CallChecker checker(table);
        CHECK_FALSE(checker.check(*block));
        CHECK(checker.hasErrors());
        CHECK(checker.diagnostics()[0].message == "called object 'foo' is not a function");
    }

    SECTION("Convenience checkCalls function") {
        SymbolTable table;
        table.insert(Symbol::createFunction(
            "run",
            Type::getVoid(),
            {},
            false,
            ParameterListKind::Void
        ));

        Parser pGood = makeParser("run()");
        auto diagsGood = checkCalls(*pGood.parseExpression(), table);
        CHECK(diagsGood.empty());

        Parser pBad = makeParser("run(99)");
        auto diagsBad = checkCalls(*pBad.parseExpression(), table);
        REQUIRE(diagsBad.size() == 1);
        CHECK(diagsBad[0].message == "too many arguments to function 'run'");
    }

    SECTION("clear() resets diagnostics and state") {
        CallChecker checker;
        Parser p = makeParser("ghost()");
        checker.check(*p.parseExpression());
        CHECK(checker.hasErrors());

        checker.clear();
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
    }
}

// =============================================================================
// DAY 50 INTEGRATION REVIEW — Diagnostic Ownership & Multi-Checker Verification
// =============================================================================
//
// Tests diagnostic ownership and absence of duplicate diagnostics between:
//   - UndeclaredChecker
//   - CallChecker
//   - TypeChecker::inferCall
//
// Specifically covers:
//   1. Calling an undeclared function: `unknown_fn(10);`
//   2. Calling a declared variable: `int x; x(10);`
//   3. Calling a declared function with incompatible arguments: `int add(int a, int b); add(10, "hello");`
//
TEST_CASE("CallCheck: Integration Review - Diagnostic ownership between checkers", "[semantic][call_check][integration]") {
    SECTION("Case 1: Calling an undeclared function produces a single diagnostic across checkers") {
        // Source: unknown_fn(10);
        Parser p = makeParser("unknown_fn(10)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);
        const auto* call = dynamic_cast<const CallExpr*>(expr.get());
        REQUIRE(call != nullptr);

        SymbolTable table;

        // 1. UndeclaredChecker traverses expression; callee is owned by CallChecker
        UndeclaredChecker undeclaredChecker(table);
        bool undeclaredOk = undeclaredChecker.check(*expr);
        CHECK(undeclaredOk);
        CHECK(undeclaredChecker.errorCount() == 0);

        // 2. CallChecker validates call expression and reports undeclared callee
        CallChecker callChecker(table);
        bool callOk = callChecker.check(*expr);
        CHECK_FALSE(callOk);
        REQUIRE(callChecker.errorCount() == 1);
        CHECK(callChecker.diagnostics()[0].message == "use of undeclared identifier 'unknown_fn'");
        CHECK(callChecker.diagnostics()[0].identifier == "unknown_fn");

        // 3. TypeChecker::inferCall returns nullptr without generating diagnostics
        TypeChecker typeChecker(table);
        auto inferredType = typeChecker.inferCall(*call);
        CHECK(inferredType == nullptr);
        CHECK_FALSE(typeChecker.hasErrors());
        CHECK(typeChecker.errorCount() == 0);

        // Multi-checker combined diagnostic count: exactly 1 diagnostic, zero duplicates
        std::vector<Diagnostic> combined;
        combined.insert(combined.end(), undeclaredChecker.diagnostics().begin(), undeclaredChecker.diagnostics().end());
        combined.insert(combined.end(), callChecker.diagnostics().begin(), callChecker.diagnostics().end());
        combined.insert(combined.end(), typeChecker.diagnostics().begin(), typeChecker.diagnostics().end());

        REQUIRE(combined.size() == 1);
        CHECK(combined[0].message == "use of undeclared identifier 'unknown_fn'");
    }

    SECTION("Case 2: Calling a declared variable produces not-a-function diagnostic without misleading argument errors") {
        // Source: int x; x(10);
        Parser p = makeParser("x(10)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);
        const auto* call = dynamic_cast<const CallExpr*>(expr.get());
        REQUIRE(call != nullptr);

        SymbolTable table;
        table.insert(Symbol::createVariable("x", Type::getInt()));

        // 1. UndeclaredChecker: 'x' is declared in table, 10 is literal -> 0 diagnostics
        UndeclaredChecker undeclaredChecker(table);
        bool undeclaredOk = undeclaredChecker.check(*expr);
        CHECK(undeclaredOk);
        CHECK(undeclaredChecker.errorCount() == 0);

        // 2. CallChecker: 'x' is declared but not a function -> reports called object is not a function
        //    Must not report misleading argument-count or argument-type errors!
        CallChecker callChecker(table);
        bool callOk = callChecker.check(*expr);
        CHECK_FALSE(callOk);
        REQUIRE(callChecker.errorCount() == 1);
        CHECK(callChecker.diagnostics()[0].message == "called object 'x' is not a function");
        CHECK(callChecker.diagnostics()[0].identifier == "x");

        // 3. TypeChecker::inferCall: 'x' is not a function -> returns nullptr without reporting errors
        TypeChecker typeChecker(table);
        auto inferredType = typeChecker.inferCall(*call);
        CHECK(inferredType == nullptr);
        CHECK_FALSE(typeChecker.hasErrors());
        CHECK(typeChecker.errorCount() == 0);

        // Multi-checker combined diagnostic count: exactly 1 diagnostic
        std::vector<Diagnostic> combined;
        combined.insert(combined.end(), undeclaredChecker.diagnostics().begin(), undeclaredChecker.diagnostics().end());
        combined.insert(combined.end(), callChecker.diagnostics().begin(), callChecker.diagnostics().end());
        combined.insert(combined.end(), typeChecker.diagnostics().begin(), typeChecker.diagnostics().end());

        REQUIRE(combined.size() == 1);
        CHECK(combined[0].message == "called object 'x' is not a function");
    }

    SECTION("Case 3: Calling a declared function with incompatible arguments") {
        // Source: int add(int a, int b); add(10, \"hello\");
        Parser p = makeParser("add(10, \"hello\")");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);
        const auto* call = dynamic_cast<const CallExpr*>(expr.get());
        REQUIRE(call != nullptr);

        SymbolTable table;
        table.insert(Symbol::createFunction(
            "add",
            Type::getInt(),
            {Type::getInt(), Type::getInt()},
            false,
            ParameterListKind::Typed
        ));

        // 1. UndeclaredChecker: 'add' is declared, literals contain no identifiers -> 0 diagnostics
        UndeclaredChecker undeclaredChecker(table);
        bool undeclaredOk = undeclaredChecker.check(*expr);
        CHECK(undeclaredOk);
        CHECK(undeclaredChecker.errorCount() == 0);

        // 2. CallChecker: argument 2 incompatible -> reports conversion error
        CallChecker callChecker(table);
        bool callOk = callChecker.check(*expr);
        CHECK_FALSE(callOk);
        REQUIRE(callChecker.errorCount() == 1);
        CHECK(callChecker.diagnostics()[0].message.find("incompatible type for argument 2 of 'add'") != std::string::npos);
        CHECK(callChecker.diagnostics()[0].message.find("cannot convert string literal to 'int'") != std::string::npos);

        // 3. TypeChecker::inferCall: returns int return type without duplicating argument type errors
        TypeChecker typeChecker(table);
        auto inferredType = typeChecker.inferCall(*call);
        REQUIRE(inferredType != nullptr);
        CHECK(inferredType->kind() == TypeKind::Int);
        CHECK_FALSE(typeChecker.hasErrors());
        CHECK(typeChecker.errorCount() == 0);

        // Multi-checker combined diagnostic count: exactly 1 diagnostic, no duplication
        std::vector<Diagnostic> combined;
        combined.insert(combined.end(), undeclaredChecker.diagnostics().begin(), undeclaredChecker.diagnostics().end());
        combined.insert(combined.end(), callChecker.diagnostics().begin(), callChecker.diagnostics().end());
        combined.insert(combined.end(), typeChecker.diagnostics().begin(), typeChecker.diagnostics().end());

        REQUIRE(combined.size() == 1);
        CHECK(combined[0].message.find("incompatible type for argument 2 of 'add'") != std::string::npos);
    }

    SECTION("Case 4: Function call with undeclared argument and valid function") {
        // Source: int add(int a, int b); add(10, unknown_arg);
        Parser p = makeParser("add(10, unknown_arg)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        SymbolTable table;
        table.insert(Symbol::createFunction(
            "add",
            Type::getInt(),
            {Type::getInt(), Type::getInt()},
            false,
            ParameterListKind::Typed
        ));

        // 1. UndeclaredChecker identifies unknown_arg as undeclared
        UndeclaredChecker undeclaredChecker(table);
        bool undeclaredOk = undeclaredChecker.check(*expr);
        CHECK_FALSE(undeclaredOk);
        REQUIRE(undeclaredChecker.errorCount() == 1);
        CHECK(undeclaredChecker.diagnostics()[0].identifier == "unknown_arg");

        // 2. CallChecker suppresses cascading argument type error for unresolvable argument
        CallChecker callChecker(table);
        callChecker.check(*expr);
        // Argument type inference for unknown_arg yields nullptr, CallChecker avoids cascading errors
        CHECK(callChecker.errorCount() == 0);

        // Combined diagnostics: exactly 1 for unknown_arg
        std::vector<Diagnostic> combined;
        combined.insert(combined.end(), undeclaredChecker.diagnostics().begin(), undeclaredChecker.diagnostics().end());
        combined.insert(combined.end(), callChecker.diagnostics().begin(), callChecker.diagnostics().end());

        REQUIRE(combined.size() == 1);
        CHECK(combined[0].identifier == "unknown_arg");
        CHECK(combined[0].message.find("use of undeclared identifier 'unknown_arg'") != std::string::npos);
    }
}
