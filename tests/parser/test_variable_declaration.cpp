#include <catch2/catch.hpp>

#include "ast/ast.h"
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
// DAY 37 TESTS — Variable Declarations
// =============================================================================

TEST_CASE("VarDecl: Primitive type declarations without initializer", "[parser][decl][variable]") {
    SECTION("int x;") {
        Parser parser = makeParser("int x;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "x");
        CHECK(decl->getName() == "x");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Int);
        CHECK(decl->typeKind() == TypeKind::Int);
        CHECK(decl->type()->isInteger());
        CHECK_FALSE(decl->hasInitializer());
        CHECK(decl->initializer() == nullptr);
        CHECK(decl->getInitializer() == nullptr);
        CHECK(decl->toString() == "VarDecl: int x");
    }

    SECTION("char letter;") {
        Parser parser = makeParser("char letter;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "letter");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Char);
        CHECK(decl->typeKind() == TypeKind::Char);
        CHECK_FALSE(decl->hasInitializer());
        CHECK(decl->toString() == "VarDecl: char letter");
    }

    SECTION("float price;") {
        Parser parser = makeParser("float price;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "price");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Float);
        CHECK(decl->typeKind() == TypeKind::Float);
        CHECK_FALSE(decl->hasInitializer());
        CHECK(decl->toString() == "VarDecl: float price");
    }

    SECTION("double value;") {
        Parser parser = makeParser("double value;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "value");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Double);
        CHECK(decl->typeKind() == TypeKind::Double);
        CHECK_FALSE(decl->hasInitializer());
        CHECK(decl->toString() == "VarDecl: double value");
    }

    SECTION("void placeholder;") {
        Parser parser = makeParser("void placeholder;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "placeholder");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Void);
        CHECK(decl->typeKind() == TypeKind::Void);
        CHECK_FALSE(decl->hasInitializer());
        CHECK(decl->toString() == "VarDecl: void placeholder");
    }
}

TEST_CASE("VarDecl: Primitive type declarations with initializer", "[parser][decl][variable]") {
    SECTION("int y = 10;") {
        Parser parser = makeParser("int y = 10;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "y");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Int);
        REQUIRE(decl->hasInitializer());
        REQUIRE(decl->initializer() != nullptr);

        auto* lit = dynamic_cast<const LiteralExpression*>(decl->initializer());
        REQUIRE(lit != nullptr);
        CHECK(lit->value() == "10");
        CHECK(lit->kind() == LiteralKind::Integer);
    }

    SECTION("char letter = 'A';") {
        Parser parser = makeParser("char letter = 'A';");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "letter");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Char);
        REQUIRE(decl->hasInitializer());

        auto* lit = dynamic_cast<const LiteralExpression*>(decl->initializer());
        REQUIRE(lit != nullptr);
        CHECK(lit->value() == "'A'");
        CHECK(lit->kind() == LiteralKind::Char);
    }

    SECTION("float price = 10.5;") {
        Parser parser = makeParser("float price = 10.5;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "price");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Float);
        REQUIRE(decl->hasInitializer());

        auto* lit = dynamic_cast<const LiteralExpression*>(decl->initializer());
        REQUIRE(lit != nullptr);
        CHECK(lit->value() == "10.5");
        CHECK(lit->kind() == LiteralKind::Float);
    }

    SECTION("double value = 3.14;") {
        Parser parser = makeParser("double value = 3.14;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "value");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Double);
        REQUIRE(decl->hasInitializer());

        auto* lit = dynamic_cast<const LiteralExpression*>(decl->initializer());
        REQUIRE(lit != nullptr);
        CHECK(lit->value() == "3.14");
        CHECK(lit->kind() == LiteralKind::Float);
    }
}

TEST_CASE("VarDecl: Diverse initializer expression types", "[parser][decl][initializer]") {
    SECTION("Identifier expression initializer: int copy = original;") {
        Parser parser = makeParser("int copy = original;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(decl->hasInitializer());
        auto* id = dynamic_cast<const IdentifierExpression*>(decl->initializer());
        REQUIRE(id != nullptr);
        CHECK(id->name() == "original");
    }

    SECTION("Unary expression initializer: int neg = -count;") {
        Parser parser = makeParser("int neg = -count;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(decl->hasInitializer());
        auto* un = dynamic_cast<const UnaryExpr*>(decl->initializer());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "-");
    }

    SECTION("Binary arithmetic initializer: int sum = a + b * 2;") {
        Parser parser = makeParser("int sum = a + b * 2;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(decl->hasInitializer());
        auto* bin = dynamic_cast<const BinaryExpr*>(decl->initializer());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");
    }

    SECTION("Conditional expression initializer: int max = a > b ? a : b;") {
        Parser parser = makeParser("int max = a > b ? a : b;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(decl->hasInitializer());
        auto* cond = dynamic_cast<const ConditionalExpr*>(decl->initializer());
        REQUIRE(cond != nullptr);
    }

    SECTION("Parenthesized assignment expression initializer: int k = (x = 5);") {
        Parser parser = makeParser("int k = (x = 5);");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        REQUIRE(decl->hasInitializer());
        auto* assign = dynamic_cast<const AssignExpr*>(decl->initializer());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "=");
    }
}

TEST_CASE("VarDecl: Accessors, mutators, and source location", "[parser][decl][ast]") {
    SECTION("Source location tracking") {
        const std::string src = "\n  int count = 42;";
        Parser parser = makeParser(src);
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->location().line == 2);
        CHECK(decl->location().column == 3);
    }

    SECTION("Initializer mutator and takeover") {
        Parser parser = makeParser("int val = 99;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK(decl->hasInitializer());

        auto taken = decl->takeInitializer();
        REQUIRE(taken != nullptr);
        CHECK_FALSE(decl->hasInitializer());
        CHECK(decl->initializer() == nullptr);

        decl->setInitializer(std::move(taken));
        CHECK(decl->hasInitializer());
        CHECK(decl->initializer() != nullptr);
    }

    SECTION("Type shared_ptr accessors") {
        Parser parser = makeParser("float temp = 98.6;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);

        CHECK(decl->typeShared() == Type::getFloat());
        CHECK(decl->getType() == Type::getFloat());
        CHECK(decl->type() == Type::getFloat().get());
    }
}

TEST_CASE("VarDecl: ASTPrinter output verification", "[parser][decl][printer]") {
    ASTPrinter printer(2);

    SECTION("Uninitialized variable declaration") {
        Parser parser = makeParser("int total;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);

        std::string expected = "VarDecl: int total\n";
        CHECK(printer.print(decl.get()) == expected);
    }

    SECTION("Initialized variable declaration") {
        Parser parser = makeParser("int counter = 0;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);

        std::string expected =
            "VarDecl: int counter\n"
            "  Initializer:\n"
            "    LiteralExpression (Integer): 0\n";
        CHECK(printer.print(decl.get()) == expected);
    }

    SECTION("Initialized with binary expression") {
        Parser parser = makeParser("int total = a + b;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);

        std::string expected =
            "VarDecl: int total\n"
            "  Initializer:\n"
            "    BinaryExpr: +\n"
            "      Left:\n"
            "        IdentifierExpression: a\n"
            "      Right:\n"
            "        IdentifierExpression: b\n";
        CHECK(printer.print(decl.get()) == expected);
    }
}

TEST_CASE("VarDecl: Error handling and diagnostics", "[parser][decl][errors]") {
    SECTION("Missing identifier: 'int;'") {
        Parser parser = makeParser("int;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected identifier after type specifier") != std::string::npos);
    }

    SECTION("Missing identifier with equal sign: 'int = 10;'") {
        Parser parser = makeParser("int = 10;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected identifier after type specifier") != std::string::npos);
    }

    SECTION("Missing identifier at EOF: 'int'") {
        Parser parser = makeParser("int");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected identifier after type specifier") != std::string::npos);
    }

    SECTION("Missing initializer after '=': 'int x = ;'") {
        Parser parser = makeParser("int x = ;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression after '='") != std::string::npos);
    }

    SECTION("Missing initializer at EOF: 'int x ='") {
        Parser parser = makeParser("int x =");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression after '='") != std::string::npos);
    }

    SECTION("Missing semicolon: 'int x'") {
        Parser parser = makeParser("int x");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';' after variable declaration") != std::string::npos);
    }

    SECTION("Missing semicolon with initializer: 'int x = 10'") {
        Parser parser = makeParser("int x = 10");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';' after variable declaration") != std::string::npos);
    }

    SECTION("Missing semicolon between declarations: 'int x int y;'") {
        Parser parser = makeParser("int x int y;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';' after variable declaration") != std::string::npos);
    }

    SECTION("Invalid type specifier: non-type token 'return x;'") {
        Parser parser = makeParser("return x;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected type specifier") != std::string::npos);
    }

    SECTION("Invalid type specifier: identifier 'my_type x;'") {
        Parser parser = makeParser("my_type x;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected type specifier") != std::string::npos);
    }

    SECTION("Graceful termination on empty source") {
        Parser parser = makeParser("");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
    }
}

TEST_CASE("VarDecl: Regression and non-confusion tests", "[parser][decl][regression]") {
    SECTION("Function declaration is not parsed by parseVarDecl") {
        // parseVarDecl on 'int add(int a, int b);' fails at '(' because '(' is not an identifier or ';'
        Parser parser = makeParser("int add(int a, int b);");
        auto decl = parser.parseVarDecl();
        // It consumed 'int' then saw 'add' (identifier), then expected '=' or ';', but found '('
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
    }

    SECTION("Existing parseDeclaration still distinguishes functions from variable declarations") {
        Parser parser = makeParser("int x = 10;");
        auto fn = parser.parseDeclaration();
        CHECK(fn == nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.peek().lexeme == "int");

        // Now parseVarDecl cleanly consumes it
        auto var = parser.parseVarDecl();
        REQUIRE(var != nullptr);
        CHECK(var->name() == "x");
        CHECK(parser.isAtEnd());
    }

    SECTION("parseFunctionDeclaration still parses functions accurately") {
        Parser parser = makeParser("int add(int a, int b);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(fn->name() == "add");
        CHECK(fn->returnType() == "int");
    }

    SECTION("Assignment expressions parsed separately from declarations") {
        Parser parser = makeParser("x = 10;");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        auto* assign = dynamic_cast<const AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "=");
    }
}
