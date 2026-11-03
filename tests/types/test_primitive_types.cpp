#include <catch2/catch.hpp>

#include "types/type.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

// =============================================================================
// DAY 36 TESTS — C Primitive Types
// =============================================================================

TEST_CASE("PrimitiveType: All five primitive types representation", "[types][primitive]") {
    SECTION("Void primitive type properties") {
        auto voidType = Type::getVoid();
        REQUIRE(voidType != nullptr);
        CHECK(voidType->kind() == TypeKind::Void);
        CHECK(voidType->getKind() == TypeKind::Void);
        CHECK(voidType->isVoid());
        CHECK_FALSE(voidType->isChar());
        CHECK_FALSE(voidType->isInt());
        CHECK_FALSE(voidType->isFloat());
        CHECK_FALSE(voidType->isDouble());
        CHECK_FALSE(voidType->isUnknown());
        CHECK(voidType->isPrimitive());
        CHECK_FALSE(voidType->isInteger());
        CHECK_FALSE(voidType->isFloatingPoint());
        CHECK_FALSE(voidType->isArithmetic());
        CHECK(voidType->size() == 0);
        CHECK(std::string(voidType->name()) == "void");
        CHECK(voidType->toString() == "void");
    }

    SECTION("Char primitive type properties") {
        auto charType = Type::getChar();
        REQUIRE(charType != nullptr);
        CHECK(charType->kind() == TypeKind::Char);
        CHECK(charType->getKind() == TypeKind::Char);
        CHECK(charType->isChar());
        CHECK_FALSE(charType->isVoid());
        CHECK_FALSE(charType->isInt());
        CHECK_FALSE(charType->isFloat());
        CHECK_FALSE(charType->isDouble());
        CHECK_FALSE(charType->isUnknown());
        CHECK(charType->isPrimitive());
        CHECK(charType->isInteger());
        CHECK_FALSE(charType->isFloatingPoint());
        CHECK(charType->isArithmetic());
        CHECK(charType->size() == 1);
        CHECK(std::string(charType->name()) == "char");
        CHECK(charType->toString() == "char");
    }

    SECTION("Int primitive type properties") {
        auto intType = Type::getInt();
        REQUIRE(intType != nullptr);
        CHECK(intType->kind() == TypeKind::Int);
        CHECK(intType->getKind() == TypeKind::Int);
        CHECK(intType->isInt());
        CHECK_FALSE(intType->isVoid());
        CHECK_FALSE(intType->isChar());
        CHECK_FALSE(intType->isFloat());
        CHECK_FALSE(intType->isDouble());
        CHECK_FALSE(intType->isUnknown());
        CHECK(intType->isPrimitive());
        CHECK(intType->isInteger());
        CHECK_FALSE(intType->isFloatingPoint());
        CHECK(intType->isArithmetic());
        CHECK(intType->size() == 4);
        CHECK(std::string(intType->name()) == "int");
        CHECK(intType->toString() == "int");
    }

    SECTION("Float primitive type properties") {
        auto floatType = Type::getFloat();
        REQUIRE(floatType != nullptr);
        CHECK(floatType->kind() == TypeKind::Float);
        CHECK(floatType->getKind() == TypeKind::Float);
        CHECK(floatType->isFloat());
        CHECK_FALSE(floatType->isVoid());
        CHECK_FALSE(floatType->isChar());
        CHECK_FALSE(floatType->isInt());
        CHECK_FALSE(floatType->isDouble());
        CHECK_FALSE(floatType->isUnknown());
        CHECK(floatType->isPrimitive());
        CHECK_FALSE(floatType->isInteger());
        CHECK(floatType->isFloatingPoint());
        CHECK(floatType->isArithmetic());
        CHECK(floatType->size() == 4);
        CHECK(std::string(floatType->name()) == "float");
        CHECK(floatType->toString() == "float");
    }

    SECTION("Double primitive type properties") {
        auto doubleType = Type::getDouble();
        REQUIRE(doubleType != nullptr);
        CHECK(doubleType->kind() == TypeKind::Double);
        CHECK(doubleType->getKind() == TypeKind::Double);
        CHECK(doubleType->isDouble());
        CHECK_FALSE(doubleType->isVoid());
        CHECK_FALSE(doubleType->isChar());
        CHECK_FALSE(doubleType->isInt());
        CHECK_FALSE(doubleType->isFloat());
        CHECK_FALSE(doubleType->isUnknown());
        CHECK(doubleType->isPrimitive());
        CHECK_FALSE(doubleType->isInteger());
        CHECK(doubleType->isFloatingPoint());
        CHECK(doubleType->isArithmetic());
        CHECK(doubleType->size() == 8);
        CHECK(std::string(doubleType->name()) == "double");
        CHECK(doubleType->toString() == "double");
    }
}

