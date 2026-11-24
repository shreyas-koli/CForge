#include <catch2/catch.hpp>

#include "ast/assign_expr.h"
#include "ast/ast.h"
#include "ast/binary_expr.h"
#include "ast/call_expr.h"
#include "ast/compound_stmt.h"
#include "ast/conditional_expr.h"
#include "ast/function.h"
#include "ast/identifier_expression.h"
#include "ast/if_stmt.h"
#include "ast/literal_expression.h"
#include "ast/program.h"
#include "ast/return_stmt.h"
#include "ast/unary_expr.h"
#include "ast/var_decl.h"
#include "ast/variable_declaration.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/checks/call_check.h"
#include "semantic/checks/redeclaration.h"
#include "semantic/checks/return_check.h"
#include "semantic/checks/undeclared.h"
#include "semantic/diagnostics.h"
#include "semantic/symbol_table.h"
#include "semantic/type_checker.h"
#include "types/conversions.h"
#include "types/type.h"

#include <memory>
#include <string>
#include <vector>

// Helper to construct a Parser from source code with an optional filename
static Parser makeParser(const std::string& src, const std::string& filename = "") {
    Lexer lexer(src);
    return Parser(lexer.tokenize(), filename);
}

// =============================================================================
// DAY 53 REGRESSION TEST SUITE — PHASE 6 SEMANTIC ANALYSIS
// =============================================================================

// =============================================================================
// Area A: Symbol Table Regression
// =============================================================================

TEST_CASE("Regression: Symbol Table - Insertion, Lookup, and Kind Verification", "[semantic][regression][symbol_table]") {
    SymbolTable table;

    SECTION("Global variable insertion and lookup") {
        auto intType = Type::getInt();
        SourceLocation loc("global.c", 10, 5);
        Symbol varSym("g_counter", SymbolKind::Variable, intType, loc, StorageClass::Static);

        CHECK(table.insert(varSym));
        CHECK(table.contains("g_counter"));
        CHECK(table.containsCurrentScope("g_counter"));

        const Symbol* found = table.lookup("g_counter");
        REQUIRE(found != nullptr);
        CHECK(found->name() == "g_counter");
        CHECK(found->isVariable());
        CHECK_FALSE(found->isFunction());
        CHECK(found->isStatic());
        CHECK(found->type()->kind() == TypeKind::Int);
        CHECK(found->location().line == 10);
        CHECK(found->location().column == 5);
        CHECK(found->location().filename == "global.c");
        CHECK(found->scopeLevel() == 0);
        CHECK(found->isGlobal());
    }

    SECTION("Function symbol insertion, prototype, and definition lookup") {
        auto floatType = Type::getFloat();
        SourceLocation loc("math.c", 5, 1);
        Symbol fnSym = Symbol::createFunction("compute", floatType, {Type::getInt(), Type::getInt()}, false);
        fnSym.setLocation(loc);

        CHECK(table.insert(fnSym));
        CHECK(table.contains("compute"));

        const Symbol* foundProto = table.lookup("compute");
        REQUIRE(foundProto != nullptr);
        CHECK(foundProto->isFunction());
        CHECK(foundProto->isPrototype());
        CHECK_FALSE(foundProto->isDefinition());
        CHECK(foundProto->parameterCount() == 2);
        CHECK(foundProto->parameter(0)->kind() == TypeKind::Int);
        CHECK(foundProto->parameter(1)->kind() == TypeKind::Int);
        CHECK(foundProto->returnTypeKind() == TypeKind::Float);

        // Update prototype to definition via insertOrAssign
        Symbol fnDef = Symbol::createFunction("compute", floatType, {Type::getInt(), Type::getInt()}, true);
        fnDef.setLocation(SourceLocation("math.c", 20, 1));
        table.insertOrAssign(fnDef);

        const Symbol* foundDef = table.lookup("compute");
        REQUIRE(foundDef != nullptr);
        CHECK(foundDef->isDefinition());
        CHECK(foundDef->location().line == 20);
    }

    SECTION("Duplicate insertion rejection in same scope") {
        auto var1 = Symbol::createVariable("val", Type::getInt());
        auto var2 = Symbol::createVariable("val", Type::getFloat());

        CHECK(table.insert(var1));
        CHECK_FALSE(table.insert(var2)); // Rejection in same scope
        CHECK(table.lookup("val")->type()->kind() == TypeKind::Int);
    }
}

TEST_CASE("Regression: Symbol Table - Metadata, Removal, and Clear Operations", "[semantic][regression][symbol_table]") {
    SymbolTable table;

    SECTION("Symbol metadata preservation") {
        SourceLocation loc("meta.c", 42, 12);
        Symbol sym("buffer", SymbolKind::Variable, Type::getInt(true, false), loc, StorageClass::Extern);

        table.insert(sym);
        const Symbol* retrieved = table.lookup("buffer");
        REQUIRE(retrieved != nullptr);

        CHECK(retrieved->getName() == "buffer");
        CHECK(retrieved->isConst());
        CHECK(retrieved->isExtern());
        CHECK(retrieved->location() == loc);
        CHECK(retrieved->location().filename == "meta.c");
    }

    SECTION("Removal and clearing resets state to clean global scope") {
        table.insert(Symbol::createVariable("x", Type::getInt()));
        table.insert(Symbol::createVariable("y", Type::getFloat()));
        CHECK(table.size() == 2);
        CHECK_FALSE(table.empty());

        CHECK(table.remove("x"));
        CHECK_FALSE(table.contains("x"));
        CHECK(table.size() == 1);
        CHECK(table.contains("y"));

        table.pushScope();
        table.insert(Symbol::createVariable("z", Type::getChar()));
        CHECK(table.scopeDepth() == 2);

        table.clear();
        CHECK(table.empty());
        CHECK(table.size() == 0);
        CHECK(table.scopeDepth() == 1);
        CHECK(table.currentScopeLevel() == 0);
        CHECK(table.isGlobalScope());
        CHECK_FALSE(table.contains("y"));
        CHECK_FALSE(table.contains("z"));
    }
}

