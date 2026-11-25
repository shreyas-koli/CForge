#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/var_decl.h"
#include "ast/compound_stmt.h"
#include "ast/function.h"
#include "ast/literal_expression.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "types/type.h"

static Parser makeParser(const std::string& src) {
    Lexer lexer(src);
    return Parser(lexer.tokenize());
}

// =============================================================================
// DAY 54 TESTS — Fixed-Size Array Declarations
// =============================================================================

TEST_CASE("ArrayDecl: Valid fixed-size array declarations", "[parser][decl][array]") {
    SECTION("int numbers[10];") {
        Parser parser = makeParser("int numbers[10];");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "numbers");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Array);
        CHECK(decl->type()->isArray());
        CHECK_FALSE(decl->hasInitializer());

        auto* arrType = dynamic_cast<const ArrayType*>(decl->type());
        REQUIRE(arrType != nullptr);
        REQUIRE(arrType->elementType() != nullptr);
        CHECK(arrType->elementType()->kind() == TypeKind::Int);
        CHECK(arrType->elementType()->isInt());
        CHECK(arrType->arraySize() == 10);
        CHECK(arrType->sizeInBytes() == 40);
        CHECK(arrType->toString() == "int[10]");
        CHECK(decl->toString() == "VarDecl: int[10] numbers");
    }

    SECTION("float prices[5];") {
        Parser parser = makeParser("float prices[5];");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "prices");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Array);

        auto* arrType = dynamic_cast<const ArrayType*>(decl->type());
        REQUIRE(arrType != nullptr);
        REQUIRE(arrType->elementType() != nullptr);
        CHECK(arrType->elementType()->kind() == TypeKind::Float);
        CHECK(arrType->elementType()->isFloat());
        CHECK(arrType->arraySize() == 5);
        CHECK(arrType->sizeInBytes() == 20);
        CHECK(arrType->toString() == "float[5]");
        CHECK(decl->toString() == "VarDecl: float[5] prices");
    }

    SECTION("char name[20];") {
        Parser parser = makeParser("char name[20];");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "name");
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Array);

        auto* arrType = dynamic_cast<const ArrayType*>(decl->type());
        REQUIRE(arrType != nullptr);
        REQUIRE(arrType->elementType() != nullptr);
        CHECK(arrType->elementType()->kind() == TypeKind::Char);
        CHECK(arrType->elementType()->isChar());
        CHECK(arrType->arraySize() == 20);
        CHECK(arrType->sizeInBytes() == 20);
        CHECK(arrType->toString() == "char[20]");
        CHECK(decl->toString() == "VarDecl: char[20] name");
    }

    SECTION("const int values[8]; preserves element type qualifier") {
        Parser parser = makeParser("const int values[8];");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "values");
        CHECK(decl->isConst());
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Array);
        CHECK(decl->type()->isConst());

        auto* arrType = dynamic_cast<const ArrayType*>(decl->type());
        REQUIRE(arrType != nullptr);
        REQUIRE(arrType->elementType() != nullptr);
        CHECK(arrType->elementType()->kind() == TypeKind::Int);
        CHECK(arrType->elementType()->isConst());
        CHECK(arrType->arraySize() == 8);
        CHECK(arrType->sizeInBytes() == 32);
        CHECK(arrType->toString() == "const int[8]");
        CHECK(decl->toString() == "VarDecl: const int[8] values");
    }

    SECTION("static int buffer[1024];") {
        Parser parser = makeParser("static int buffer[1024];");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "buffer");
        CHECK(decl->isStatic());
        CHECK(decl->storageClass() == StorageClass::Static);
        auto* arrType = dynamic_cast<const ArrayType*>(decl->type());
        REQUIRE(arrType != nullptr);
        CHECK(arrType->arraySize() == 1024);
        CHECK(decl->toString() == "VarDecl: static int[1024] buffer");
    }

    SECTION("extern double table[50];") {
        Parser parser = makeParser("extern double table[50];");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        CHECK(decl->name() == "table");
        CHECK(decl->isExtern());
        auto* arrType = dynamic_cast<const ArrayType*>(decl->type());
        REQUIRE(arrType != nullptr);
        CHECK(arrType->elementType()->isDouble());
        CHECK(arrType->arraySize() == 50);
        CHECK(decl->toString() == "VarDecl: extern double[50] table");
    }
}

