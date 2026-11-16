#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/function.h"
#include "ast/variable_declaration.h"
#include "ast/printer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "types/type.h"

#include <sstream>

// Helper to construct a Parser from a source string
static Parser makeParser(const std::string& src) {
    Lexer lexer(src);
    return Parser(lexer.tokenize());
}

// =============================================================================
// DAY 41 TESTS — Function Return Types
// =============================================================================

TEST_CASE("FunctionReturn: Primitive return types in prototypes", "[parser][decl][function][return_type]") {
    SECTION("void prototype: void doNothing(void);") {
        Parser parser = makeParser("void doNothing(void);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(fn->name() == "doNothing");
        CHECK(fn->isPrototype());
        CHECK_FALSE(fn->isDefinition());
        CHECK(fn->parameterListKind() == ParameterListKind::Void);

        // Return type queries
        REQUIRE(fn->returnTypePtr() != nullptr);
        CHECK(fn->returnTypePtr()->kind() == TypeKind::Void);
        CHECK(fn->returnTypeKind() == TypeKind::Void);
        CHECK(fn->returnType()->kind() == TypeKind::Void);
        CHECK(fn->isReturnVoid());
        CHECK_FALSE(fn->isReturnInt());
        CHECK_FALSE(fn->isReturnChar());
        CHECK_FALSE(fn->isReturnFloat());
        CHECK_FALSE(fn->isReturnDouble());

        // Canonical singleton equality
        CHECK(fn->returnTypeShared() == Type::getVoid());
        CHECK(fn->getReturnTypeShared() == Type::getVoid());
        CHECK(fn->getType() == Type::getVoid());
        CHECK(fn->returnType() == Type::getVoid().get());

        // String representation
        CHECK(fn->returnTypeName() == "void");
        CHECK(fn->getReturnType() == "void");
        CHECK(fn->getReturnTypeName() == "void");
        CHECK(fn->returnType() == "void");
    }

    SECTION("int prototype: int getCount();") {
        Parser parser = makeParser("int getCount();");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(fn->name() == "getCount");
        CHECK(fn->isPrototype());
        CHECK(fn->parameterListKind() == ParameterListKind::Unspecified);

        // Return type queries
        REQUIRE(fn->returnTypePtr() != nullptr);
        CHECK(fn->returnTypePtr()->kind() == TypeKind::Int);
        CHECK(fn->returnTypeKind() == TypeKind::Int);
        CHECK(fn->returnType()->kind() == TypeKind::Int);
        CHECK(fn->isReturnInt());
        CHECK_FALSE(fn->isReturnVoid());
        CHECK_FALSE(fn->isReturnChar());
        CHECK_FALSE(fn->isReturnFloat());
        CHECK_FALSE(fn->isReturnDouble());

        // Canonical singleton equality
        CHECK(fn->returnTypeShared() == Type::getInt());
        CHECK(fn->returnType() == Type::getInt().get());

        // String representation
        CHECK(fn->returnTypeName() == "int");
        CHECK(fn->getReturnType() == "int");
        CHECK(fn->returnType() == "int");
    }

    SECTION("char prototype: char getInitial(void);") {
        Parser parser = makeParser("char getInitial(void);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(fn->name() == "getInitial");
        CHECK(fn->isPrototype());

        // Return type queries
        REQUIRE(fn->returnTypePtr() != nullptr);
        CHECK(fn->returnTypePtr()->kind() == TypeKind::Char);
        CHECK(fn->returnTypeKind() == TypeKind::Char);
        CHECK(fn->returnType()->kind() == TypeKind::Char);
        CHECK(fn->isReturnChar());
        CHECK_FALSE(fn->isReturnVoid());
        CHECK_FALSE(fn->isReturnInt());

        // Canonical singleton equality
        CHECK(fn->returnTypeShared() == Type::getChar());
        CHECK(fn->returnType() == Type::getChar().get());

        // String representation
        CHECK(fn->returnTypeName() == "char");
        CHECK(fn->returnType() == "char");
    }

    SECTION("float prototype: float getRatio(int x);") {
        Parser parser = makeParser("float getRatio(int x);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(fn->name() == "getRatio");
        CHECK(fn->isPrototype());
        CHECK(fn->parameterCount() == 1);

        // Return type queries
        REQUIRE(fn->returnTypePtr() != nullptr);
        CHECK(fn->returnTypePtr()->kind() == TypeKind::Float);
        CHECK(fn->returnTypeKind() == TypeKind::Float);
        CHECK(fn->returnType()->kind() == TypeKind::Float);
        CHECK(fn->isReturnFloat());
        CHECK_FALSE(fn->isReturnDouble());

        // Canonical singleton equality
        CHECK(fn->returnTypeShared() == Type::getFloat());
        CHECK(fn->returnType() == Type::getFloat().get());

        // String representation
        CHECK(fn->returnTypeName() == "float");
        CHECK(fn->returnType() == "float");
    }

    SECTION("double prototype: double getPi();") {
        Parser parser = makeParser("double getPi();");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(fn->name() == "getPi");
        CHECK(fn->isPrototype());

        // Return type queries
        REQUIRE(fn->returnTypePtr() != nullptr);
        CHECK(fn->returnTypePtr()->kind() == TypeKind::Double);
        CHECK(fn->returnTypeKind() == TypeKind::Double);
        CHECK(fn->returnType()->kind() == TypeKind::Double);
        CHECK(fn->isReturnDouble());
        CHECK_FALSE(fn->isReturnFloat());

        // Canonical singleton equality
        CHECK(fn->returnTypeShared() == Type::getDouble());
        CHECK(fn->returnType() == Type::getDouble().get());

        // String representation
        CHECK(fn->returnTypeName() == "double");
        CHECK(fn->returnType() == "double");
    }
}

TEST_CASE("FunctionReturn: Primitive return types in definitions", "[parser][decl][function][return_type]") {
    SECTION("void definition: void reset(void) {}") {
        Parser parser = makeParser("void reset(void) {}");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(fn->name() == "reset");
        CHECK(fn->isDefinition());
        CHECK_FALSE(fn->isPrototype());
        CHECK(fn->isReturnVoid());
        CHECK(fn->returnTypeKind() == TypeKind::Void);
        CHECK(fn->returnTypeShared() == Type::getVoid());
        CHECK(fn->returnTypeName() == "void");
    }

    SECTION("int definition: int answer() { return 42; }") {
        Parser parser = makeParser("int answer() { return 42; }");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(fn->name() == "answer");
        CHECK(fn->isDefinition());
        CHECK(fn->isReturnInt());
        CHECK(fn->returnTypeKind() == TypeKind::Int);
        CHECK(fn->returnTypeShared() == Type::getInt());
        CHECK(fn->returnTypeName() == "int");
    }

    SECTION("char definition: char firstChar(char c) { return c; }") {
        Parser parser = makeParser("char firstChar(char c) { return c; }");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(fn->name() == "firstChar");
        CHECK(fn->isDefinition());
        CHECK(fn->isReturnChar());
        CHECK(fn->returnTypeKind() == TypeKind::Char);
        CHECK(fn->returnTypeShared() == Type::getChar());
        CHECK(fn->returnTypeName() == "char");
        REQUIRE(fn->parameterCount() == 1);
        CHECK(fn->parameter(0)->name() == "c");
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Char);
    }

    SECTION("float definition: float compute(float a, float b) { return a; }") {
        Parser parser = makeParser("float compute(float a, float b) { return a; }");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(fn->name() == "compute");
        CHECK(fn->isDefinition());
        CHECK(fn->isReturnFloat());
        CHECK(fn->returnTypeKind() == TypeKind::Float);
        CHECK(fn->returnTypeShared() == Type::getFloat());
        CHECK(fn->returnTypeName() == "float");
        REQUIRE(fn->parameterCount() == 2);
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Float);
        CHECK(fn->parameter(1)->typeKind() == TypeKind::Float);
    }

    SECTION("double definition: double calculate(double d) { return d; }") {
        Parser parser = makeParser("double calculate(double d) { return d; }");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(fn->name() == "calculate");
        CHECK(fn->isDefinition());
        CHECK(fn->isReturnDouble());
        CHECK(fn->returnTypeKind() == TypeKind::Double);
        CHECK(fn->returnTypeShared() == Type::getDouble());
        CHECK(fn->returnTypeName() == "double");
        REQUIRE(fn->parameterCount() == 1);
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Double);
    }
}