// =============================================================================
// Area B: Scope Management Regression
// =============================================================================

TEST_CASE("Regression: Scope Management - Nested Scopes, Shadowing, and Lifetime", "[semantic][regression][scope]") {
    SymbolTable table;

    SECTION("Global scope visibility across nested levels") {
        table.insert(Symbol::createVariable("global_var", Type::getInt()));

        table.pushScope(); // Level 1
        table.pushScope(); // Level 2
        CHECK(table.currentScopeLevel() == 2);

        const Symbol* sym = table.lookup("global_var");
        REQUIRE(sym != nullptr);
        CHECK(sym->scopeLevel() == 0);
        CHECK(sym->isGlobal());

        table.popScope();
        table.popScope();
    }

    SECTION("Innermost shadowing resolves first and outer becomes visible after pop") {
        table.insert(Symbol::createVariable("item", Type::getInt())); // Level 0 (int)

        table.pushScope(); // Level 1
        table.insert(Symbol::createVariable("item", Type::getFloat())); // Level 1 (float)

        table.pushScope(); // Level 2
        table.insert(Symbol::createVariable("item", Type::getChar())); // Level 2 (char)

        // Lookup resolves innermost (Level 2 char)
        const Symbol* innerSym = table.lookup("item");
        REQUIRE(innerSym != nullptr);
        CHECK(innerSym->type()->kind() == TypeKind::Char);
        CHECK(innerSym->scopeLevel() == 2);

        // Pop Level 2 -> Level 1 (float) becomes active
        CHECK(table.popScope());
        const Symbol* midSym = table.lookup("item");
        REQUIRE(midSym != nullptr);
        CHECK(midSym->type()->kind() == TypeKind::Float);
        CHECK(midSym->scopeLevel() == 1);

        // Pop Level 1 -> Level 0 (int) becomes active
        CHECK(table.popScope());
        const Symbol* outerSym = table.lookup("item");
        REQUIRE(outerSym != nullptr);
        CHECK(outerSym->type()->kind() == TypeKind::Int);
        CHECK(outerSym->scopeLevel() == 0);
    }

    SECTION("Inner-only declarations become inaccessible after scope exit") {
        table.pushScope();
        table.insert(Symbol::createVariable("inner_only", Type::getDouble()));
        CHECK(table.contains("inner_only"));

        CHECK(table.popScope());
        CHECK_FALSE(table.contains("inner_only"));
        CHECK(table.lookup("inner_only") == nullptr);
    }

    SECTION("Same identifier accepted in sibling scopes") {
        // Sibling 1
        table.pushScope();
        CHECK(table.insert(Symbol::createVariable("sibling_var", Type::getInt())));
        CHECK(table.popScope());

        // Sibling 2
        table.pushScope();
        CHECK(table.insert(Symbol::createVariable("sibling_var", Type::getFloat())));
        CHECK(table.popScope());
    }

    SECTION("Global scope cannot accidentally be popped") {
        CHECK(table.scopeDepth() == 1);
        CHECK_FALSE(table.popScope()); // Must return false and stay at depth 1
        CHECK(table.scopeDepth() == 1);
        CHECK(table.isGlobalScope());
    }
}

// =============================================================================
// Area C: Redeclaration Detection Regression
// =============================================================================