TEST_CASE("ArrayDecl: Multiple array declarations", "[parser][decl][array][multiple]") {
    SECTION("Successive array declarations") {
        Parser p1 = makeParser("int a[10];");
        auto d1 = p1.parseVarDecl();
        REQUIRE(d1 != nullptr);
        CHECK(d1->name() == "a");

        Parser p2 = makeParser("float b[20];");
        auto d2 = p2.parseVarDecl();
        REQUIRE(d2 != nullptr);
        CHECK(d2->name() == "b");

        Parser p3 = makeParser("char c[30];");
        auto d3 = p3.parseVarDecl();
        REQUIRE(d3 != nullptr);
        CHECK(d3->name() == "c");
    }

    SECTION("Multiple array declarations inside compound statement") {
        const std::string src =
            "{\n"
            "    int numbers[10];\n"
            "    float prices[5];\n"
            "    char name[20];\n"
            "    const int values[8];\n"
            "}";
        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());
        REQUIRE(block->statementCount() == 4);

        auto* d0 = dynamic_cast<const VarDecl*>(block->statements()[0].get());
        REQUIRE(d0 != nullptr);
        CHECK(d0->name() == "numbers");
        CHECK(d0->type()->toString() == "int[10]");

        auto* d1 = dynamic_cast<const VarDecl*>(block->statements()[1].get());
        REQUIRE(d1 != nullptr);
        CHECK(d1->name() == "prices");
        CHECK(d1->type()->toString() == "float[5]");

        auto* d2 = dynamic_cast<const VarDecl*>(block->statements()[2].get());
        REQUIRE(d2 != nullptr);
        CHECK(d2->name() == "name");
        CHECK(d2->type()->toString() == "char[20]");

        auto* d3 = dynamic_cast<const VarDecl*>(block->statements()[3].get());
        REQUIRE(d3 != nullptr);
        CHECK(d3->name() == "values");
        CHECK(d3->type()->toString() == "const int[8]");
        CHECK(d3->isConst());
    }

    SECTION("Nested block array declarations") {
        const std::string src =
            "{\n"
            "    int outer[10];\n"
            "    {\n"
            "        float inner[20];\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());
        REQUIRE(block->statementCount() == 2);

        auto* d0 = dynamic_cast<const VarDecl*>(block->statements()[0].get());
        REQUIRE(d0 != nullptr);
        CHECK(d0->name() == "outer");
        CHECK(d0->type()->toString() == "int[10]");

        auto* innerBlock = dynamic_cast<const CompoundStmt*>(block->statements()[1].get());
        REQUIRE(innerBlock != nullptr);
        REQUIRE(innerBlock->statementCount() == 1);
        auto* dInner = dynamic_cast<const VarDecl*>(innerBlock->statements()[0].get());
        REQUIRE(dInner != nullptr);
        CHECK(dInner->name() == "inner");
        CHECK(dInner->type()->toString() == "float[20]");
    }
}

TEST_CASE("ArrayDecl: Array declaration inside function body", "[parser][decl][array][function]") {
    SECTION("Array declaration inside function body parsed as compound statement") {
        const std::string src =
            "{\n"
            "    int data[64];\n"
            "    int total = 100;\n"
            "}";
        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());
        REQUIRE(block->statementCount() == 2);

        auto* arrDecl = dynamic_cast<const VarDecl*>(block->statements()[0].get());
        REQUIRE(arrDecl != nullptr);
        CHECK(arrDecl->name() == "data");
        CHECK(arrDecl->type()->toString() == "int[64]");
    }

    SECTION("Function definition containing array declaration parses without error") {
        const std::string src =
            "int process(int count) {\n"
            "    int temp[16];\n"
            "    return count;\n"
            "}";
        Parser parser = makeParser(src);
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(fn->name() == "process");
        CHECK(fn->isDefinition());
    }
}

TEST_CASE("ArrayDecl: Invalid array declarations rejected cleanly", "[parser][decl][array][errors]") {
    SECTION("Zero-sized array: 'int a[0];'") {
        Parser parser = makeParser("int a[0];");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("greater than zero") != std::string::npos);
    }

    SECTION("Negative array size: 'int a[-5];'") {
        Parser parser = makeParser("int a[-5];");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("positive") != std::string::npos);
    }

    SECTION("Missing array size: 'int a[];'") {
        Parser parser = makeParser("int a[];");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected array size") != std::string::npos);
    }

    SECTION("Missing closing bracket: 'int a[10;'") {
        Parser parser = makeParser("int a[10;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ']'") != std::string::npos);
    }

    SECTION("Non-integer size identifier: 'int a[size];'") {
        Parser parser = makeParser("int a[size];");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Variable-length arrays are not supported") != std::string::npos);
    }

    SECTION("Malformed declarator: float literal size 'int a[3.14];'") {
        Parser parser = makeParser("int a[3.14];");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected integer constant") != std::string::npos);
    }

    SECTION("Malformed declarator: operator without integer 'int a[+];'") {
        Parser parser = makeParser("int a[+];");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected integer constant") != std::string::npos);
    }

    SECTION("Multidimensional array: 'int a[10][20];' is rejected") {
        Parser parser = makeParser("int a[10][20];");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Multidimensional arrays are not supported") != std::string::npos);
    }

    SECTION("Array initialization: 'int a[10] = 5;' is rejected in this milestone") {
        Parser parser = makeParser("int a[10] = 5;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Array initialization is not supported") != std::string::npos);
    }

    SECTION("Missing semicolon after array declarator: 'int a[10]'") {
        Parser parser = makeParser("int a[10]");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';'") != std::string::npos);
    }
}

TEST_CASE("ArrayDecl: Regression testing - existing declaration behavior preserved", "[parser][decl][array][regression]") {
    SECTION("Scalar primitive declarations still parse") {
        Parser parser = makeParser("int count = 42;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(decl->name() == "count");
        CHECK(decl->type()->kind() == TypeKind::Int);
        CHECK(decl->hasInitializer());
    }

    SECTION("Const scalar declarations still parse") {
        Parser parser = makeParser("const float pi = 3.14;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(decl->name() == "pi");
        CHECK(decl->isConst());
    }

    SECTION("Function declarations not confused with array declarations") {
        Parser parser = makeParser("int getValue();");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(fn->name() == "getValue");
    }

    SECTION("parseDeclaration does not consume array variable declaration") {
        Parser parser = makeParser("int numbers[10];");
        auto fn = parser.parseDeclaration();
        CHECK(fn == nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.peek().lexeme == "int");

        auto var = parser.parseVarDecl();
        REQUIRE(var != nullptr);
        CHECK(var->name() == "numbers");
        CHECK(var->type()->isArray());
        CHECK(parser.isAtEnd());
    }
}
