#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/assign_expr.h"
#include "ast/call_expr.h"
#include "ast/compound_stmt.h"
#include "ast/function.h"
#include "ast/identifier_expression.h"
#include "ast/literal_expression.h"
#include "ast/program.h"
#include "ast/return_stmt.h"
#include "ast/var_decl.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/diagnostics.h"
#include "semantic/symbol_table.h"
#include "semantic/checks/redeclaration.h"
#include "semantic/checks/undeclared.h"
#include "semantic/type_checker.h"
#include "semantic/checks/call_check.h"
#include "semantic/checks/return_check.h"
#include "types/type.h"

#include <memory>
#include <string>
#include <vector>

// =============================================================================
// DAY 52 TEST SUITE: Shared Semantic Diagnostics
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Standard Error Formatting
// -----------------------------------------------------------------------------
TEST_CASE("Standard error formatting: file:line:column: severity: message", "[diagnostics][formatting]") {
    SourceLocation loc("main.c", 5, 5);
    Diagnostic diag("counter", "undeclared identifier 'counter'", loc, {}, false, DiagnosticSeverity::Error);

    CHECK(diag.isError());
    CHECK_FALSE(diag.isWarning());
    CHECK_FALSE(diag.isNote());
    CHECK(diag.getSeverity() == DiagnosticSeverity::Error);

    // Standard format: file:line:column: error: message
    CHECK(diag.toString() == "main.c:5:5: error: undeclared identifier 'counter'");
    CHECK(diag.format() == "main.c:5:5: error: undeclared identifier 'counter'");
}

// -----------------------------------------------------------------------------
// 2. Warning Formatting
// -----------------------------------------------------------------------------
TEST_CASE("Warning formatting: file:line:column: warning: message", "[diagnostics][warning]") {
    SourceLocation loc("utils.c", 12, 8);
    Diagnostic diag("unused_var", "unused variable 'unused_var'", loc, {}, false, DiagnosticSeverity::Warning);

    CHECK(diag.isWarning());
    CHECK_FALSE(diag.isError());
    CHECK_FALSE(diag.isNote());
    CHECK(diag.getSeverity() == DiagnosticSeverity::Warning);

    CHECK(diag.toString() == "utils.c:12:8: warning: unused variable 'unused_var'");
    CHECK(diag.format() == "utils.c:12:8: warning: unused variable 'unused_var'");
}

// -----------------------------------------------------------------------------
// 3. Correct Filename, Line, and Column
// -----------------------------------------------------------------------------
TEST_CASE("Correct filename, line, and column preservation", "[diagnostics][location]") {
    SourceLocation loc("src/core/eval.c", 42, 17);
    Diagnostic diag("x", "type mismatch", loc);

    CHECK(diag.filename() == "src/core/eval.c");
    CHECK(diag.line() == 42);
    CHECK(diag.column() == 17);
    CHECK(diag.getLocation().filename == "src/core/eval.c");
    CHECK(diag.getLocation().line == 42);
    CHECK(diag.getLocation().column == 17);
    CHECK(diag.getLocation().hasFilename());
    CHECK(diag.getLocation().hasLocation());

    CHECK(diag.toString() == "src/core/eval.c:42:17: error: type mismatch");
}

// -----------------------------------------------------------------------------
// 4. Missing or Unavailable Source Filename
// -----------------------------------------------------------------------------
TEST_CASE("Missing or unavailable source filename fallback", "[diagnostics][fallback]") {
    // When no filename is provided, format is line:column: severity: message
    SourceLocation loc("", 15, 3);
    Diagnostic diag("y", "incompatible operands", loc);

    CHECK_FALSE(diag.getLocation().hasFilename());
    CHECK(diag.getLocation().hasLocation());
    CHECK(diag.toString() == "15:3: error: incompatible operands");
}

// -----------------------------------------------------------------------------
// 5. Missing or Unavailable Source Location
// -----------------------------------------------------------------------------
TEST_CASE("Missing or unavailable source location fallbacks", "[diagnostics][fallback]") {
    SECTION("Filename present, line and column unavailable (line == 0)") {
        SourceLocation loc("config.c", 0, 0);
        Diagnostic diag("", "fatal compilation error", loc);

        CHECK(diag.getLocation().hasFilename());
        CHECK_FALSE(diag.getLocation().hasLocation());
        CHECK(diag.toString() == "config.c: error: fatal compilation error");
    }

    SECTION("Neither filename nor line/column available") {
        SourceLocation loc("", 0, 0);
        Diagnostic diag("val", "use of undeclared identifier 'val'", loc);

        CHECK_FALSE(diag.getLocation().hasFilename());
        CHECK_FALSE(diag.getLocation().hasLocation());
        CHECK(diag.toString() == "error: use of undeclared identifier 'val'");
    }

    SECTION("Default constructed Diagnostic") {
        Diagnostic diag;
        diag.message = "generic error";
        diag.severity = DiagnosticSeverity::Error;
        CHECK(diag.toString() == "error: generic error");
    }
}