TEST_CASE("Regression: Redeclaration Detection - Scope Boundaries and Functions", "[semantic][regression][redeclaration]") {
    SECTION("Duplicate global declarations are diagnosed") {
        auto prog = std::make_unique<Program>(SourceLocation("dup_global.c", 1, 1));
        prog->addStatement(std::make_unique<VarDecl>(Type::getInt(), "count", nullptr, SourceLocation("dup_global.c", 1, 1)));
        prog->addStatement(std::make_unique<VarDecl>(Type::getInt(), "count", nullptr, SourceLocation("dup_global.c", 2, 1)));

        RedeclarationChecker checker;
        CHECK_FALSE(checker.check(*prog));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message.find("duplicate declaration of 'count'") != std::string::npos);
        CHECK(checker.diagnostics()[0].hasPreviousLocation);
        CHECK(checker.diagnostics()[0].previousLocation.line == 1);
    }

    SECTION("Duplicate local declarations inside same block are diagnosed") {
        auto block = std::make_unique<CompoundStmt>(SourceLocation("dup_local.c", 1, 1));
        block->addStatement(std::make_unique<VarDecl>(Type::getInt(), "temp", nullptr, SourceLocation("dup_local.c", 2, 5)));
        block->addStatement(std::make_unique<VarDecl>(Type::getInt(), "temp", nullptr, SourceLocation("dup_local.c", 3, 5)));

        RedeclarationChecker checker;
        CHECK_FALSE(checker.check(*block));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "temp");
    }

    SECTION("Valid shadowing of global variables by local variables") {
        auto prog = std::make_unique<Program>(SourceLocation("shadow.c", 1, 1));
        prog->addStatement(std::make_unique<VarDecl>(Type::getInt(), "val", nullptr, SourceLocation("shadow.c", 1, 1)));

        auto fn = std::make_unique<Function>("test", Type::getVoid(), SourceLocation("shadow.c", 2, 1), true);
        auto body = std::make_unique<CompoundStmt>(SourceLocation("shadow.c", 2, 13));
        body->addStatement(std::make_unique<VarDecl>(Type::getFloat(), "val", nullptr, SourceLocation("shadow.c", 3, 5)));
        fn->addBodyStatement(std::move(body));
        prog->addStatement(std::move(fn));

        RedeclarationChecker checker;
        CHECK(checker.check(*prog));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Duplicate function definitions are diagnosed") {
        auto prog = std::make_unique<Program>(SourceLocation("dup_fn.c", 1, 1));
        auto fn1 = std::make_unique<Function>("init", Type::getVoid(), SourceLocation("dup_fn.c", 1, 1), true);
        auto fn2 = std::make_unique<Function>("init", Type::getVoid(), SourceLocation("dup_fn.c", 5, 1), true);

        prog->addStatement(std::move(fn1));
        prog->addStatement(std::move(fn2));

        RedeclarationChecker checker;
        CHECK_FALSE(checker.check(*prog));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message.find("redefinition of function 'init'") != std::string::npos);
    }

    SECTION("Valid function prototype followed by one definition is accepted") {
        auto prog = std::make_unique<Program>(SourceLocation("proto_def.c", 1, 1));
        auto proto = std::make_unique<Function>("setup", Type::getInt(), SourceLocation("proto_def.c", 1, 1), false);
        auto def = std::make_unique<Function>("setup", Type::getInt(), SourceLocation("proto_def.c", 4, 1), true);

        prog->addStatement(std::move(proto));
        prog->addStatement(std::move(def));

        RedeclarationChecker checker;
        CHECK(checker.check(*prog));
        CHECK_FALSE(checker.hasErrors());
    }
}

// =============================================================================
// Area D: Undefined Identifier Detection Regression
// =============================================================================

TEST_CASE("Regression: Undefined Identifier Detection across Expressions and Contexts", "[semantic][regression][undeclared]") {
    SECTION("Declared variable and declared function uses are accepted") {
        SymbolTable table;
        table.insert(Symbol::createVariable("counter", Type::getInt()));
        table.insert(Symbol::createFunction("reset", Type::getVoid(), {}, false));

        UndeclaredChecker checker(table);

        IdentifierExpression idVar("counter", SourceLocation("test.c", 1, 1));
        CHECK(checker.check(idVar));
        CHECK_FALSE(checker.hasErrors());

        IdentifierExpression idFn("reset", SourceLocation("test.c", 2, 1));
        CHECK(checker.check(idFn));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Undeclared identifier in initializer expression is caught") {
        Parser p = makeParser("int a = unk_val;");
        auto decl = p.parseVarDecl();
        REQUIRE(decl != nullptr);

        UndeclaredChecker checker;
        CHECK_FALSE(checker.check(*decl));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "unk_val");
    }

    SECTION("Undeclared identifier in binary operation and condition is caught") {
        SymbolTable table;
        table.insert(Symbol::createVariable("x", Type::getInt()));

        // x + missing
        Parser pBin = makeParser("x + missing");
        auto binExpr = pBin.parseExpression();
        REQUIRE(binExpr != nullptr);

        UndeclaredChecker checker(table);
        CHECK_FALSE(checker.check(*binExpr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "missing");

        // if (missing_cond) ...
        table.clear();
        table.insert(Symbol::createVariable("x", Type::getInt()));
        auto condExpr = std::make_unique<IdentifierExpression>("missing_cond", SourceLocation("if.c", 2, 9));
        auto ifStmt = std::make_unique<IfStmt>(std::move(condExpr), std::make_unique<CompoundStmt>());

        UndeclaredChecker checkerIf(table);
        CHECK_FALSE(checkerIf.check(*ifStmt));
        CHECK(checkerIf.hasErrors());
        REQUIRE(checkerIf.errorCount() == 1);
        CHECK(checkerIf.diagnostics()[0].identifier == "missing_cond");
    }

    SECTION("Undeclared identifier inside function-call argument is caught") {
        SymbolTable table;
        table.insert(Symbol::createFunction("callme", Type::getVoid(), {Type::getInt()}, false));

        Parser p = makeParser("callme(undeclared_arg)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        UndeclaredChecker checker(table);
        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].identifier == "undeclared_arg");
    }
}

// =============================================================================
// Area E: Expression Type Checking Regression
// =============================================================================

