#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/var_decl.h"
#include "ast/variable_declaration.h"
#include "ast/function.h"
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
// DAY 42 TESTS — Storage-Class Specifiers & Type Qualifiers
// =============================================================================

TEST_CASE("StorageQualifiers: Volatile type qualifier on variable declarations", "[parser][decl][qualifier][volatile]") {
    SECTION("volatile int x;") {
        Parser parser = makeParser("volatile int x;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "x");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Int);
        CHECK(decl->typeKind() == TypeKind::Int);
        CHECK(decl->type()->isVolatile());
        CHECK(decl->isVolatile());
        CHECK_FALSE(decl->isConst());
        CHECK_FALSE(decl->type()->isConst());
        CHECK(decl->storageClass() == StorageClass::None);
        CHECK_FALSE(decl->isStatic());
        CHECK_FALSE(decl->isExtern());
        CHECK_FALSE(decl->isRegister());
        CHECK(decl->type()->toString() == "volatile int");
        CHECK(decl->toString() == "VarDecl: volatile int x");
    }

    SECTION("volatile char ch;") {
        Parser parser = makeParser("volatile char ch;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "ch");
        CHECK(decl->isVolatile());
        CHECK(decl->type()->size() == 1);
        CHECK(decl->type()->toString() == "volatile char");
    }

    SECTION("volatile float f = 1.5;") {
        Parser parser = makeParser("volatile float f = 1.5;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "f");
        CHECK(decl->isVolatile());
        CHECK(decl->hasInitializer());
        CHECK(decl->type()->toString() == "volatile float");
    }

    SECTION("const volatile int cv;") {
        Parser parser = makeParser("const volatile int cv;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "cv");
        CHECK(decl->isConst());
        CHECK(decl->isVolatile());
        CHECK(decl->type()->isConst());
        CHECK(decl->type()->isVolatile());
        CHECK(decl->type()->toString() == "const volatile int");
        CHECK(decl->type() == Type::getInt(true, true).get());
    }

    SECTION("volatile const int vc = 10;") {
        Parser parser = makeParser("volatile const int vc = 10;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "vc");
        CHECK(decl->isConst());
        CHECK(decl->isVolatile());
        CHECK(decl->type()->toString() == "const volatile int");
        CHECK(decl->type() == Type::getInt(true, true).get());
    }

    SECTION("Trailing qualifiers: int volatile y;") {
        Parser parser = makeParser("int volatile y;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "y");
        CHECK(decl->isVolatile());
        CHECK_FALSE(decl->isConst());
        CHECK(decl->type()->toString() == "volatile int");
    }

    SECTION("Trailing qualifiers: int const volatile z;") {
        Parser parser = makeParser("int const volatile z;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "z");
        CHECK(decl->isConst());
        CHECK(decl->isVolatile());
        CHECK(decl->type()->toString() == "const volatile int");
    }
}

TEST_CASE("StorageQualifiers: Storage-class specifiers on variable declarations", "[parser][decl][storage_class]") {
    SECTION("static int s;") {
        Parser parser = makeParser("static int s;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "s");
        CHECK(decl->storageClass() == StorageClass::Static);
        CHECK(decl->isStatic());
        CHECK_FALSE(decl->isExtern());
        CHECK_FALSE(decl->isRegister());
        CHECK_FALSE(decl->isConst());
        CHECK_FALSE(decl->isVolatile());
        CHECK(decl->type()->toString() == "int");
        CHECK(decl->toString() == "VarDecl: static int s");
    }

    SECTION("static int s2 = 100;") {
        Parser parser = makeParser("static int s2 = 100;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "s2");
        CHECK(decl->isStatic());
        CHECK(decl->hasInitializer());
        CHECK(decl->toString() == "VarDecl: static int s2 = LiteralExpression(Integer, 100)");
    }

    SECTION("extern int ext;") {
        Parser parser = makeParser("extern int ext;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "ext");
        CHECK(decl->storageClass() == StorageClass::Extern);
        CHECK(decl->isExtern());
        CHECK_FALSE(decl->isStatic());
        CHECK_FALSE(decl->isRegister());
        CHECK(decl->toString() == "VarDecl: extern int ext");
    }

    SECTION("extern char ext_ch;") {
        Parser parser = makeParser("extern char ext_ch;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "ext_ch");
        CHECK(decl->isExtern());
        CHECK(decl->type()->kind() == TypeKind::Char);
    }

    SECTION("register int r;") {
        Parser parser = makeParser("register int r;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "r");
        CHECK(decl->storageClass() == StorageClass::Register);
        CHECK(decl->isRegister());
        CHECK_FALSE(decl->isStatic());
        CHECK_FALSE(decl->isExtern());
        CHECK(decl->toString() == "VarDecl: register int r");
    }

    SECTION("register int count = 0;") {
        Parser parser = makeParser("register int count = 0;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "count");
        CHECK(decl->isRegister());
        CHECK(decl->hasInitializer());
    }
}

TEST_CASE("StorageQualifiers: Combined storage classes and type qualifiers", "[parser][decl][storage_class][qualifier]") {
    SECTION("static const int sc = 42;") {
        Parser parser = makeParser("static const int sc = 42;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "sc");
        CHECK(decl->isStatic());
        CHECK(decl->isConst());
        CHECK_FALSE(decl->isVolatile());
        CHECK(decl->type()->toString() == "const int");
        CHECK(decl->toString() == "VarDecl: static const int sc = LiteralExpression(Integer, 42)");
    }

    SECTION("const static int cs = 42; (reversed specifier order)") {
        Parser parser = makeParser("const static int cs = 42;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "cs");
        CHECK(decl->isStatic());
        CHECK(decl->isConst());
        CHECK(decl->type()->toString() == "const int");
    }

    SECTION("static volatile int sv;") {
        Parser parser = makeParser("static volatile int sv;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "sv");
        CHECK(decl->isStatic());
        CHECK(decl->isVolatile());
        CHECK_FALSE(decl->isConst());
        CHECK(decl->type()->toString() == "volatile int");
    }

    SECTION("extern const int ec;") {
        Parser parser = makeParser("extern const int ec;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "ec");
        CHECK(decl->isExtern());
        CHECK(decl->isConst());
        CHECK(decl->type()->toString() == "const int");
    }

    SECTION("static const volatile int scv = 5;") {
        Parser parser = makeParser("static const volatile int scv = 5;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "scv");
        CHECK(decl->isStatic());
        CHECK(decl->isConst());
        CHECK(decl->isVolatile());
        CHECK(decl->type()->toString() == "const volatile int");
    }

    SECTION("extern const volatile double ecvd;") {
        Parser parser = makeParser("extern const volatile double ecvd;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "ecvd");
        CHECK(decl->isExtern());
        CHECK(decl->isConst());
        CHECK(decl->isVolatile());
        CHECK(decl->type()->kind() == TypeKind::Double);
        CHECK(decl->type()->toString() == "const volatile double");
    }

    SECTION("Interleaved order: int static const isc = 10;") {
        Parser parser = makeParser("int static const isc = 10;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "isc");
        CHECK(decl->isStatic());
        CHECK(decl->isConst());
        CHECK(decl->type()->toString() == "const int");
    }

    SECTION("Interleaved order: int volatile static ivs;") {
        Parser parser = makeParser("int volatile static ivs;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "ivs");
        CHECK(decl->isStatic());
        CHECK(decl->isVolatile());
        CHECK(decl->type()->toString() == "volatile int");
    }
}

TEST_CASE("StorageQualifiers: Function declarations with storage classes and qualifiers", "[parser][decl][function][storage_class]") {
    SECTION("static int add(int a, int b);") {
        Parser parser = makeParser("static int add(int a, int b);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(fn->name() == "add");
        CHECK(fn->isStatic());
        CHECK_FALSE(fn->isExtern());
        CHECK(fn->storageClass() == StorageClass::Static);
        CHECK(fn->isPrototype());
        CHECK(fn->returnTypeKind() == TypeKind::Int);
        CHECK(fn->toString().find("Function: static int add(") != std::string::npos);
    }

    SECTION("extern void doNothing(void);") {
        Parser parser = makeParser("extern void doNothing(void);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(fn->name() == "doNothing");
        CHECK(fn->isExtern());
        CHECK_FALSE(fn->isStatic());
        CHECK(fn->storageClass() == StorageClass::Extern);
        CHECK(fn->isPrototype());
        CHECK(fn->returnTypeKind() == TypeKind::Void);
        CHECK(fn->isReturnVoid());
    }

    SECTION("static void helper(void) {} (definition)") {
        Parser parser = makeParser("static void helper(void) {}");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(fn->name() == "helper");
        CHECK(fn->isStatic());
        CHECK(fn->isDefinition());
    }

    SECTION("Function with qualified return type: static const int getFixed(void);") {
        Parser parser = makeParser("static const int getFixed(void);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(fn->name() == "getFixed");
        CHECK(fn->isStatic());
        REQUIRE(fn->returnTypePtr() != nullptr);
        CHECK(fn->returnTypePtr()->isConst());
        CHECK(fn->returnTypePtr()->kind() == TypeKind::Int);
        CHECK(fn->returnTypePtr()->toString() == "const int");
    }

    SECTION("Function with qualified return type: extern volatile int readPort();") {
        Parser parser = makeParser("extern volatile int readPort();");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(fn->name() == "readPort");
        CHECK(fn->isExtern());
        REQUIRE(fn->returnTypePtr() != nullptr);
        CHECK(fn->returnTypePtr()->isVolatile());
        CHECK(fn->returnTypePtr()->kind() == TypeKind::Int);
        CHECK(fn->returnTypePtr()->toString() == "volatile int");
    }
}

TEST_CASE("StorageQualifiers: Function parameters with qualifiers and register", "[parser][decl][function][parameter]") {
    SECTION("void process(register int counter);") {
        Parser parser = makeParser("void process(register int counter);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(fn->parameterCount() == 1);
        const auto* param = fn->parameter(0);
        REQUIRE(param != nullptr);
        CHECK(param->name() == "counter");
        CHECK(param->isRegister());
        CHECK(param->storageClass() == StorageClass::Register);
        CHECK(param->typeKind() == TypeKind::Int);
    }

    SECTION("void compute(volatile int flag);") {
        Parser parser = makeParser("void compute(volatile int flag);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(fn->parameterCount() == 1);
        const auto* param = fn->parameter(0);
        REQUIRE(param != nullptr);
        CHECK(param->name() == "flag");
        CHECK(param->isVolatile());
        CHECK(param->type()->isVolatile());
        CHECK_FALSE(param->isRegister());
    }

    SECTION("void handle(const volatile int status);") {
        Parser parser = makeParser("void handle(const volatile int status);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(fn->parameterCount() == 1);
        const auto* param = fn->parameter(0);
        REQUIRE(param != nullptr);
        CHECK(param->isConst());
        CHECK(param->isVolatile());
        CHECK(param->type()->toString() == "const volatile int");
    }

    SECTION("void mixed(register int a, const char b, volatile float c);") {
        Parser parser = makeParser("void mixed(register int a, const char b, volatile float c);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(fn->parameterCount() == 3);
        CHECK(fn->parameter(0)->isRegister());
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Int);

        CHECK(fn->parameter(1)->isConst());
        CHECK_FALSE(fn->parameter(1)->isRegister());
        CHECK(fn->parameter(1)->typeKind() == TypeKind::Char);

        CHECK(fn->parameter(2)->isVolatile());
        CHECK_FALSE(fn->parameter(2)->isRegister());
        CHECK(fn->parameter(2)->typeKind() == TypeKind::Float);
    }
}

TEST_CASE("StorageQualifiers: ASTPrinter output verification", "[ast][printer][storage_class][qualifier]") {
    ASTPrinter printer;

    SECTION("Variable declarations printed with storage class") {
        Parser parser = makeParser("static int x = 10;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);

        std::string output = printer.print(decl.get());
        CHECK(output.find("VarDecl: static int x") != std::string::npos);
    }

    SECTION("Variable declarations printed with extern and const volatile") {
        Parser parser = makeParser("extern const volatile int y;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);

        std::string output = printer.print(decl.get());
        CHECK(output.find("VarDecl: extern const volatile int y") != std::string::npos);
    }

    SECTION("Function declaration printed with static") {
        Parser parser = makeParser("static int add(int a, int b);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);

        std::string output = printer.print(fn.get());
        CHECK(output.find("Function (prototype): static int add") != std::string::npos);
    }

    SECTION("Function declaration printed with extern") {
        Parser parser = makeParser("extern void greet(void);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);

        std::string output = printer.print(fn.get());
        CHECK(output.find("Function (prototype): extern void greet") != std::string::npos);
    }

    SECTION("Function parameter printed with register") {
        Parser parser = makeParser("void run(register int x);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);

        std::string output = printer.print(fn.get());
        CHECK(output.find("VariableDeclaration: register int x") != std::string::npos);
    }
}

TEST_CASE("StorageQualifiers: Type flyweights and qualifier helpers", "[types][qualifiers]") {
    SECTION("Volatile flyweight instances") {
        auto volInt1 = Type::getInt(false, true);
        auto volInt2 = Type::getInt(false, true);
        REQUIRE(volInt1 != nullptr);
        CHECK(volInt1 == volInt2); // Flyweight canonical identity
        CHECK(volInt1->isVolatile());
        CHECK_FALSE(volInt1->isConst());
        CHECK(volInt1->toString() == "volatile int");

        auto cvInt1 = Type::getInt(true, true);
        auto cvInt2 = Type::getInt(true, true);
        REQUIRE(cvInt1 != nullptr);
        CHECK(cvInt1 == cvInt2);
        CHECK(cvInt1->isConst());
        CHECK(cvInt1->isVolatile());
        CHECK(cvInt1->toString() == "const volatile int");

        // Mutually distinct singletons
        CHECK(Type::getInt() != volInt1);
        CHECK(Type::getInt(true) != cvInt1);
        CHECK(volInt1 != cvInt1);
    }

    SECTION("withVolatile and asVolatile transformations") {
        auto tInt = Type::getInt();
        auto tVolInt = tInt->withVolatile(true);
        REQUIRE(tVolInt != nullptr);
        CHECK(tVolInt == Type::getInt(false, true));

        auto tConstInt = Type::getInt(true);
        auto tConstVolInt = tConstInt->withVolatile(true);
        REQUIRE(tConstVolInt != nullptr);
        CHECK(tConstVolInt == Type::getInt(true, true));

        auto tUnvol = tConstVolInt->withVolatile(false);
        CHECK(tUnvol == Type::getInt(true, false));
    }

    SECTION("primitiveTypeFromKeyword with volatile") {
        auto t1 = primitiveTypeFromKeyword("volatile int");
        REQUIRE(t1 != nullptr);
        CHECK(t1 == Type::getInt(false, true));

        auto t2 = primitiveTypeFromKeyword("const volatile char");
        REQUIRE(t2 != nullptr);
        CHECK(t2 == Type::getChar(true, true));

        auto t3 = primitiveTypeFromKeyword("volatile const double");
        REQUIRE(t3 != nullptr);
        CHECK(t3 == Type::getDouble(true, true));
    }
}

TEST_CASE("StorageQualifiers: Error handling for malformed or conflicting specifiers", "[parser][decl][errors]") {
    SECTION("Multiple conflicting storage classes: 'static extern int x;'") {
        Parser parser = makeParser("static extern int x;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Multiple storage class specifiers in declaration") != std::string::npos);
    }

    SECTION("Multiple conflicting storage classes: 'static register int x;'") {
        Parser parser = makeParser("static register int x;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Multiple storage class specifiers in declaration") != std::string::npos);
    }

    SECTION("Multiple conflicting storage classes: 'extern static int x;'") {
        Parser parser = makeParser("extern static int x;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Multiple storage class specifiers in declaration") != std::string::npos);
    }

    SECTION("Duplicate storage class: 'static static int x;'") {
        Parser parser = makeParser("static static int x;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Duplicate storage class specifier 'static'") != std::string::npos);
    }

    SECTION("Duplicate storage class: 'extern extern int x;'") {
        Parser parser = makeParser("extern extern int x;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Duplicate storage class specifier 'extern'") != std::string::npos);
    }

    SECTION("Duplicate storage class: 'register register int x;'") {
        Parser parser = makeParser("register register int x;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Duplicate storage class specifier 'register'") != std::string::npos);
    }

    SECTION("Missing type specifier after static: 'static;'") {
        Parser parser = makeParser("static;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected type specifier after 'static'") != std::string::npos);
    }

    SECTION("Missing type specifier after extern: 'extern;'") {
        Parser parser = makeParser("extern;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected type specifier after 'extern'") != std::string::npos);
    }

    SECTION("Missing type specifier after register: 'register;'") {
        Parser parser = makeParser("register;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected type specifier after 'register'") != std::string::npos);
    }

    SECTION("Missing type specifier after volatile: 'volatile;'") {
        Parser parser = makeParser("volatile;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected type specifier after 'volatile'") != std::string::npos);
    }

    SECTION("Function declaration with register storage class: 'register int f();'") {
        Parser parser = makeParser("register int f();");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Function declaration cannot have 'register' storage class") != std::string::npos);
    }

    SECTION("Function parameter with static storage class: 'void f(static int x);'") {
        Parser parser = makeParser("void f(static int x);");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Parameter cannot have 'static' storage class") != std::string::npos);
    }

    SECTION("Function parameter with extern storage class: 'void f(extern int x);'") {
        Parser parser = makeParser("void f(extern int x);");
        auto fn = parser.parseFunctionDeclaration();
        CHECK(fn == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Parameter cannot have 'extern' storage class") != std::string::npos);
    }

    SECTION("Multiple type specifiers: 'int char x;'") {
        Parser parser = makeParser("int char x;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Cannot combine with previous type specifier") != std::string::npos);
    }
}

TEST_CASE("StorageQualifiers: Regression and non-confusion verification", "[parser][decl][regression]") {
    SECTION("parseDeclaration ignores variable declarations with storage class") {
        Parser p1 = makeParser("static int x = 10;");
        auto fn1 = p1.parseDeclaration();
        CHECK(fn1 == nullptr);
        CHECK_FALSE(p1.hasError());
        CHECK(p1.peek().lexeme == "static");

        Parser p2 = makeParser("extern int x;");
        auto fn2 = p2.parseDeclaration();
        CHECK(fn2 == nullptr);
        CHECK_FALSE(p2.hasError());
        CHECK(p2.peek().lexeme == "extern");

        Parser p3 = makeParser("register int x;");
        auto fn3 = p3.parseDeclaration();
        CHECK(fn3 == nullptr);
        CHECK_FALSE(p3.hasError());
        CHECK(p3.peek().lexeme == "register");

        Parser p4 = makeParser("volatile int x;");
        auto fn4 = p4.parseDeclaration();
        CHECK(fn4 == nullptr);
        CHECK_FALSE(p4.hasError());
        CHECK(p4.peek().lexeme == "volatile");
    }

    SECTION("Ordinary declarations without qualifiers continue working unchanged") {
        Parser parser = makeParser("int a; char b = 'c';");
        auto d1 = parser.parseVarDecl();
        REQUIRE(d1 != nullptr);
        CHECK(d1->storageClass() == StorageClass::None);
        CHECK_FALSE(d1->isConst());
        CHECK_FALSE(d1->isVolatile());
        CHECK(d1->name() == "a");

        auto d2 = parser.parseVarDecl();
        REQUIRE(d2 != nullptr);
        CHECK(d2->storageClass() == StorageClass::None);
        CHECK_FALSE(d2->isConst());
        CHECK_FALSE(d2->isVolatile());
        CHECK(d2->name() == "b");
    }
}