TEST_CASE("FunctionReturn: Functions with multiple parameters and various return types", "[parser][decl][function][return_type]") {
    SECTION("double mix(int a, float b, char c);") {
        Parser parser = makeParser("double mix(int a, float b, char c);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(fn->name() == "mix");
        CHECK(fn->returnTypeKind() == TypeKind::Double);
        CHECK(fn->isReturnDouble());
        CHECK(fn->returnTypeName() == "double");

        REQUIRE(fn->parameterCount() == 3);
        CHECK(fn->parameter(0)->name() == "a");
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Int);
        CHECK(fn->parameter(1)->name() == "b");
        CHECK(fn->parameter(1)->typeKind() == TypeKind::Float);
        CHECK(fn->parameter(2)->name() == "c");
        CHECK(fn->parameter(2)->typeKind() == TypeKind::Char);
    }

    SECTION("char pick(int idx, char first, char second);") {
        Parser parser = makeParser("char pick(int idx, char first, char second);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(fn->name() == "pick");
        CHECK(fn->returnTypeKind() == TypeKind::Char);
        CHECK(fn->isReturnChar());
        CHECK(fn->returnTypeName() == "char");

        REQUIRE(fn->parameterCount() == 3);
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Int);
        CHECK(fn->parameter(1)->typeKind() == TypeKind::Char);
        CHECK(fn->parameter(2)->typeKind() == TypeKind::Char);
    }

    SECTION("int compare(double x, double y);") {
        Parser parser = makeParser("int compare(double x, double y);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(fn->name() == "compare");
        CHECK(fn->returnTypeKind() == TypeKind::Int);
        CHECK(fn->isReturnInt());
        CHECK(fn->returnTypeName() == "int");

        REQUIRE(fn->parameterCount() == 2);
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Double);
        CHECK(fn->parameter(1)->typeKind() == TypeKind::Double);
    }

    SECTION("void setBounds(int min, int max, float step);") {
        Parser parser = makeParser("void setBounds(int min, int max, float step);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(fn->name() == "setBounds");
        CHECK(fn->returnTypeKind() == TypeKind::Void);
        CHECK(fn->isReturnVoid());
        CHECK(fn->returnTypeName() == "void");

        REQUIRE(fn->parameterCount() == 3);
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Int);
        CHECK(fn->parameter(1)->typeKind() == TypeKind::Int);
        CHECK(fn->parameter(2)->typeKind() == TypeKind::Float);
    }
}