TEST_CASE("Regression: Expression Type Checking - Conversions, Operations, and Literals", "[semantic][regression][type_checker]") {
    TypeChecker checker;

    SECTION("Usual arithmetic conversions and promotion rules") {
        // int + float -> float
        auto leftInt = std::make_unique<LiteralExpression>(LiteralKind::Integer, "5");
        auto rightFloat = std::make_unique<LiteralExpression>(LiteralKind::Float, "2.5f");
        BinaryExpr addExpr("+", std::move(leftInt), std::move(rightFloat));

        auto resType = checker.inferType(addExpr);
        REQUIRE(resType != nullptr);
        CHECK(resType->kind() == TypeKind::Float);

        // float + double -> double
        auto leftF = std::make_unique<LiteralExpression>(LiteralKind::Float, "1.5f");
        auto rightD = std::make_unique<LiteralExpression>(LiteralKind::Float, "3.14");
        BinaryExpr mixExpr("+", std::move(leftF), std::move(rightD));

        auto mixType = checker.inferType(mixExpr);
        REQUIRE(mixType != nullptr);
        CHECK(mixType->kind() == TypeKind::Double);
    }

    SECTION("Modulo requires integer operands") {
        // Valid: int % int -> int
        auto i1 = std::make_unique<LiteralExpression>(LiteralKind::Integer, "10");
        auto i2 = std::make_unique<LiteralExpression>(LiteralKind::Integer, "3");
        BinaryExpr validMod("%", std::move(i1), std::move(i2));
        auto modType = checker.inferType(validMod);
        REQUIRE(modType != nullptr);
        CHECK(modType->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());

        // Invalid: float % int -> error
        auto f1 = std::make_unique<LiteralExpression>(LiteralKind::Float, "10.5f");
        auto i3 = std::make_unique<LiteralExpression>(LiteralKind::Integer, "2");
        BinaryExpr invalidMod("%", std::move(f1), std::move(i3));
        CHECK(checker.inferType(invalidMod) == nullptr);
        CHECK(checker.hasErrors());
        CHECK(checker.diagnostics()[0].message.find("invalid operands to binary operator '%'") != std::string::npos);
    }

    SECTION("Bitwise operators require integer operands") {
        TypeChecker tc;
        // Invalid: 2.5f & 1
        auto f = std::make_unique<LiteralExpression>(LiteralKind::Float, "2.5f");
        auto i = std::make_unique<LiteralExpression>(LiteralKind::Integer, "1");
        BinaryExpr bitExpr("&", std::move(f), std::move(i));
        CHECK(tc.inferType(bitExpr) == nullptr);
        CHECK(tc.hasErrors());
    }

    SECTION("Relational and equality operators yield int") {
        TypeChecker tc;
        auto left = std::make_unique<LiteralExpression>(LiteralKind::Integer, "10");
        auto right = std::make_unique<LiteralExpression>(LiteralKind::Integer, "20");
        BinaryExpr eqExpr("==", std::move(left), std::move(right));
        auto eqType = tc.inferType(eqExpr);
        REQUIRE(eqType != nullptr);
        CHECK(eqType->kind() == TypeKind::Int);
    }

    SECTION("Assignment to const-qualified variable is rejected") {
        SymbolTable table;
        table.insert(Symbol::createVariable("MAX", Type::getInt(true, false)));
        TypeChecker tc(table);

        Parser p = makeParser("MAX = 100");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK(tc.inferType(*expr) == nullptr);
        CHECK(tc.hasErrors());
        CHECK(tc.diagnostics()[0].message.find("cannot assign to variable with const-qualified type") != std::string::npos);
    }

    SECTION("Assignment to non-lvalue literal is rejected") {
        TypeChecker tc;
        Parser p = makeParser("10 = 20");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK(tc.inferType(*expr) == nullptr);
        CHECK(tc.hasErrors());
        CHECK(tc.diagnostics()[0].message.find("expression is not assignable") != std::string::npos);
    }

    SECTION("Floating-point literal typing precision") {
        TypeChecker tc;

        // unsuffixed -> double
        LiteralExpression litDouble(LiteralKind::Float, "3.14");
        auto tDouble = tc.inferType(litDouble);
        REQUIRE(tDouble != nullptr);
        CHECK(tDouble->kind() == TypeKind::Double);

        // 'f' suffixed -> float
        LiteralExpression litFloatLower(LiteralKind::Float, "3.14f");
        auto tFloatL = tc.inferType(litFloatLower);
        REQUIRE(tFloatL != nullptr);
        CHECK(tFloatL->kind() == TypeKind::Float);

        // 'F' suffixed -> float
        LiteralExpression litFloatUpper(LiteralKind::Float, "3.14F");
        auto tFloatU = tc.inferType(litFloatUpper);
        REQUIRE(tFloatU != nullptr);
        CHECK(tFloatU->kind() == TypeKind::Float);

        // 'L' suffixed -> unsupported diagnostic
        LiteralExpression litLongDouble(LiteralKind::Float, "3.14L");
        auto tLongD = tc.inferType(litLongDouble);
        CHECK(tLongD == nullptr);
        CHECK(tc.hasErrors());
        CHECK(tc.diagnostics()[0].message.find("long double' is not supported") != std::string::npos);
    }
}

// =============================================================================
// Area F: Function-Call Validation Regression
// =============================================================================