TEST_CASE("PrimitiveType: Keyword to TypeKind mapping", "[types][keywords]") {
    SECTION("Valid keyword strings map to TypeKind") {
        CHECK(typeKindFromKeyword("void") == TypeKind::Void);
        CHECK(typeKindFromKeyword("char") == TypeKind::Char);
        CHECK(typeKindFromKeyword("int") == TypeKind::Int);
        CHECK(typeKindFromKeyword("float") == TypeKind::Float);
        CHECK(typeKindFromKeyword("double") == TypeKind::Double);
    }

    SECTION("isPrimitiveTypeKeyword recognition") {
        CHECK(isPrimitiveTypeKeyword("void"));
        CHECK(isPrimitiveTypeKeyword("char"));
        CHECK(isPrimitiveTypeKeyword("int"));
        CHECK(isPrimitiveTypeKeyword("float"));
        CHECK(isPrimitiveTypeKeyword("double"));
    }

    SECTION("Valid Token mapping to TypeKind") {
        Token tokVoid{TokenType::Keyword, "void", 1, 1};
        Token tokChar{TokenType::Keyword, "char", 1, 6};
        Token tokInt{TokenType::Keyword, "int", 1, 11};
        Token tokFloat{TokenType::Keyword, "float", 1, 15};
        Token tokDouble{TokenType::Keyword, "double", 1, 21};

        CHECK(isPrimitiveTypeToken(tokVoid));
        CHECK(isPrimitiveTypeToken(tokChar));
        CHECK(isPrimitiveTypeToken(tokInt));
        CHECK(isPrimitiveTypeToken(tokFloat));
        CHECK(isPrimitiveTypeToken(tokDouble));

        CHECK(typeKindFromToken(tokVoid) == TypeKind::Void);
        CHECK(typeKindFromToken(tokChar) == TypeKind::Char);
        CHECK(typeKindFromToken(tokInt) == TypeKind::Int);
        CHECK(typeKindFromToken(tokFloat) == TypeKind::Float);
        CHECK(typeKindFromToken(tokDouble) == TypeKind::Double);
    }

    SECTION("Factory from keyword and token") {
        auto tVoid = primitiveTypeFromKeyword("void");
        REQUIRE(tVoid != nullptr);
        CHECK(tVoid->isVoid());

        auto tInt = Type::fromKeyword("int");
        REQUIRE(tInt != nullptr);
        CHECK(tInt->isInt());

        Token tokDouble{TokenType::Keyword, "double", 1, 1};
        auto tDouble = Type::fromToken(tokDouble);
        REQUIRE(tDouble != nullptr);
        CHECK(tDouble->isDouble());

        auto tKindDouble = getPrimitiveType(TypeKind::Double);
        CHECK(tDouble == tKindDouble);
    }
}

TEST_CASE("PrimitiveType: Type names and string conversions", "[types][names]") {
    CHECK(std::string(typeKindName(TypeKind::Void)) == "void");
    CHECK(std::string(typeKindName(TypeKind::Char)) == "char");
    CHECK(std::string(typeKindName(TypeKind::Int)) == "int");
    CHECK(std::string(typeKindName(TypeKind::Float)) == "float");
    CHECK(std::string(typeKindName(TypeKind::Double)) == "double");
    CHECK(std::string(typeKindName(TypeKind::Unknown)) == "<unknown>");

    CHECK(typeKindToString(TypeKind::Void) == "void");
    CHECK(typeKindToString(TypeKind::Char) == "char");
    CHECK(typeKindToString(TypeKind::Int) == "int");
    CHECK(typeKindToString(TypeKind::Float) == "float");
    CHECK(typeKindToString(TypeKind::Double) == "double");
}

