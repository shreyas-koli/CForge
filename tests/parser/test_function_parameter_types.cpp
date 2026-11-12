#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/function.h"
#include "ast/variable_declaration.h"
#include "ast/var_decl.h"
#include "ast/printer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "types/type.h"

// Helper to construct a Parser from a source string
static Parser makeParser(const std::string& src) {
    Lexer lexer(src);
    return Parser(lexer.tokenize());
}

// =============================================================================
// DAY 40 TESTS — Function Parameter Types & C Correctness
// =============================================================================

TEST_CASE("FunctionParameter: Single parameter declarations", "[parser][decl][function][parameter]") {
    SECTION("int calculate(int x);") {
        Parser parser = makeParser("int calculate(int x);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(fn->name() == "calculate");
        CHECK(fn->returnType() == "int");
        CHECK(fn->isPrototype());
        CHECK_FALSE(fn->isDefinition());
        CHECK(fn->parameterListKind() == ParameterListKind::Typed);
        CHECK(fn->isParameterListTyped());
        CHECK_FALSE(fn->isParameterListUnspecified());
        CHECK_FALSE(fn->isParameterListVoid());
        REQUIRE(fn->parameterCount() == 1);
        REQUIRE(fn->parameters().size() == 1);

        const auto* param = fn->parameter(0);
        REQUIRE(param != nullptr);
        CHECK(param->name() == "x");
        CHECK(param->getName() == "x");
        CHECK(param->hasName());

        // Type queries
        REQUIRE(param->declaredType() != nullptr);
        CHECK(param->declaredType()->kind() == TypeKind::Int);
        CHECK(param->typeKind() == TypeKind::Int);
        CHECK(param->type()->kind() == TypeKind::Int);
        CHECK(param->type()->isInteger());
        CHECK(param->type()->isPrimitive());
        CHECK_FALSE(param->isConst());
        CHECK_FALSE(param->type()->isConst());

        // Canonical singleton equality
        CHECK(param->typeShared() == Type::getInt());
        CHECK(param->getType() == Type::getInt());
        CHECK(param->type() == Type::getInt().get());

        // Dual-compatibility string accessors
        CHECK(param->type() == "int");
        CHECK(param->typeName() == "int");
        CHECK(param->getTypeName() == "int");
        CHECK(param->type()->toString() == "int");

        // Location & initializers
        CHECK(param->location().line == 1);
        CHECK(param->location().column == 15);
        CHECK_FALSE(param->hasInitializer());
        CHECK(param->initializer() == nullptr);
    }

    SECTION("char getChar(char ch);") {
        Parser parser = makeParser("char getChar(char ch);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(fn->parameterCount() == 1);
        const auto* param = fn->getParameter(0);
        REQUIRE(param != nullptr);
        CHECK(param->name() == "ch");
        CHECK(param->hasName());
        CHECK(param->typeKind() == TypeKind::Char);
        CHECK(param->type()->isInteger());
        CHECK(param->type()->size() == 1);
        CHECK(param->typeShared() == Type::getChar());
        CHECK(param->type() == "char");
    }

    SECTION("float getFloat(float val);") {
        Parser parser = makeParser("float getFloat(float val);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(fn->parameterCount() == 1);
        const auto* param = fn->parameter(0);
        REQUIRE(param != nullptr);
        CHECK(param->name() == "val");
        CHECK(param->typeKind() == TypeKind::Float);
        CHECK(param->type()->isFloatingPoint());
        CHECK(param->type()->size() == 4);
        CHECK(param->typeShared() == Type::getFloat());
        CHECK(param->type() == "float");
    }

    SECTION("double getDouble(double result);") {
        Parser parser = makeParser("double getDouble(double result);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(fn->parameterCount() == 1);
        const auto* param = fn->parameter(0);
        REQUIRE(param != nullptr);
        CHECK(param->name() == "result");
        CHECK(param->typeKind() == TypeKind::Double);
        CHECK(param->type()->isFloatingPoint());
        CHECK(param->type()->size() == 8);
        CHECK(param->typeShared() == Type::getDouble());
        CHECK(param->type() == "double");
    }
}

TEST_CASE("FunctionParameter: Fix 1 — Distinguish empty and void parameter lists", "[parser][decl][function][parameter][c17]") {
    SECTION("Empty parameter list 'int calculate();' is Unspecified") {
        Parser parser = makeParser("int calculate();");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(fn->isPrototype());
        CHECK(fn->parameterCount() == 0);
        CHECK(fn->parameters().empty());
        CHECK(fn->parameter(0) == nullptr);

        // C17 distinction: '()' is Unspecified parameter list
        CHECK(fn->parameterListKind() == ParameterListKind::Unspecified);
        CHECK(fn->isParameterListUnspecified());
        CHECK(fn->hasUnspecifiedParameters());
        CHECK_FALSE(fn->isParameterListVoid());
        CHECK_FALSE(fn->hasVoidParameterList());
        CHECK_FALSE(fn->isParameterListTyped());

        CHECK(fn->toString() == "Function: int calculate()");
    }

    SECTION("Explicit void parameter list 'int calculate(void);' is Void") {
        Parser parser = makeParser("int calculate(void);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(fn->isPrototype());
        CHECK(fn->parameterCount() == 0);
        CHECK(fn->parameters().empty());
        CHECK(fn->parameter(0) == nullptr);

        // C17 distinction: '(void)' is explicitly empty parameter list
        CHECK(fn->parameterListKind() == ParameterListKind::Void);
        CHECK(fn->isParameterListVoid());
        CHECK(fn->hasVoidParameterList());
        CHECK_FALSE(fn->isParameterListUnspecified());
        CHECK_FALSE(fn->hasUnspecifiedParameters());
        CHECK_FALSE(fn->isParameterListTyped());

        CHECK(fn->toString() == "Function: int calculate(void)");
    }

    SECTION("Typed parameter list 'int calculate(int x);' is Typed") {
        Parser parser = makeParser("int calculate(int x);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK(fn->parameterListKind() == ParameterListKind::Typed);
        CHECK(fn->isParameterListTyped());
        CHECK_FALSE(fn->isParameterListUnspecified());
        CHECK_FALSE(fn->isParameterListVoid());
    }

    SECTION("Signatures of 'int f()' and 'int f(void)' are structurally distinct") {
        Parser parser1 = makeParser("int f();");
        auto fnUnspecified = parser1.parseFunctionDeclaration();

        Parser parser2 = makeParser("int f(void);");
        auto fnVoid = parser2.parseFunctionDeclaration();

        REQUIRE(fnUnspecified != nullptr);
        REQUIRE(fnVoid != nullptr);

        CHECK(fnUnspecified->parameterListKind() != fnVoid->parameterListKind());
        CHECK(fnUnspecified->isParameterListUnspecified());
        CHECK(fnVoid->isParameterListVoid());
        CHECK(fnUnspecified->toString() != fnVoid->toString());
    }
}

TEST_CASE("FunctionParameter: Fix 2 — Unnamed prototype parameters", "[parser][decl][function][parameter][unnamed]") {
    SECTION("int add(int, int);") {
        Parser parser = makeParser("int add(int, int);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(fn->name() == "add");
        CHECK(fn->isPrototype());
        CHECK(fn->parameterListKind() == ParameterListKind::Typed);
        REQUIRE(fn->parameterCount() == 2);

        const auto* p0 = fn->parameter(0);
        REQUIRE(p0 != nullptr);
        CHECK(p0->name().empty());
        CHECK_FALSE(p0->hasName());
        CHECK(p0->typeKind() == TypeKind::Int);
        CHECK(p0->typeShared() == Type::getInt());

        const auto* p1 = fn->parameter(1);
        REQUIRE(p1 != nullptr);
        CHECK(p1->name().empty());
        CHECK_FALSE(p1->hasName());
        CHECK(p1->typeKind() == TypeKind::Int);
        CHECK(p1->typeShared() == Type::getInt());

        CHECK(fn->toString() == "Function: int add(int, int)");
    }

    SECTION("float process(char, double);") {
        Parser parser = makeParser("float process(char, double);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(fn->name() == "process");
        REQUIRE(fn->parameterCount() == 2);

        CHECK(fn->parameter(0)->name().empty());
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Char);

        CHECK(fn->parameter(1)->name().empty());
        CHECK(fn->parameter(1)->typeKind() == TypeKind::Double);

        CHECK(fn->toString() == "Function: float process(char, double)");
    }

    SECTION("Mixed named and unnamed parameters: int mix(int a, float, double c);") {
        Parser parser = makeParser("int mix(int a, float, double c);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(fn->parameterCount() == 3);

        CHECK(fn->parameter(0)->name() == "a");
        CHECK(fn->parameter(0)->hasName());
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Int);

        CHECK(fn->parameter(1)->name().empty());
        CHECK_FALSE(fn->parameter(1)->hasName());
        CHECK(fn->parameter(1)->typeKind() == TypeKind::Float);

        CHECK(fn->parameter(2)->name() == "c");
        CHECK(fn->parameter(2)->hasName());
        CHECK(fn->parameter(2)->typeKind() == TypeKind::Double);

        CHECK(fn->toString() == "Function: int mix(int a, float, double c)");
    }

    SECTION("Unnamed const parameters: void log(const char, const int);") {
        Parser parser = makeParser("void log(const char, const int);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(fn->parameterCount() == 2);

        CHECK(fn->parameter(0)->name().empty());
        CHECK(fn->parameter(0)->isConst());
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Char);
        CHECK(fn->parameter(0)->typeName() == "const char");

        CHECK(fn->parameter(1)->name().empty());
        CHECK(fn->parameter(1)->isConst());
        CHECK(fn->parameter(1)->typeKind() == TypeKind::Int);
        CHECK(fn->parameter(1)->typeName() == "const int");

        CHECK(fn->toString() == "Function: void log(const char, const int)");
    }

    SECTION("Trailing const unnamed parameter: void update(int const);") {
        Parser parser = makeParser("void update(int const);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(fn->parameterCount() == 1);
        CHECK(fn->parameter(0)->name().empty());
        CHECK(fn->parameter(0)->isConst());
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Int);
    }
}

TEST_CASE("FunctionParameter: Named prototype parameters continue working", "[parser][decl][function][parameter][named]") {
    SECTION("int add(int a, int b);") {
        Parser parser = makeParser("int add(int a, int b);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(fn->parameterCount() == 2);
        CHECK(fn->parameter(0)->name() == "a");
        CHECK(fn->parameter(0)->hasName());
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Int);

        CHECK(fn->parameter(1)->name() == "b");
        CHECK(fn->parameter(1)->hasName());
        CHECK(fn->parameter(1)->typeKind() == TypeKind::Int);

        CHECK(fn->toString() == "Function: int add(int a, int b)");
    }

    SECTION("void display(char ch, double result);") {
        Parser parser = makeParser("void display(char ch, double result);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(fn->parameterCount() == 2);
        CHECK(fn->parameter(0)->name() == "ch");
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Char);
        CHECK(fn->parameter(1)->name() == "result");
        CHECK(fn->parameter(1)->typeKind() == TypeKind::Double);
    }
}

TEST_CASE("FunctionParameter: Function definitions require parameter names", "[parser][decl][function][parameter][definition]") {
    SECTION("Missing name in single-parameter definition: 'int foo(int) { return 0; }'") {
        Parser parser = makeParser("int foo(int) { return 0; }");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Function definition requires parameter names");
    }

    SECTION("Missing name in multi-parameter definition: 'int add(int, int) { return 0; }'") {
        Parser parser = makeParser("int add(int, int) { return 0; }");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Function definition requires parameter names");
    }

    SECTION("Partially missing name in definition: 'int add(int a, int) { return 0; }'") {
        Parser parser = makeParser("int add(int a, int) { return 0; }");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Function definition requires parameter names");
    }

    SECTION("Valid named definition succeeds: 'int add(int a, int b) { return a + b; }'") {
        Parser parser = makeParser("int add(int a, int b) { return a + b; }");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(fn->isDefinition());
        REQUIRE(fn->parameterCount() == 2);
        CHECK(fn->parameter(0)->name() == "a");
        CHECK(fn->parameter(1)->name() == "b");
    }

    SECTION("Empty parameter definition succeeds: 'int zero() { return 0; }'") {
        Parser parser = makeParser("int zero() { return 0; }");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(fn->isDefinition());
        CHECK(fn->parameterCount() == 0);
    }

    SECTION("Void parameter definition succeeds: 'int zero(void) { return 0; }'") {
        Parser parser = makeParser("int zero(void) { return 0; }");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(fn->isDefinition());
        CHECK(fn->parameterCount() == 0);
        CHECK(fn->isParameterListVoid());
    }
}

TEST_CASE("FunctionParameter: Invalid void parameter combinations", "[parser][decl][function][parameter][void][error]") {
    SECTION("Named void: 'int foo(void x);'") {
        Parser parser = makeParser("int foo(void x);");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Parameter cannot have 'void' type");
    }

    SECTION("Void followed by comma: 'int foo(void, int x);'") {
        Parser parser = makeParser("int foo(void, int x);");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected ')' after 'void' in parameter list");
    }

    SECTION("Void with comma and no parameters: 'int foo(void, );'") {
        Parser parser = makeParser("int foo(void, );");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected ')' after 'void' in parameter list");
    }

    SECTION("Const void parameter: 'int foo(const void x);'") {
        Parser parser = makeParser("int foo(const void x);");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Parameter cannot have 'void' type");
    }

    SECTION("Unnamed const void: 'int foo(const void);'") {
        Parser parser = makeParser("int foo(const void);");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Parameter cannot have 'void' type");
    }

    SECTION("Second parameter is void: 'int foo(int a, void);'") {
        Parser parser = makeParser("int foo(int a, void);");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Parameter cannot have 'void' type");
    }

    SECTION("Second parameter is named void: 'int foo(int a, void b);'") {
        Parser parser = makeParser("int foo(int a, void b);");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Parameter cannot have 'void' type");
    }
}

TEST_CASE("FunctionParameter: Const-qualified parameter behavior", "[parser][decl][function][parameter][const]") {
    SECTION("Leading const with identifiers") {
        Parser parser = makeParser("int send(const char ch, const int id);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(fn->parameterCount() == 2);
        CHECK(fn->parameter(0)->name() == "ch");
        CHECK(fn->parameter(0)->isConst());
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Char);
        CHECK(fn->parameter(0)->type() == "const char");

        CHECK(fn->parameter(1)->name() == "id");
        CHECK(fn->parameter(1)->isConst());
        CHECK(fn->parameter(1)->typeKind() == TypeKind::Int);
        CHECK(fn->parameter(1)->type() == "const int");
    }

    SECTION("Trailing const with identifiers") {
        Parser parser = makeParser("void config(int const flags, float const rate);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(fn->parameterCount() == 2);
        CHECK(fn->parameter(0)->name() == "flags");
        CHECK(fn->parameter(0)->isConst());
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Int);

        CHECK(fn->parameter(1)->name() == "rate");
        CHECK(fn->parameter(1)->isConst());
        CHECK(fn->parameter(1)->typeKind() == TypeKind::Float);
    }

    SECTION("Mixed constness across multiple parameters") {
        Parser parser = makeParser("int mix(const int a, float b, double const c, char d);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(fn->parameterCount() == 4);
        CHECK(fn->parameter(0)->isConst());
        CHECK_FALSE(fn->parameter(1)->isConst());
        CHECK(fn->parameter(2)->isConst());
        CHECK_FALSE(fn->parameter(3)->isConst());
    }
}

TEST_CASE("FunctionParameter: Source locations across lines", "[parser][decl][function][parameter][location]") {
    SECTION("Multi-line parameter list") {
        const std::string src =
            "int calculate(\n"
            "    int a,\n"
            "    const float b,\n"
            "    double c\n"
            ");";
        Parser parser = makeParser(src);
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(fn->parameterCount() == 3);

        const auto* p0 = fn->parameter(0);
        CHECK(p0->location().line == 2);
        CHECK(p0->location().column == 5);

        const auto* p1 = fn->parameter(1);
        CHECK(p1->location().line == 3);
        CHECK(p1->location().column == 5); // start of 'const'

        const auto* p2 = fn->parameter(2);
        CHECK(p2->location().line == 4);
        CHECK(p2->location().column == 5);
    }
}

TEST_CASE("FunctionParameter: AST node accessors and toString", "[parser][decl][function][parameter][ast]") {
    Function fn("testFunc", "int", SourceLocation{1, 1}, false);
    fn.addParameter(std::make_unique<VariableDeclaration>(Type::getInt(), "x", nullptr, SourceLocation{1, 10}));
    fn.addParameter(std::make_unique<VariableDeclaration>(Type::getFloat(true), "y", nullptr, SourceLocation{1, 17}));

    REQUIRE(fn.parameterCount() == 2);
    CHECK(fn.parameter(0)->name() == "x");
    CHECK(fn.parameter(0)->typeKind() == TypeKind::Int);
    CHECK_FALSE(fn.parameter(0)->isConst());

    CHECK(fn.parameter(1)->name() == "y");
    CHECK(fn.parameter(1)->typeKind() == TypeKind::Float);
    CHECK(fn.parameter(1)->isConst());

    // Out-of-bounds parameter access returns nullptr
    CHECK(fn.parameter(2) == nullptr);
    CHECK(fn.getParameter(99) == nullptr);

    // Function toString
    CHECK(fn.toString() == "Function: int testFunc(int x, const float y)");
}

TEST_CASE("FunctionParameter: ASTPrinter output verification", "[parser][decl][function][parameter][printer]") {
    SECTION("Prototype with typed named parameters") {
        Parser parser = makeParser("int add(int a, const char b);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);

        ASTPrinter printer;
        std::string output = printer.print(fn.get());

        std::string expected =
            "Function (prototype): int add\n"
            "  Parameters:\n"
            "    VariableDeclaration: int a\n"
            "    VariableDeclaration: const char b\n";
        CHECK(output == expected);
    }

    SECTION("Prototype with unnamed parameters") {
        Parser parser = makeParser("int add(int, const char);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);

        ASTPrinter printer;
        std::string output = printer.print(fn.get());

        std::string expected =
            "Function (prototype): int add\n"
            "  Parameters:\n"
            "    VariableDeclaration: int\n"
            "    VariableDeclaration: const char\n";
        CHECK(output == expected);
    }

    SECTION("Prototype with void parameter list") {
        Parser parser = makeParser("void run(void);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);

        ASTPrinter printer;
        std::string output = printer.print(fn.get());

        std::string expected =
            "Function (prototype): void run\n"
            "  Parameters: (none)\n";
        CHECK(output == expected);
    }
}

TEST_CASE("FunctionParameter: General error handling and diagnostic messages", "[parser][decl][function][parameter][error]") {
    SECTION("Missing type specifier: 'int foo(a);'") {
        Parser parser = makeParser("int foo(a);");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected parameter type before 'a'");
    }

    SECTION("Missing type after const: 'int foo(const);'") {
        Parser parser = makeParser("int foo(const);");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected type specifier after 'const'");
    }

    SECTION("Trailing comma in parameter list: 'int foo(int a, );'") {
        Parser parser = makeParser("int foo(int a, );");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected parameter declaration after ','");
    }

    SECTION("Missing comma between parameters: 'int foo(int a int b);'") {
        Parser parser = makeParser("int foo(int a int b);");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected ',' or ')' after parameter declaration");
    }

    SECTION("Unclosed parameter list at EOF: 'int foo(int a'") {
        Parser parser = makeParser("int foo(int a");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected ')' after parameter list");
    }
}

TEST_CASE("FunctionParameter: Non-confusion and regression verification", "[parser][decl][function][parameter][regression]") {
    SECTION("Variable declarations are not confused with function declarations") {
        Parser parser = makeParser("int x = 10;");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK_FALSE(parser.hasError());

        // Parse as variable declaration instead
        Parser varParser = makeParser("int x = 10;");
        auto var = varParser.parseVarDecl();
        REQUIRE(var != nullptr);
        CHECK(var->name() == "x");
        CHECK(var->typeKind() == TypeKind::Int);
    }

    SECTION("Const variable declarations remain unaffected") {
        Parser parser = makeParser("const float pi = 3.14;");
        auto var = parser.parseVarDecl();
        REQUIRE(var != nullptr);
        CHECK(var->name() == "pi");
        CHECK(var->typeKind() == TypeKind::Float);
        CHECK(var->isConst());
    }

    SECTION("Multiple functions in translation unit with mixed prototypes and definitions") {
        const std::string src =
            "int add(int, int);\n"
            "float scale(float val, double factor);\n"
            "void reset(void);\n"
            "int main(void) { return 0; }\n";
        Parser parser = makeParser(src);
        auto prog = parser.parseProgram();
        REQUIRE(prog != nullptr);
        CHECK_FALSE(parser.hasError());
        REQUIRE(prog->statements().size() == 4);

        const auto* fn0 = dynamic_cast<const Function*>(prog->statements()[0].get());
        REQUIRE(fn0 != nullptr);
        CHECK(fn0->name() == "add");
        CHECK(fn0->isPrototype());
        REQUIRE(fn0->parameterCount() == 2);
        CHECK(fn0->parameter(0)->name().empty());
        CHECK(fn0->parameter(1)->name().empty());

        const auto* fn1 = dynamic_cast<const Function*>(prog->statements()[1].get());
        REQUIRE(fn1 != nullptr);
        CHECK(fn1->name() == "scale");
        CHECK(fn1->isPrototype());
        REQUIRE(fn1->parameterCount() == 2);
        CHECK(fn1->parameter(0)->name() == "val");
        CHECK(fn1->parameter(1)->name() == "factor");

        const auto* fn2 = dynamic_cast<const Function*>(prog->statements()[2].get());
        REQUIRE(fn2 != nullptr);
        CHECK(fn2->name() == "reset");
        CHECK(fn2->isPrototype());
        CHECK(fn2->isParameterListVoid());

        const auto* fn3 = dynamic_cast<const Function*>(prog->statements()[3].get());
        REQUIRE(fn3 != nullptr);
        CHECK(fn3->name() == "main");
        CHECK(fn3->isDefinition());
        CHECK(fn3->isParameterListVoid());
    }
}