TEST_CASE("Regression: Function-Call Validation - Arguments, Arity, and Types", "[semantic][regression][call_check]") {
    SymbolTable table;
    table.insert(Symbol::createFunction("calc", Type::getInt(), {Type::getInt(), Type::getFloat()}, false));
    table.insert(Symbol::createFunction("greet", Type::getVoid(), {}, false, ParameterListKind::Void));
    table.insert(Symbol::createVariable("not_a_fn", Type::getInt()));

    CallChecker checker(table);

    SECTION("Valid call with implicit numeric conversion is accepted") {
        Parser p = makeParser("calc(10, 20)"); // 20 (int) implicitly convertible to float parameter
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK(checker.check(*expr));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Too few arguments diagnosed") {
        Parser p = makeParser("calc(10)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "too few arguments to function 'calc'");
    }

    SECTION("Too many arguments diagnosed") {
        Parser p = makeParser("calc(10, 2.5f, 30)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "too many arguments to function 'calc'");
    }

    SECTION("Explicit void parameter list accepts 0 arguments and rejects > 0") {
        Parser pValid = makeParser("greet()");
        CHECK(checker.check(*pValid.parseExpression()));

        Parser pInvalid = makeParser("greet(1)");
        CHECK_FALSE(checker.check(*pInvalid.parseExpression()));
        CHECK(checker.hasErrors());
        CHECK(checker.diagnostics()[0].message == "too many arguments to function 'greet'");
    }

    SECTION("Calling undeclared function reports single clean diagnostic") {
        Parser p = makeParser("missing_function(10, 20)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message.find("use of undeclared identifier 'missing_function'") != std::string::npos);
    }

    SECTION("Calling a variable reports called object is not a function without arity noise") {
        Parser p = makeParser("not_a_fn(42)");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK_FALSE(checker.check(*expr));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message == "called object 'not_a_fn' is not a function");
    }

    SECTION("Nested function calls validate correctly") {
        Parser p = makeParser("calc(10, calc(1, 2.0f))");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);

        CHECK(checker.check(*expr));
        CHECK_FALSE(checker.hasErrors());
    }
}

// =============================================================================
// Area G: Return Statement Validation Regression
// =============================================================================

TEST_CASE("Regression: Return Statement Validation - Types and Void vs Non-Void", "[semantic][regression][return_check]") {
    ReturnChecker checker;

    SECTION("Void function returning a value is diagnosed") {
        Function fnVoid("do_work", Type::getVoid(), SourceLocation("ret.c", 1, 1));
        Parser p = makeParser("return 42;");
        auto ret = p.parseReturnStatement();
        REQUIRE(ret != nullptr);

        CHECK_FALSE(checker.checkReturn(*ret, fnVoid));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message.find("void function 'do_work' should not return a value") != std::string::npos);
    }

    SECTION("Non-void function returning empty value is diagnosed") {
        Function fnInt("get_count", Type::getInt(), SourceLocation("ret.c", 1, 1));
        Parser p = makeParser("return;");
        auto ret = p.parseReturnStatement();
        REQUIRE(ret != nullptr);

        CHECK_FALSE(checker.checkReturn(*ret, fnInt));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() == 1);
        CHECK(checker.diagnostics()[0].message.find("non-void function 'get_count' should return a value") != std::string::npos);
    }

    SECTION("Compatible implicit conversion in return value is accepted") {
        Function fnFloat("get_ratio", Type::getFloat(), SourceLocation("ret.c", 1, 1));
        Parser p = makeParser("return 10;"); // int 10 implicitly convertible to float
        auto ret = p.parseReturnStatement();
        REQUIRE(ret != nullptr);

        CHECK(checker.checkReturn(*ret, fnFloat));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Incompatible return type is diagnosed") {
        SymbolTable table;
        table.insert(Symbol::createVariable("void_val", Type::getVoid()));
        ReturnChecker tcChecker(table);

        Function fnInt("get_val", Type::getInt(), SourceLocation("ret.c", 1, 1));
        Parser p = makeParser("return void_val;");
        auto ret = p.parseReturnStatement();
        REQUIRE(ret != nullptr);

        CHECK_FALSE(tcChecker.checkReturn(*ret, fnInt));
        CHECK(tcChecker.hasErrors());
        REQUIRE(tcChecker.errorCount() >= 1);
        CHECK(tcChecker.diagnostics()[0].message.find("incompatible return type") != std::string::npos);
    }

    SECTION("Multiple return statements inside branches validate cleanly") {
        Function fnInt("max", Type::getInt(), SourceLocation("ret.c", 1, 1), true);
        auto body = std::make_unique<CompoundStmt>();

        Parser p1 = makeParser("return 1;");
        auto r1 = p1.parseReturnStatement();
        Parser p2 = makeParser("return 2;");
        auto r2 = p2.parseReturnStatement();

        body->addStatement(std::move(r1));
        body->addStatement(std::move(r2));
        fnInt.addBodyStatement(std::move(body));

        CHECK(checker.check(fnInt));
        CHECK_FALSE(checker.hasErrors());
    }
}

// =============================================================================
// Area H: Shared Semantic Diagnostics Regression
// =============================================================================

