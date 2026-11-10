#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/var_decl.h"
#include "ast/literal_expression.h"
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
// DAY 38 TESTS — const Qualifier
// =============================================================================

TEST_CASE("ConstQualifier: Declarations without initializer", "[parser][decl][const]") {
    SECTION("const int x;") {
        Parser parser = makeParser("const int x;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "x");
        CHECK(decl->getName() == "x");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Int);
        CHECK(decl->typeKind() == TypeKind::Int);
        CHECK(decl->type()->isConst());
        CHECK(decl->isConst());
        CHECK(decl->type()->isInteger());
        CHECK_FALSE(decl->hasInitializer());
        CHECK(decl->initializer() == nullptr);
        CHECK(decl->type()->toString() == "const int");
        CHECK(std::string(decl->type()->name()) == "const int");
        CHECK(decl->location().line == 1);
        CHECK(decl->location().column == 1);
    }

    SECTION("const char ch;") {
        Parser parser = makeParser("const char ch;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "ch");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Char);
        CHECK(decl->type()->isConst());
        CHECK(decl->isConst());
        CHECK_FALSE(decl->hasInitializer());
        CHECK(decl->type()->toString() == "const char");
        CHECK(decl->type()->size() == 1);
    }

    SECTION("const float value;") {
        Parser parser = makeParser("const float value;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "value");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Float);
        CHECK(decl->type()->isConst());
        CHECK(decl->isConst());
        CHECK(decl->type()->isFloatingPoint());
        CHECK_FALSE(decl->hasInitializer());
        CHECK(decl->type()->toString() == "const float");
        CHECK(decl->type()->size() == 4);
    }

    SECTION("const double result;") {
        Parser parser = makeParser("const double result;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "result");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Double);
        CHECK(decl->type()->isConst());
        CHECK(decl->isConst());
        CHECK(decl->type()->isFloatingPoint());
        CHECK_FALSE(decl->hasInitializer());
        CHECK(decl->type()->toString() == "const double");
        CHECK(decl->type()->size() == 8);
    }

    SECTION("const void v;") {
        Parser parser = makeParser("const void v;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "v");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Void);
        CHECK(decl->type()->isConst());
        CHECK(decl->isConst());
        CHECK(decl->type()->isVoid());
        CHECK_FALSE(decl->hasInitializer());
        CHECK(decl->type()->toString() == "const void");
    }
}

TEST_CASE("ConstQualifier: Declarations with initializer", "[parser][decl][const]") {
    SECTION("const int y = 10;") {
        Parser parser = makeParser("const int y = 10;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "y");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Int);
        CHECK(decl->isConst());
        CHECK(decl->type()->isConst());
        CHECK(decl->hasInitializer());
        REQUIRE(decl->initializer() != nullptr);

        auto lit = dynamic_cast<const LiteralExpression*>(decl->initializer());
        REQUIRE(lit != nullptr);
        CHECK(lit->value() == "10");
        CHECK(lit->kind() == LiteralKind::Integer);
    }

    SECTION("const char ch = 'A';") {
        Parser parser = makeParser("const char ch = 'A';");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "ch");
        CHECK(decl->isConst());
        CHECK(decl->type()->isConst());
        CHECK(decl->hasInitializer());
        REQUIRE(decl->initializer() != nullptr);

        auto lit = dynamic_cast<const LiteralExpression*>(decl->initializer());
        REQUIRE(lit != nullptr);
        CHECK(lit->value() == "'A'");
        CHECK(lit->kind() == LiteralKind::Char);
    }

    SECTION("const float value = 3.14;") {
        Parser parser = makeParser("const float value = 3.14;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "value");
        CHECK(decl->isConst());
        CHECK(decl->type()->isConst());
        CHECK(decl->hasInitializer());
        REQUIRE(decl->initializer() != nullptr);

        auto lit = dynamic_cast<const LiteralExpression*>(decl->initializer());
        REQUIRE(lit != nullptr);
        CHECK(lit->value() == "3.14");
        CHECK(lit->kind() == LiteralKind::Float);
    }

    SECTION("const double result = 2.71828;") {
        Parser parser = makeParser("const double result = 2.71828;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "result");
        CHECK(decl->isConst());
        CHECK(decl->type()->isConst());
        CHECK(decl->hasInitializer());
        REQUIRE(decl->initializer() != nullptr);

        auto lit = dynamic_cast<const LiteralExpression*>(decl->initializer());
        REQUIRE(lit != nullptr);
        CHECK(lit->value() == "2.71828");
        CHECK(lit->kind() == LiteralKind::Float);
    }
}

TEST_CASE("ConstQualifier: Diverse initializer expressions", "[parser][decl][const]") {
    SECTION("Binary arithmetic expression: const int sum = a + b;") {
        Parser parser = makeParser("const int sum = a + b;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "sum");
        CHECK(decl->isConst());
        REQUIRE(decl->initializer() != nullptr);

        auto bin = dynamic_cast<const BinaryExpr*>(decl->initializer());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");

        auto left = dynamic_cast<const IdentifierExpression*>(bin->left());
        auto right = dynamic_cast<const IdentifierExpression*>(bin->right());
        REQUIRE(left != nullptr);
        REQUIRE(right != nullptr);
        CHECK(left->name() == "a");
        CHECK(right->name() == "b");
    }

    SECTION("Unary expression: const int neg = -x;") {
        Parser parser = makeParser("const int neg = -x;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "neg");
        CHECK(decl->isConst());
        REQUIRE(decl->initializer() != nullptr);

        auto un = dynamic_cast<const UnaryExpr*>(decl->initializer());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "-");
    }

    SECTION("Conditional operator: const int max = (a > b) ? a : b;") {
        Parser parser = makeParser("const int max = (a > b) ? a : b;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "max");
        CHECK(decl->isConst());
        REQUIRE(decl->initializer() != nullptr);

        auto cond = dynamic_cast<const ConditionalExpr*>(decl->initializer());
        REQUIRE(cond != nullptr);
    }
}

TEST_CASE("ConstQualifier: Trailing const qualifier ('int const x;')", "[parser][decl][const]") {
    SECTION("int const x;") {
        Parser parser = makeParser("int const x;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "x");
        CHECK(decl->isConst());
        CHECK(decl->type()->isConst());
        CHECK(decl->type()->kind() == TypeKind::Int);
        CHECK_FALSE(decl->hasInitializer());
    }

    SECTION("int const y = 42;") {
        Parser parser = makeParser("int const y = 42;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "y");
        CHECK(decl->isConst());
        CHECK(decl->type()->isConst());
        CHECK(decl->hasInitializer());
    }

    SECTION("float const factor = 2.5;") {
        Parser parser = makeParser("float const factor = 2.5;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "factor");
        CHECK(decl->isConst());
        CHECK(decl->type()->isConst());
        CHECK(decl->type()->kind() == TypeKind::Float);
        CHECK(decl->hasInitializer());
    }
}

TEST_CASE("ConstQualifier: Architectural isolation & immutability", "[parser][decl][const][types]") {
    SECTION("Global primitive flyweight singletons are never mutated") {
        // Check baseline non-const primitive types
        CHECK_FALSE(Type::getInt()->isConst());
        CHECK_FALSE(Type::getChar()->isConst());
        CHECK_FALSE(Type::getFloat()->isConst());
        CHECK_FALSE(Type::getDouble()->isConst());
        CHECK_FALSE(Type::getVoid()->isConst());

        // Check const primitive flyweights
        CHECK(Type::getInt(true)->isConst());
        CHECK(Type::getChar(true)->isConst());
        CHECK(Type::getFloat(true)->isConst());
        CHECK(Type::getDouble(true)->isConst());
        CHECK(Type::getVoid(true)->isConst());

        // Distinct instances
        CHECK(Type::getInt(false) != Type::getInt(true));
        CHECK(*Type::getInt(false) != *Type::getInt(true));
    }

    SECTION("A const declaration does not affect subsequent non-const declarations") {
        // Parse a const declaration
        Parser p1 = makeParser("const int a = 10;");
        auto d1 = p1.parseVarDecl();
        REQUIRE(d1 != nullptr);
        CHECK(d1->isConst());
        CHECK(d1->type()->isConst());
        CHECK(d1->type()->toString() == "const int");

        // Canonical singleton must remain non-const
        CHECK_FALSE(Type::getInt()->isConst());
        CHECK(Type::getInt()->toString() == "int");

        // Parse a subsequent non-const declaration
        Parser p2 = makeParser("int b = 20;");
        auto d2 = p2.parseVarDecl();
        REQUIRE(d2 != nullptr);
        CHECK_FALSE(d2->isConst());
        CHECK_FALSE(d2->type()->isConst());
        CHECK(d2->type()->toString() == "int");

        // Verify independent type identity
        CHECK(d1->typeShared() != d2->typeShared());
        CHECK(d1->typeShared() == Type::getInt(true));
        CHECK(d2->typeShared() == Type::getInt(false));
    }

    SECTION("Non-const declaration followed by const declaration") {
        Parser p1 = makeParser("char c1;");
        auto d1 = p1.parseVarDecl();
        REQUIRE(d1 != nullptr);
        CHECK_FALSE(d1->isConst());
        CHECK_FALSE(d1->type()->isConst());

        Parser p2 = makeParser("const char c2;");
        auto d2 = p2.parseVarDecl();
        REQUIRE(d2 != nullptr);
        CHECK(d2->isConst());
        CHECK(d2->type()->isConst());

        // d1 must still be non-const
        CHECK_FALSE(d1->isConst());
        CHECK_FALSE(d1->type()->isConst());
    }

    SECTION("Repeated const declarations share canonical const type") {
        Parser p1 = makeParser("const int x;");
        auto d1 = p1.parseVarDecl();

        Parser p2 = makeParser("const int y = 5;");
        auto d2 = p2.parseVarDecl();

        REQUIRE(d1 != nullptr);
        REQUIRE(d2 != nullptr);
        CHECK(d1->typeShared() == d2->typeShared());
        CHECK(d1->typeShared() == Type::getInt(true));
    }
}

TEST_CASE("ConstQualifier: ASTPrinter output verification", "[parser][decl][const][printer]") {
    ASTPrinter printer(2);

    SECTION("Uninitialized const declaration") {
        Parser parser = makeParser("const int x;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);

        std::string expected = "VarDecl: const int x\n";
        CHECK(printer.print(decl.get()) == expected);
        CHECK(decl->toString() == "VarDecl: const int x");
    }

    SECTION("Initialized const declaration with literal") {
        Parser parser = makeParser("const int y = 10;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);

        std::string expected =
            "VarDecl: const int y\n"
            "  Initializer:\n"
            "    LiteralExpression (Integer): 10\n";
        CHECK(printer.print(decl.get()) == expected);
    }

    SECTION("Initialized const declaration with binary expression") {
        Parser parser = makeParser("const int total = a + b;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);

        std::string expected =
            "VarDecl: const int total\n"
            "  Initializer:\n"
            "    BinaryExpr: +\n"
            "      Left:\n"
            "        IdentifierExpression: a\n"
            "      Right:\n"
            "        IdentifierExpression: b\n";
        CHECK(printer.print(decl.get()) == expected);
    }

    SECTION("Ordinary non-const declaration output remains standard") {
        Parser parser = makeParser("int z = 42;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);

        std::string expected =
            "VarDecl: int z\n"
            "  Initializer:\n"
            "    LiteralExpression (Integer): 42\n";
        CHECK(printer.print(decl.get()) == expected);
    }
}

TEST_CASE("ConstQualifier: Error handling and diagnostics", "[parser][decl][const][errors]") {
    SECTION("Missing type specifier after const: 'const;'") {
        Parser parser = makeParser("const;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected type specifier after 'const'") != std::string::npos);
    }

    SECTION("Missing type specifier before identifier: 'const x;'") {
        Parser parser = makeParser("const x;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected type specifier after 'const'") != std::string::npos);
    }

    SECTION("Missing identifier after const and type: 'const int;'") {
        Parser parser = makeParser("const int;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected identifier after type specifier") != std::string::npos);
    }

    SECTION("Missing semicolon: 'const int x = 10'") {
        Parser parser = makeParser("const int x = 10");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';' after variable declaration") != std::string::npos);
    }

    SECTION("Missing expression after '=': 'const int x = ;'") {
        Parser parser = makeParser("const int x = ;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression after '='") != std::string::npos);
    }

    SECTION("Invalid keyword after const: 'const return;'") {
        Parser parser = makeParser("const return;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected type specifier after 'const'") != std::string::npos);
    }
}

TEST_CASE("ConstQualifier: Non-confusion and regression", "[parser][decl][const][regression]") {
    SECTION("Function declaration parser does not consume const variable declaration") {
        Parser parser = makeParser("const int x = 10;");
        auto fn = parser.parseDeclaration();
        CHECK(fn == nullptr);
        CHECK_FALSE(parser.hasError());
        // Verify token stream untouched at 'const'
        CHECK(parser.peek().lexeme == "const");
    }

    SECTION("Non-const declarations still parse cleanly") {
        Parser parser = makeParser("int x; int y = 20;");
        auto d1 = parser.parseVarDecl();
        REQUIRE(d1 != nullptr);
        CHECK_FALSE(d1->isConst());
        CHECK(d1->name() == "x");

        auto d2 = parser.parseVarDecl();
        REQUIRE(d2 != nullptr);
        CHECK_FALSE(d2->isConst());
        CHECK(d2->name() == "y");
    }
}