// -----------------------------------------------------------------------------
// 6. Undeclared Identifier Diagnostic with Filename
// -----------------------------------------------------------------------------
TEST_CASE("Undeclared identifier diagnostic formatting via UndeclaredChecker", "[diagnostics][undeclared]") {
    Lexer lexer("counter = 10;");
    Parser parser(lexer, "main.c");
    auto expr = parser.parseAssignment();
    REQUIRE(expr != nullptr);

    UndeclaredChecker checker;
    bool ok = checker.check(*expr);

    CHECK_FALSE(ok);
    CHECK(checker.hasErrors());
    REQUIRE(checker.errorCount() == 1);

    const auto& diag = checker.diagnostics()[0];
    CHECK(diag.filename() == "main.c");
    CHECK(diag.line() == 1);
    CHECK(diag.column() == 1);
    CHECK(diag.getIdentifier() == "counter");
    CHECK(diag.isError());

    CHECK(diag.toString() == "main.c:1:1: error: use of undeclared identifier 'counter'");
}

// -----------------------------------------------------------------------------
// 7. Duplicate Declaration Diagnostic with Previous Declaration
// -----------------------------------------------------------------------------
TEST_CASE("Duplicate declaration diagnostic formatting with previous declaration", "[diagnostics][redeclaration]") {
    auto program = std::make_unique<Program>();
    SourceLocation loc1("calc.c", 1, 1);
    SourceLocation loc2("calc.c", 2, 1);

    program->addStatement(std::make_unique<VarDecl>(Type::getInt(), "total", nullptr, loc1));
    program->addStatement(std::make_unique<VarDecl>(Type::getInt(), "total", nullptr, loc2));

    RedeclarationChecker checker;
    bool ok = checker.check(*program);

    CHECK_FALSE(ok);
    CHECK(checker.hasErrors());
    REQUIRE(checker.errorCount() == 1);

    const auto& diag = checker.diagnostics()[0];
    CHECK(diag.filename() == "calc.c");
    CHECK(diag.line() == 2);
    CHECK(diag.getIdentifier() == "total");
    CHECK(diag.hasPreviousLocation);
    CHECK(diag.previousLocation.filename == "calc.c");
    CHECK(diag.previousLocation.line == 1);

    std::string formatted = diag.toString();
    CHECK(formatted == "calc.c:2:1: error: duplicate declaration of 'total' (previously declared at calc.c:1:1)");
}

// -----------------------------------------------------------------------------
// 8. Incompatible Type Diagnostic
// -----------------------------------------------------------------------------
TEST_CASE("Incompatible type diagnostic formatting via TypeChecker", "[diagnostics][type_checker]") {
    SymbolTable table;
    table.insert(Symbol::createVariable("void_val", Type::getVoid()));

    TypeChecker checker(table);
    Lexer lexer("int result = void_val;");
    Parser parser(lexer, "types.c");
    auto decl = parser.parseVarDecl();
    REQUIRE(decl != nullptr);

    bool ok = checker.check(*decl);

    CHECK_FALSE(ok);
    CHECK(checker.hasErrors());
    REQUIRE(checker.errorCount() >= 1);

    const auto& diag = checker.diagnostics()[0];
    CHECK(diag.filename() == "types.c");
    CHECK(diag.line() == 1);
    CHECK(diag.isError());

    std::string formatted = diag.toString();
    CHECK(formatted.rfind("types.c:1:1: error: incompatible types in initialization", 0) == 0);
}

// -----------------------------------------------------------------------------
// 9. Invalid Function-Call Diagnostic
// -----------------------------------------------------------------------------
TEST_CASE("Invalid function-call diagnostic formatting via CallChecker", "[diagnostics][call_check]") {
    SECTION("Too few arguments to function") {
        SymbolTable table;
        auto fn = std::make_unique<Function>("multiply", Type::getInt(), SourceLocation("calls.c", 1, 1), false);
        fn->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "a", nullptr, SourceLocation("calls.c", 1, 10)));
        fn->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "b", nullptr, SourceLocation("calls.c", 1, 17)));
        table.insert(*fn);

        Lexer lexer("multiply(42)");
        Parser parser(lexer, "calls.c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        CallChecker checker(table);
        bool ok = checker.check(*expr);

        CHECK_FALSE(ok);
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() >= 1);

        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.filename() == "calls.c");
        CHECK(diag.line() == 1);
        CHECK(diag.isError());

        std::string formatted = diag.toString();
        CHECK(formatted == "calls.c:1:1: error: too few arguments to function 'multiply'");
    }

    SECTION("Calling variable as a function") {
        SymbolTable table;
        auto var = std::make_unique<VarDecl>(Type::getInt(), "not_a_func", nullptr, SourceLocation("bad_call.c", 1, 1));
        table.insert(*var);

        Lexer lexer("not_a_func(5)");
        Parser parser(lexer, "bad_call.c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        CallChecker checker(table);
        bool ok = checker.check(*expr);

        CHECK_FALSE(ok);
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() >= 1);

        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.filename() == "bad_call.c");
        CHECK(diag.line() == 1);

        std::string formatted = diag.toString();
        CHECK(formatted == "bad_call.c:1:1: error: called object 'not_a_func' is not a function");
    }
}