TEST_CASE("Regression: Shared Semantic Diagnostics - Format, Severities, and Precision", "[semantic][regression][diagnostics]") {
    SECTION("Standard error and warning formatting") {
        Diagnostic errDiag("val", "invalid expression", SourceLocation("source.c", 14, 8), {}, false, DiagnosticSeverity::Error);
        CHECK(errDiag.toString() == "source.c:14:8: error: invalid expression");
        CHECK(errDiag.isError());

        Diagnostic warnDiag("old_opt", "deprecated feature", SourceLocation("source.c", 22, 1), {}, false, DiagnosticSeverity::Warning);
        CHECK(warnDiag.toString() == "source.c:22:1: warning: deprecated feature");
        CHECK(warnDiag.isWarning());

        Diagnostic noteDiag("", "declared here", SourceLocation("source.c", 5, 1), {}, false, DiagnosticSeverity::Note);
        CHECK(noteDiag.toString() == "source.c:5:1: note: declared here");
        CHECK(noteDiag.isNote());
    }

    SECTION("Previous declaration location is properly formatted") {
        SourceLocation curLoc("app.c", 10, 5);
        SourceLocation prevLoc("app.c", 2, 5);
        Diagnostic dup("total", "duplicate declaration of 'total'", curLoc, prevLoc, true);

        CHECK(dup.hasPreviousLocation);
        CHECK(dup.toString() == "app.c:10:5: error: duplicate declaration of 'total' (previously declared at app.c:2:5)");
    }

    SECTION("Fallbacks for missing filename or location") {
        // Line & col only
        Diagnostic dNoFile("x", "type error", SourceLocation("", 7, 3));
        CHECK(dNoFile.toString() == "7:3: error: type error");

        // Filename only
        Diagnostic dNoLine("", "file failure", SourceLocation("input.c", 0, 0));
        CHECK(dNoLine.toString() == "input.c: error: file failure");

        // Neither
        Diagnostic dBare("", "fatal", SourceLocation("", 0, 0));
        CHECK(dBare.toString() == "error: fatal");
    }
}

// =============================================================================
// Section 3: Important Integration and Edge Cases (Cases 1 to 8)
// =============================================================================

TEST_CASE("Integration Case 1: Nested shadowing", "[semantic][regression][integration][case1]") {
    // int main(void) {
    //     int x = 10;
    //     {
    //         float x = 2.5f;
    //         {
    //             char x = 'A';
    //         }
    //     }
    //     return x;
    // }
    auto mainFn = std::make_unique<Function>("main", Type::getInt(), SourceLocation("main.c", 1, 1), true);
    mainFn->setParameterListKind(ParameterListKind::Void);

    auto outerBlock = std::make_unique<CompoundStmt>(SourceLocation("main.c", 1, 16));

    // int x = 10;
    outerBlock->addStatement(std::make_unique<VarDecl>(
        Type::getInt(), "x",
        std::make_unique<LiteralExpression>(LiteralKind::Integer, "10", SourceLocation("main.c", 2, 13)),
        SourceLocation("main.c", 2, 5)));

    // { float x = 2.5f; { char x = 'A'; } }
    auto midBlock = std::make_unique<CompoundStmt>(SourceLocation("main.c", 3, 5));
    midBlock->addStatement(std::make_unique<VarDecl>(
        Type::getFloat(), "x",
        std::make_unique<LiteralExpression>(LiteralKind::Float, "2.5f", SourceLocation("main.c", 4, 19)),
        SourceLocation("main.c", 4, 9)));

    auto innerBlock = std::make_unique<CompoundStmt>(SourceLocation("main.c", 5, 9));
    innerBlock->addStatement(std::make_unique<VarDecl>(
        Type::getChar(), "x",
        std::make_unique<LiteralExpression>(LiteralKind::Char, "'A'", SourceLocation("main.c", 6, 20)),
        SourceLocation("main.c", 6, 13)));
    midBlock->addStatement(std::move(innerBlock));
    outerBlock->addStatement(std::move(midBlock));

    // return x;
    outerBlock->addStatement(std::make_unique<ReturnStmt>(
        std::make_unique<IdentifierExpression>("x", SourceLocation("main.c", 9, 12)),
        SourceLocation("main.c", 9, 5)));

    mainFn->addBodyStatement(std::move(outerBlock));

    // Valid declarations and correct scope resolution
    RedeclarationChecker redec;
    CHECK(redec.check(*mainFn));
    CHECK_FALSE(redec.hasErrors());

    UndeclaredChecker undef;
    CHECK(undef.check(*mainFn));
    CHECK_FALSE(undef.hasErrors());

    TypeChecker tc;
    CHECK(tc.check(*mainFn));
    CHECK_FALSE(tc.hasErrors());

    ReturnChecker retc;
    CHECK(retc.check(*mainFn));
    CHECK_FALSE(retc.hasErrors());
}