TEST_CASE("FunctionReturn: AST node return type accessors and mutators", "[ast][function][return_type]") {
    SECTION("Construct with std::shared_ptr<Type>") {
        Function fn("myFunc", Type::getDouble(), SourceLocation{1, 1}, false, ParameterListKind::Void);

        CHECK(fn.name() == "myFunc");
        CHECK(fn.returnTypeKind() == TypeKind::Double);
        CHECK(fn.isReturnDouble());
        CHECK_FALSE(fn.isReturnInt());
        CHECK(fn.returnTypeShared() == Type::getDouble());
        CHECK(fn.getReturnTypeShared() == Type::getDouble());
        CHECK(fn.getType() == Type::getDouble());
        CHECK(fn.returnTypePtr() == Type::getDouble().get());
        CHECK(fn.declaredReturnType() == Type::getDouble().get());
        CHECK(fn.returnTypeName() == "double");
        CHECK(fn.getReturnType() == "double");
        CHECK(fn.getReturnTypeName() == "double");
        CHECK(fn.returnType() == "double");
        CHECK(fn.returnType() == Type::getDouble().get());

        // Mutate return type via shared_ptr
        fn.setReturnType(Type::getInt());
        CHECK(fn.returnTypeKind() == TypeKind::Int);
        CHECK(fn.isReturnInt());
        CHECK_FALSE(fn.isReturnDouble());
        CHECK(fn.returnTypeName() == "int");
        CHECK(fn.returnTypeShared() == Type::getInt());

        // Mutate return type via string
        fn.setReturnType("float");
        CHECK(fn.returnTypeKind() == TypeKind::Float);
        CHECK(fn.isReturnFloat());
        CHECK(fn.returnTypeName() == "float");
        CHECK(fn.returnTypeShared() == Type::getFloat());
    }

    SECTION("Construct with string return type (backward compatibility)") {
        Function fn("legacyFunc", "char", SourceLocation{1, 1}, true);

        CHECK(fn.name() == "legacyFunc");
        CHECK(fn.returnTypeKind() == TypeKind::Char);
        CHECK(fn.isReturnChar());
        CHECK(fn.returnTypeShared() == Type::getChar());
        CHECK(fn.returnTypeName() == "char");
        CHECK(fn.returnType() == "char");
        CHECK(fn.returnType() == Type::getChar().get());

        // Mutate to void via string
        fn.setReturnType("void");
        CHECK(fn.returnTypeKind() == TypeKind::Void);
        CHECK(fn.isReturnVoid());
        CHECK(fn.returnTypeShared() == Type::getVoid());
        CHECK(fn.returnTypeName() == "void");
    }

    SECTION("Function::toString() includes return type") {
        Function fnProto("calc", Type::getInt(), SourceLocation{1, 1}, false, ParameterListKind::Unspecified);
        CHECK(fnProto.toString() == "Function: int calc()");

        Function fnVoid("display", Type::getVoid(), SourceLocation{1, 1}, false, ParameterListKind::Void);
        CHECK(fnVoid.toString() == "Function: void display(void)");

        Function fnParams("square", Type::getDouble(), SourceLocation{1, 1}, false, ParameterListKind::Typed);
        fnParams.addParameter(std::make_unique<VariableDeclaration>(Type::getDouble(), "x", nullptr, SourceLocation{1, 10}));
        CHECK(fnParams.toString() == "Function: double square(double x)");
    }
}