TEST_CASE("PrimitiveType: Invalid and unsupported type lookups", "[types][invalid]") {
    SECTION("Other C keywords are not primitive types") {
        CHECK_FALSE(isPrimitiveTypeKeyword("return"));
        CHECK_FALSE(isPrimitiveTypeKeyword("if"));
        CHECK_FALSE(isPrimitiveTypeKeyword("else"));
        CHECK_FALSE(isPrimitiveTypeKeyword("while"));
        CHECK_FALSE(isPrimitiveTypeKeyword("for"));
        CHECK_FALSE(isPrimitiveTypeKeyword("break"));
        CHECK_FALSE(isPrimitiveTypeKeyword("continue"));
        CHECK_FALSE(isPrimitiveTypeKeyword("do"));

        CHECK(typeKindFromKeyword("return") == TypeKind::Unknown);
        CHECK(typeKindFromKeyword("while") == TypeKind::Unknown);
        CHECK(primitiveTypeFromKeyword("return") == nullptr);
    }

    SECTION("Unsupported and future types return Unknown and nullptr") {
        CHECK_FALSE(isPrimitiveTypeKeyword("short"));
        CHECK_FALSE(isPrimitiveTypeKeyword("long"));
        CHECK_FALSE(isPrimitiveTypeKeyword("unsigned"));
        CHECK_FALSE(isPrimitiveTypeKeyword("signed"));
        CHECK_FALSE(isPrimitiveTypeKeyword("struct"));
        CHECK_FALSE(isPrimitiveTypeKeyword("union"));
        CHECK_FALSE(isPrimitiveTypeKeyword("enum"));
        CHECK_FALSE(isPrimitiveTypeKeyword("typedef"));
        CHECK_FALSE(isPrimitiveTypeKeyword("bool"));

        CHECK(typeKindFromKeyword("struct") == TypeKind::Unknown);
        CHECK(typeKindFromKeyword("typedef") == TypeKind::Unknown);
        CHECK(primitiveTypeFromKeyword("struct") == nullptr);
    }

    SECTION("Arbitrary identifiers and invalid strings return Unknown") {
        CHECK_FALSE(isPrimitiveTypeKeyword(""));
        CHECK_FALSE(isPrimitiveTypeKeyword("my_int"));
        CHECK_FALSE(isPrimitiveTypeKeyword("Integer"));
        CHECK_FALSE(isPrimitiveTypeKeyword("INT"));
        CHECK_FALSE(isPrimitiveTypeKeyword("Void"));
        CHECK_FALSE(isPrimitiveTypeKeyword("char*"));
        CHECK_FALSE(isPrimitiveTypeKeyword("int[]"));

        CHECK(typeKindFromKeyword("my_type") == TypeKind::Unknown);
        CHECK(primitiveTypeFromKeyword("my_type") == nullptr);
    }

    SECTION("Non-keyword tokens are rejected even with primitive lexemes") {
        Token identInt{TokenType::Identifier, "int", 1, 1};
        CHECK_FALSE(isPrimitiveTypeToken(identInt));
        CHECK(typeKindFromToken(identInt) == TypeKind::Unknown);
        CHECK(primitiveTypeFromToken(identInt) == nullptr);

        Token punctSemi{TokenType::Punctuation, ";", 1, 5};
        CHECK_FALSE(isPrimitiveTypeToken(punctSemi));
        CHECK(typeKindFromToken(punctSemi) == TypeKind::Unknown);

        Token numLiteral{TokenType::Integer, "42", 1, 1};
        CHECK_FALSE(isPrimitiveTypeToken(numLiteral));
    }
}