TEST_CASE("Integration Case 2: Duplicate declaration in one scope", "[semantic][regression][integration][case2]") {
    // int main(void) {
    //     int x = 10;
    //     int x = 20;
    //     return 0;
    // }
    auto mainFn = std::make_unique<Function>("main", Type::getInt(), SourceLocation("main.c", 1, 1), true);
    auto body = std::make_unique<CompoundStmt>(SourceLocation("main.c", 1, 16));
    body->addStatement(std::make_unique<VarDecl>(
        Type::getInt(), "x",
        std::make_unique<LiteralExpression>(LiteralKind::Integer, "10", SourceLocation("main.c", 2, 13)),
        SourceLocation("main.c", 2, 5)));
    body->addStatement(std::make_unique<VarDecl>(
        Type::getInt(), "x",
        std::make_unique<LiteralExpression>(LiteralKind::Integer, "20", SourceLocation("main.c", 3, 13)),
        SourceLocation("main.c", 3, 5)));
    body->addStatement(std::make_unique<ReturnStmt>(
        std::make_unique<LiteralExpression>(LiteralKind::Integer, "0", SourceLocation("main.c", 4, 12)),
        SourceLocation("main.c", 4, 5)));
    mainFn->addBodyStatement(std::move(body));

    RedeclarationChecker checker;
    CHECK_FALSE(checker.check(*mainFn));
    CHECK(checker.hasErrors());
    REQUIRE(checker.errorCount() == 1);
    CHECK(checker.diagnostics()[0].message.find("duplicate declaration of 'x'") != std::string::npos);
    CHECK(checker.diagnostics()[0].line() == 3);
    CHECK(checker.diagnostics()[0].previousLocation.line == 2);
}

TEST_CASE("Integration Case 3: Parameter and outermost body conflict", "[semantic][regression][integration][case3]") {
    // int calculate(int x) {
    //     int x = 20;
    //     return x;
    // }
    auto calcFn = std::make_unique<Function>("calculate", Type::getInt(), SourceLocation("calc.c", 1, 1), true);
    calcFn->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "x", nullptr, SourceLocation("calc.c", 1, 15)));

    auto body = std::make_unique<CompoundStmt>(SourceLocation("calc.c", 1, 24));
    body->addStatement(std::make_unique<VarDecl>(
        Type::getInt(), "x",
        std::make_unique<LiteralExpression>(LiteralKind::Integer, "20", SourceLocation("calc.c", 2, 13)),
        SourceLocation("calc.c", 2, 5)));
    body->addStatement(std::make_unique<ReturnStmt>(
        std::make_unique<IdentifierExpression>("x", SourceLocation("calc.c", 3, 12)),
        SourceLocation("calc.c", 3, 5)));
    calcFn->addBodyStatement(std::move(body));

    RedeclarationChecker checker;
    CHECK_FALSE(checker.check(*calcFn));
    CHECK(checker.hasErrors());
    REQUIRE(checker.errorCount() == 1);
    CHECK(checker.diagnostics()[0].message.find("duplicate declaration of 'x'") != std::string::npos);
    CHECK(checker.diagnostics()[0].line() == 2);
    CHECK(checker.diagnostics()[0].previousLocation.line == 1);
}

TEST_CASE("Integration Case 4: Legal nested shadowing of a parameter", "[semantic][regression][integration][case4]") {
    // int calculate(int x) {
    //     {
    //         int x = 20;
    //     }
    //     return x;
    // }
    auto calcFn = std::make_unique<Function>("calculate", Type::getInt(), SourceLocation("calc.c", 1, 1), true);
    calcFn->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "x", nullptr, SourceLocation("calc.c", 1, 15)));

    auto outerBody = std::make_unique<CompoundStmt>(SourceLocation("calc.c", 1, 24));
    auto nestedBlock = std::make_unique<CompoundStmt>(SourceLocation("calc.c", 2, 5));
    nestedBlock->addStatement(std::make_unique<VarDecl>(
        Type::getInt(), "x",
        std::make_unique<LiteralExpression>(LiteralKind::Integer, "20", SourceLocation("calc.c", 3, 17)),
        SourceLocation("calc.c", 3, 9)));
    outerBody->addStatement(std::move(nestedBlock));
    outerBody->addStatement(std::make_unique<ReturnStmt>(
        std::make_unique<IdentifierExpression>("x", SourceLocation("calc.c", 5, 12)),
        SourceLocation("calc.c", 5, 5)));
    calcFn->addBodyStatement(std::move(outerBody));

    RedeclarationChecker checker;
    CHECK(checker.check(*calcFn));
    CHECK_FALSE(checker.hasErrors());
}

TEST_CASE("Integration Case 5: Undefined identifier", "[semantic][regression][integration][case5]") {
    // int main(void) {
    //     int x = 10;
    //     return x + missing;
    // }
    auto mainFn = std::make_unique<Function>("main", Type::getInt(), SourceLocation("main.c", 1, 1), true);
    auto body = std::make_unique<CompoundStmt>(SourceLocation("main.c", 1, 16));
    body->addStatement(std::make_unique<VarDecl>(
        Type::getInt(), "x",
        std::make_unique<LiteralExpression>(LiteralKind::Integer, "10", SourceLocation("main.c", 2, 13)),
        SourceLocation("main.c", 2, 5)));

    auto addExpr = std::make_unique<BinaryExpr>(
        "+",
        std::make_unique<IdentifierExpression>("x", SourceLocation("main.c", 3, 12)),
        std::make_unique<IdentifierExpression>("missing", SourceLocation("main.c", 3, 16)),
        SourceLocation("main.c", 3, 14));
    body->addStatement(std::make_unique<ReturnStmt>(std::move(addExpr), SourceLocation("main.c", 3, 5)));
    mainFn->addBodyStatement(std::move(body));

    UndeclaredChecker checker;
    CHECK_FALSE(checker.check(*mainFn));
    CHECK(checker.hasErrors());
    REQUIRE(checker.errorCount() == 1);
    CHECK(checker.diagnostics()[0].identifier == "missing");
    CHECK(checker.diagnostics()[0].line() == 3);
    CHECK(checker.diagnostics()[0].column() == 16);
}