TEST_CASE("FunctionReturn: ASTPrinter output verification", "[ast][printer][function][return_type]") {
    SECTION("Prototype AST print includes return type") {
        Parser parser = makeParser("void process(int x);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);

        ASTPrinter printer;
        std::string output = printer.print(fn.get());

        CHECK(output.find("Function (prototype): void process") != std::string::npos);
        CHECK(output.find("VariableDeclaration: int x") != std::string::npos);
    }

    SECTION("Definition AST print includes return type") {
        Parser parser = makeParser("float compute(void) {}");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);

        ASTPrinter printer;
        std::string output = printer.print(fn.get());

        CHECK(output.find("Function (definition): float compute") != std::string::npos);
        CHECK(output.find("Parameters: (none)") != std::string::npos);
        CHECK(output.find("Body:") != std::string::npos);
    }
}

TEST_CASE("FunctionReturn: Error handling for invalid or missing return types", "[parser][decl][function][return_type]") {
    SECTION("Missing return type before identifier: foo();") {
        Parser parser = makeParser("foo();");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        // Not a function declaration starting with a type specifier; should cleanly return nullptr
    }

    SECTION("Missing return type before function with parameters: add(int x, int y);") {
        Parser parser = makeParser("add(int x, int y);");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
    }

    SECTION("Numeric literal instead of return type: 123 foo();") {
        Parser parser = makeParser("123 foo();");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
    }

    SECTION("Operator instead of return type: + foo();") {
        Parser parser = makeParser("+ foo();");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
    }

    SECTION("Type specifier followed by non-identifier: int 123();") {
        Parser parser = makeParser("int 123();");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected identifier") != std::string::npos);
    }

    SECTION("Type specifier followed by punctuation: void ();") {
        Parser parser = makeParser("void ();");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected identifier") != std::string::npos);
    }
}

TEST_CASE("FunctionReturn: C17 parameter compatibility regression tests", "[parser][decl][function][return_type][c17]") {
    SECTION("int f(); has unspecified parameter list and int return type") {
        Parser parser = makeParser("int f();");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(fn->returnTypeKind() == TypeKind::Int);
        CHECK(fn->isReturnInt());
        CHECK(fn->parameterListKind() == ParameterListKind::Unspecified);
        CHECK(fn->isParameterListUnspecified());
    }

    SECTION("int f(void); has void parameter list and int return type") {
        Parser parser = makeParser("int f(void);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(fn->returnTypeKind() == TypeKind::Int);
        CHECK(fn->isReturnInt());
        CHECK(fn->parameterListKind() == ParameterListKind::Void);
        CHECK(fn->isParameterListVoid());
    }

    SECTION("double g(int, float); supports unnamed prototype parameters") {
        Parser parser = makeParser("double g(int, float);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(fn->returnTypeKind() == TypeKind::Double);
        CHECK(fn->isReturnDouble());
        REQUIRE(fn->parameterCount() == 2);
        CHECK(fn->parameter(0)->name().empty());
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Int);
        CHECK(fn->parameter(1)->name().empty());
        CHECK(fn->parameter(1)->typeKind() == TypeKind::Float);
    }

    SECTION("int h(int) {} definition requires named parameter and fails with error") {
        Parser parser = makeParser("int h(int) {}");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Function definition requires parameter names") != std::string::npos);
    }
}