TEST_CASE("PrimitiveType: Equality and comparison", "[types][equality]") {
    SECTION("Flyweight singleton instances share identity") {
        CHECK(Type::getInt() == Type::getInt());
        CHECK(Type::getChar() == Type::getChar());
        CHECK(Type::getFloat() == Type::getFloat());
        CHECK(Type::getDouble() == Type::getDouble());
        CHECK(Type::getVoid() == Type::getVoid());

        CHECK(*Type::getInt() == *Type::getInt());
        CHECK_FALSE(*Type::getInt() != *Type::getInt());
    }

    SECTION("Distinct primitive types are not equal") {
        CHECK(Type::getInt() != Type::getChar());
        CHECK(Type::getInt() != Type::getFloat());
        CHECK(Type::getInt() != Type::getDouble());
        CHECK(Type::getInt() != Type::getVoid());

        CHECK(*Type::getInt() != *Type::getChar());
        CHECK(*Type::getInt() != *Type::getFloat());
        CHECK(*Type::getInt() != *Type::getDouble());
        CHECK(*Type::getInt() != *Type::getVoid());

        CHECK(*Type::getFloat() != *Type::getDouble());
        CHECK(*Type::getChar() != *Type::getVoid());
    }

    SECTION("Independent PrimitiveType instances compare by value") {
        PrimitiveType a(TypeKind::Int);
        PrimitiveType b(TypeKind::Int);
        PrimitiveType c(TypeKind::Float);

        CHECK(a == b);
        CHECK(a.equals(b));
        CHECK_FALSE(a != b);

        CHECK(a != c);
        CHECK_FALSE(a == c);
        CHECK_FALSE(a.equals(c));
    }
}

TEST_CASE("PrimitiveType: Integration with Lexer and Parser", "[types][integration]") {
    SECTION("Lexer token stream recognizes primitive type tokens") {
        const std::string src = "int a; char b; float c; double d; void f();";
        Lexer lexer(src);
        auto tokens = lexer.tokenize();

        REQUIRE(tokens.size() >= 13);

        // int
        CHECK(isPrimitiveTypeToken(tokens[0]));
        CHECK(typeKindFromToken(tokens[0]) == TypeKind::Int);

        // a (identifier)
        CHECK_FALSE(isPrimitiveTypeToken(tokens[1]));

        // ; (punctuation)
        CHECK_FALSE(isPrimitiveTypeToken(tokens[2]));

        // char
        CHECK(isPrimitiveTypeToken(tokens[3]));
        CHECK(typeKindFromToken(tokens[3]) == TypeKind::Char);

        // float
        CHECK(isPrimitiveTypeToken(tokens[6]));
        CHECK(typeKindFromToken(tokens[6]) == TypeKind::Float);

        // double
        CHECK(isPrimitiveTypeToken(tokens[9]));
        CHECK(typeKindFromToken(tokens[9]) == TypeKind::Double);

        // void
        CHECK(isPrimitiveTypeToken(tokens[12]));
        CHECK(typeKindFromToken(tokens[12]) == TypeKind::Void);
    }

    SECTION("Parser continues parsing functions with all primitive types") {
        {
            Lexer lexer("int compute(int x);");
            Parser parser(lexer.tokenize());
            auto fn = parser.parseFunctionDeclaration();
            REQUIRE(fn != nullptr);
            CHECK(fn->returnType() == "int");
            CHECK(isPrimitiveTypeKeyword(fn->returnType()));
            CHECK(typeKindFromKeyword(fn->returnType()) == TypeKind::Int);
            REQUIRE(fn->parameterCount() == 1);
            CHECK(fn->parameters()[0]->type() == "int");
        }
        {
            Lexer lexer("char getChar(void);");
            Parser parser(lexer.tokenize());
            auto fn = parser.parseFunctionDeclaration();
            REQUIRE(fn != nullptr);
            CHECK(fn->returnType() == "char");
            CHECK(typeKindFromKeyword(fn->returnType()) == TypeKind::Char);
            CHECK(fn->parameterCount() == 0);
        }
        {
            Lexer lexer("float getFloat();");
            Parser parser(lexer.tokenize());
            auto fn = parser.parseFunctionDeclaration();
            REQUIRE(fn != nullptr);
            CHECK(fn->returnType() == "float");
            CHECK(typeKindFromKeyword(fn->returnType()) == TypeKind::Float);
        }
        {
            Lexer lexer("double getDouble();");
            Parser parser(lexer.tokenize());
            auto fn = parser.parseFunctionDeclaration();
            REQUIRE(fn != nullptr);
            CHECK(fn->returnType() == "double");
            CHECK(typeKindFromKeyword(fn->returnType()) == TypeKind::Double);
        }
        {
            Lexer lexer("void doAction(void);");
            Parser parser(lexer.tokenize());
            auto fn = parser.parseFunctionDeclaration();
            REQUIRE(fn != nullptr);
            CHECK(fn->returnType() == "void");
            CHECK(typeKindFromKeyword(fn->returnType()) == TypeKind::Void);
        }
    }
}