TEST_CASE("Integration Case 6: Invalid function arguments", "[semantic][regression][integration][case6]") {
    // int add(int a, int b);
    // int main(void) {
    //     return add(10, "hello");
    // }
    SymbolTable table;
    auto addFn = std::make_unique<Function>("add", Type::getInt(), SourceLocation("main.c", 1, 1), false);
    addFn->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "a", nullptr, SourceLocation("main.c", 1, 9)));
    addFn->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "b", nullptr, SourceLocation("main.c", 1, 16)));
    table.insert(*addFn);

    CallChecker checker(table);
    Parser p = makeParser("add(10, \"hello\")", "main.c");
    auto expr = p.parseExpression();
    REQUIRE(expr != nullptr);

    CHECK_FALSE(checker.check(*expr));
    CHECK(checker.hasErrors());
    REQUIRE(checker.errorCount() == 1);
    CHECK(checker.diagnostics()[0].message.find("cannot convert string literal to 'int'") != std::string::npos);
}

TEST_CASE("Integration Case 7: Multiple semantic errors", "[semantic][regression][integration][case7]") {
    // int main(void) {
    //     int x = 10;
    //     int x = 20;
    //     return missing;
    // }
    auto mainFn = std::make_unique<Function>("main", Type::getInt(), SourceLocation("main.c", 1, 1), true);
    auto body = std::make_unique<CompoundStmt>(SourceLocation("main.c", 1, 16));
    body->addStatement(std::make_unique<VarDecl>(
        Type::getInt(), "x",
        std::make_unique<LiteralExpression>(LiteralKind::Integer, "10", SourceLocation("main.c", 2, 13)),
        SourceLocation("main.c", 2, 5)));
    body->addStatement(std::make_unique<VarDecl>(
        Type::getInt(), "x",
        std::make_unique<LiteralExpression>(LiteralKind::Integer, "20", SourceLocation("main.c", 3, 13)),
        SourceLocation("main.c", 3, 5)));
    body->addStatement(std::make_unique<ReturnStmt>(
        std::make_unique<IdentifierExpression>("missing", SourceLocation("main.c", 4, 12)),
        SourceLocation("main.c", 4, 5)));
    mainFn->addBodyStatement(std::move(body));

    // 1. Redeclaration error is diagnosed
    RedeclarationChecker redec;
    CHECK_FALSE(redec.check(*mainFn));
    CHECK(redec.hasErrors());
    REQUIRE(redec.errorCount() == 1);
    CHECK(redec.diagnostics()[0].message.find("duplicate declaration of 'x'") != std::string::npos);

    // 2. Undeclared identifier is diagnosed
    UndeclaredChecker undef;
    CHECK_FALSE(undef.check(*mainFn));
    CHECK(undef.hasErrors());
    REQUIRE(undef.errorCount() == 1);
    CHECK(undef.diagnostics()[0].message.find("use of undeclared identifier 'missing'") != std::string::npos);

    // Neither checker produces duplicated messages
    CHECK(redec.errorCount() == 1);
    CHECK(undef.errorCount() == 1);
}

TEST_CASE("Integration Case 8: Independent function scopes", "[semantic][regression][integration][case8]") {
    // int first(int value) {
    //     return value;
    // }
    // int second(int value) {
    //     return value;
    // }
    auto prog = std::make_unique<Program>(SourceLocation("funcs.c", 1, 1));

    // Function 1
    auto fn1 = std::make_unique<Function>("first", Type::getInt(), SourceLocation("funcs.c", 1, 1), true);
    fn1->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "value", nullptr, SourceLocation("funcs.c", 1, 15)));
    auto body1 = std::make_unique<CompoundStmt>(SourceLocation("funcs.c", 1, 23));
    body1->addStatement(std::make_unique<ReturnStmt>(
        std::make_unique<IdentifierExpression>("value", SourceLocation("funcs.c", 2, 12)),
        SourceLocation("funcs.c", 2, 5)));
    fn1->addBodyStatement(std::move(body1));
    prog->addStatement(std::move(fn1));

    // Function 2
    auto fn2 = std::make_unique<Function>("second", Type::getInt(), SourceLocation("funcs.c", 5, 1), true);
    fn2->addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "value", nullptr, SourceLocation("funcs.c", 5, 16)));
    auto body2 = std::make_unique<CompoundStmt>(SourceLocation("funcs.c", 5, 24));
    body2->addStatement(std::make_unique<ReturnStmt>(
        std::make_unique<IdentifierExpression>("value", SourceLocation("funcs.c", 6, 12)),
        SourceLocation("funcs.c", 6, 5)));
    fn2->addBodyStatement(std::move(body2));
    prog->addStatement(std::move(fn2));

    RedeclarationChecker redec;
    CHECK(redec.check(*prog));
    CHECK_FALSE(redec.hasErrors());

    UndeclaredChecker undef;
    CHECK(undef.check(*prog));
    CHECK_FALSE(undef.hasErrors());

    TypeChecker tc;
    CHECK(tc.check(*prog));
    CHECK_FALSE(tc.hasErrors());

    ReturnChecker retc;
    CHECK(retc.check(*prog));
    CHECK_FALSE(retc.hasErrors());
}