// -----------------------------------------------------------------------------
// 10. Invalid Return Diagnostic
// -----------------------------------------------------------------------------
TEST_CASE("Invalid return diagnostic formatting via ReturnChecker", "[diagnostics][return_check]") {
    SECTION("Void function returning a value") {
        Function fn("process", Type::getVoid(), SourceLocation("returns.c", 1, 1));

        Lexer lexer("return 100;");
        Parser parser(lexer, "returns.c");
        auto ret = parser.parseReturnStatement();
        REQUIRE(ret != nullptr);

        ReturnChecker checker;
        bool ok = checker.checkReturn(*ret, fn);

        CHECK_FALSE(ok);
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() >= 1);

        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.filename() == "returns.c");
        CHECK(diag.line() == 1);
        CHECK(diag.isError());

        std::string formatted = diag.toString();
        CHECK(formatted == "returns.c:1:1: error: void function 'process' should not return a value");
    }

    SECTION("Non-void function returning empty value") {
        Function fn("calculate", Type::getInt(), SourceLocation("empty_ret.c", 1, 1));

        Lexer lexer("return;");
        Parser parser(lexer, "empty_ret.c");
        auto ret = parser.parseReturnStatement();
        REQUIRE(ret != nullptr);

        ReturnChecker checker;
        bool ok = checker.checkReturn(*ret, fn);

        CHECK_FALSE(ok);
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() >= 1);

        const auto& diag = checker.diagnostics()[0];
        CHECK(diag.filename() == "empty_ret.c");
        CHECK(diag.line() == 1);

        std::string formatted = diag.toString();
        CHECK(formatted == "empty_ret.c:1:1: error: non-void function 'calculate' should return a value");
    }
}

// -----------------------------------------------------------------------------
// 11. Previous Declaration Location Preservation
// -----------------------------------------------------------------------------
TEST_CASE("Previous declaration location preservation in Diagnostic", "[diagnostics][prev_loc]") {
    SourceLocation loc("header.h", 20, 5);
    SourceLocation prevLoc("header.h", 5, 5);
    Diagnostic diag("foo", "duplicate declaration of 'foo'", loc, prevLoc, true, DiagnosticSeverity::Error);

    CHECK(diag.hasPreviousLocation);
    CHECK(diag.getPreviousLocation().filename == "header.h");
    CHECK(diag.getPreviousLocation().line == 5);
    CHECK(diag.getPreviousLocation().column == 5);

    CHECK(diag.toString() == "header.h:20:5: error: duplicate declaration of 'foo' (previously declared at header.h:5:5)");
}

// -----------------------------------------------------------------------------
// 12. Multiple Diagnostics from One Source File
// -----------------------------------------------------------------------------
TEST_CASE("Multiple diagnostics preserve consistent filename and distinct locations", "[diagnostics][multiple]") {
    auto id1 = std::make_unique<IdentifierExpression>("a", SourceLocation("multi_err.c", 2, 5));
    auto id2 = std::make_unique<IdentifierExpression>("b", SourceLocation("multi_err.c", 3, 5));
    auto id3 = std::make_unique<IdentifierExpression>("c", SourceLocation("multi_err.c", 4, 5));

    UndeclaredChecker checker;
    checker.check(*id1);
    checker.check(*id2);
    checker.check(*id3);

    REQUIRE(checker.errorCount() == 3);

    for (const auto& diag : checker.diagnostics()) {
        CHECK(diag.filename() == "multi_err.c");
        CHECK(diag.isError());
        CHECK(diag.toString().rfind("multi_err.c:", 0) == 0);
        CHECK(diag.toString().find(": error: use of undeclared identifier '") != std::string::npos);
    }

    CHECK(checker.diagnostics()[0].line() == 2);
    CHECK(checker.diagnostics()[0].getIdentifier() == "a");

    CHECK(checker.diagnostics()[1].line() == 3);
    CHECK(checker.diagnostics()[1].getIdentifier() == "b");

    CHECK(checker.diagnostics()[2].line() == 4);
    CHECK(checker.diagnostics()[2].getIdentifier() == "c");
}

// -----------------------------------------------------------------------------
// 13. Consistent Formatting Across All Five Semantic Checkers
// -----------------------------------------------------------------------------
TEST_CASE("Consistent formatting across all five Phase 6 semantic checkers", "[diagnostics][consistency]") {
    // Checker 1: Redeclaration
    Diagnostic d1("v", "duplicate declaration of 'v'", SourceLocation("test.c", 2, 1), SourceLocation("test.c", 1, 1), true);
    // Checker 2: Undeclared
    Diagnostic d2("u", "use of undeclared identifier 'u'", SourceLocation("test.c", 4, 5));
    // Checker 3: TypeChecker
    Diagnostic d3("", "incompatible types in initialization", SourceLocation("test.c", 6, 9));
    // Checker 4: CallChecker
    Diagnostic d4("fn", "too few arguments to function 'fn'", SourceLocation("test.c", 8, 5));
    // Checker 5: ReturnChecker
    Diagnostic d5("f", "non-void function 'f' should return a value", SourceLocation("test.c", 10, 5));

    std::vector<Diagnostic> allDiagnostics = {d1, d2, d3, d4, d5};

    for (const auto& diag : allDiagnostics) {
        std::string str = diag.toString();
        // Each formatted string begins with "test.c:<line>:<col>: error: "
        CHECK(str.rfind("test.c:", 0) == 0);
        CHECK(str.find(": error: ") != std::string::npos);
        CHECK(diag.isError());
        CHECK(diag.format() == str);
    }
}

// -----------------------------------------------------------------------------
// 14. Warning Support and Queries Across Checkers
// -----------------------------------------------------------------------------
TEST_CASE("Warning reporting and query APIs across semantic checkers", "[diagnostics][warning_api]") {
    SECTION("RedeclarationChecker warning support") {
        RedeclarationChecker checker;
        checker.reportWarning("shadows a global declaration", SourceLocation("warn.c", 4, 9), "x");
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.hasWarnings());
        CHECK(checker.warningCount() == 1);
        CHECK(checker.diagnostics()[0].toString() == "warn.c:4:9: warning: shadows a global declaration");
    }

    SECTION("UndeclaredChecker warning support") {
        UndeclaredChecker checker;
        checker.reportWarning("implicit function declaration", SourceLocation("warn.c", 10, 5), "printf");
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.hasWarnings());
        CHECK(checker.warningCount() == 1);
        CHECK(checker.diagnostics()[0].toString() == "warn.c:10:5: warning: implicit function declaration");
    }

    SECTION("TypeChecker warning support") {
        TypeChecker checker;
        checker.reportWarning("implicit conversion loses integer precision", SourceLocation("warn.c", 15, 13));
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.hasWarnings());
        CHECK(checker.warningCount() == 1);
        CHECK(checker.diagnostics()[0].toString() == "warn.c:15:13: warning: implicit conversion loses integer precision");
    }

    SECTION("CallChecker warning support") {
        CallChecker checker;
        checker.reportWarning("passing argument without prototype", SourceLocation("warn.c", 20, 5), "old_fn");
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.hasWarnings());
        CHECK(checker.warningCount() == 1);
        CHECK(checker.diagnostics()[0].toString() == "warn.c:20:5: warning: passing argument without prototype");
    }

    SECTION("ReturnChecker warning support") {
        ReturnChecker checker;
        checker.reportWarning("control reaches end of non-void function", SourceLocation("warn.c", 25, 1), "main");
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.hasWarnings());
        CHECK(checker.warningCount() == 1);
        CHECK(checker.diagnostics()[0].toString() == "warn.c:25:1: warning: control reaches end of non-void function");
    }
}

// -----------------------------------------------------------------------------
// 15. Backward Compatibility with Existing Accessors and String Methods
// -----------------------------------------------------------------------------
TEST_CASE("Backward compatibility with Day 47–51 diagnostic expectations", "[diagnostics][compatibility]") {
    // Constructor matching Day 47
    Diagnostic diag("old_id", "some message", SourceLocation{3, 4}, SourceLocation{1, 2}, true);
    CHECK(diag.identifier == "old_id");
    CHECK(diag.message == "some message");
    CHECK(diag.location.line == 3);
    CHECK(diag.location.column == 4);
    CHECK(diag.previousLocation.line == 1);
    CHECK(diag.previousLocation.column == 2);
    CHECK(diag.hasPreviousLocation);

    CHECK(diag.getIdentifier() == "old_id");
    CHECK(diag.getMessage() == "some message");
    CHECK(diag.getLocation().line == 3);
    CHECK(diag.getLocation().column == 4);
    CHECK(diag.getPreviousLocation().line == 1);
    CHECK(diag.getPreviousLocation().column == 2);

    // Without filename, formatting preserves legacy "line:col: error: msg (previously declared at line:col)"
    CHECK(diag.toString() == "3:4: error: some message (previously declared at 1:2)");
}
