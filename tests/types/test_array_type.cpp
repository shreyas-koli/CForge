#include <catch2/catch.hpp>

#include "types/type.h"

// =============================================================================
// DAY 54 TESTS — C Fixed-Size Array Types
// =============================================================================

TEST_CASE("ArrayType: Construction, identification, and category queries", "[types][array]") {
    SECTION("Construct via ArrayType direct constructor") {
        auto intType = Type::getInt();
        auto arr = std::make_shared<ArrayType>(intType, 10);
        REQUIRE(arr != nullptr);

        // Kind queries
        CHECK(arr->kind() == TypeKind::Array);
        CHECK(arr->getKind() == TypeKind::Array);
        CHECK(arr->isArray());

        // Category queries
        CHECK_FALSE(arr->isPrimitive());
        CHECK_FALSE(arr->isInteger());
        CHECK_FALSE(arr->isFloatingPoint());
        CHECK_FALSE(arr->isArithmetic());
        CHECK_FALSE(arr->isVoid());
        CHECK_FALSE(arr->isChar());
        CHECK_FALSE(arr->isInt());
        CHECK_FALSE(arr->isFloat());
        CHECK_FALSE(arr->isDouble());
        CHECK_FALSE(arr->isUnknown());
    }

    SECTION("Construct via Type::getArray factory") {
        auto arr = Type::getArray(Type::getFloat(), 5);
        REQUIRE(arr != nullptr);

        CHECK(arr->kind() == TypeKind::Array);
        CHECK(arr->isArray());
        CHECK_FALSE(arr->isPrimitive());
    }
}

TEST_CASE("ArrayType: Element type retrieval", "[types][array]") {
    SECTION("Integer element type") {
        auto arr = Type::getArray(Type::getInt(), 10);
        REQUIRE(arr->elementType() != nullptr);
        CHECK(arr->elementType()->kind() == TypeKind::Int);
        CHECK(arr->elementType()->isInt());
        CHECK(arr->elementTypeShared() == Type::getInt());
        CHECK(arr->getElementType() == Type::getInt());
    }

    SECTION("Float element type") {
        auto arr = Type::getArray(Type::getFloat(), 5);
        REQUIRE(arr->elementType() != nullptr);
        CHECK(arr->elementType()->kind() == TypeKind::Float);
        CHECK(arr->elementType()->isFloat());
        CHECK(arr->elementTypeShared() == Type::getFloat());
    }

    SECTION("Char element type") {
        auto arr = Type::getArray(Type::getChar(), 20);
        REQUIRE(arr->elementType() != nullptr);
        CHECK(arr->elementType()->kind() == TypeKind::Char);
        CHECK(arr->elementType()->isChar());
        CHECK(arr->elementTypeShared() == Type::getChar());
    }

    SECTION("Double element type") {
        auto arr = Type::getArray(Type::getDouble(), 50);
        REQUIRE(arr->elementType() != nullptr);
        CHECK(arr->elementType()->kind() == TypeKind::Double);
        CHECK(arr->elementType()->isDouble());
        CHECK(arr->elementTypeShared() == Type::getDouble());
    }

    SECTION("Const-qualified element type preserves constness") {
        auto constInt = Type::getInt(true);
        auto arr = Type::getArray(constInt, 8);
        REQUIRE(arr->elementType() != nullptr);
        CHECK(arr->elementType()->kind() == TypeKind::Int);
        CHECK(arr->elementType()->isConst());
        CHECK(arr->isConst());
        CHECK(arr->isQualified());
    }
}

TEST_CASE("ArrayType: Array size and dimension retrieval", "[types][array]") {
    SECTION("Array dimension accessors") {
        auto arr = Type::getArray(Type::getInt(), 10);
        CHECK(arr->arraySize() == 10);
        CHECK(arr->getArraySize() == 10);
        CHECK(arr->elementCount() == 10);
        CHECK(arr->getElementCount() == 10);
        CHECK(arr->numElements() == 10);
        CHECK(arr->length() == 10);
    }

    SECTION("Size in bytes for primitive elements") {
        // int: 4 bytes each -> 10 * 4 = 40 bytes
        auto arrInt = Type::getArray(Type::getInt(), 10);
        CHECK(arrInt->sizeInBytes() == 40);
        CHECK(arrInt->size() == 40);

        // char: 1 byte each -> 20 * 1 = 20 bytes
        auto arrChar = Type::getArray(Type::getChar(), 20);
        CHECK(arrChar->sizeInBytes() == 20);
        CHECK(arrChar->size() == 20);

        // float: 4 bytes each -> 5 * 4 = 20 bytes
        auto arrFloat = Type::getArray(Type::getFloat(), 5);
        CHECK(arrFloat->sizeInBytes() == 20);
        CHECK(arrFloat->size() == 20);

        // double: 8 bytes each -> 8 * 8 = 64 bytes
        auto arrDouble = Type::getArray(Type::getDouble(), 8);
        CHECK(arrDouble->sizeInBytes() == 64);
        CHECK(arrDouble->size() == 64);
    }

    SECTION("Single-element array") {
        auto arrOne = Type::getArray(Type::getInt(), 1);
        CHECK(arrOne->arraySize() == 1);
        CHECK(arrOne->sizeInBytes() == 4);
        CHECK(arrOne->size() == 4);
    }
}

TEST_CASE("ArrayType: Type string formatting", "[types][array]") {
    SECTION("Primitive element types") {
        CHECK(Type::getArray(Type::getInt(), 10)->toString() == "int[10]");
        CHECK(std::string(Type::getArray(Type::getInt(), 10)->name()) == "int[10]");

        CHECK(Type::getArray(Type::getFloat(), 5)->toString() == "float[5]");
        CHECK(std::string(Type::getArray(Type::getFloat(), 5)->name()) == "float[5]");

        CHECK(Type::getArray(Type::getChar(), 20)->toString() == "char[20]");
        CHECK(std::string(Type::getArray(Type::getChar(), 20)->name()) == "char[20]");

        CHECK(Type::getArray(Type::getDouble(), 100)->toString() == "double[100]");
        CHECK(std::string(Type::getArray(Type::getDouble(), 100)->name()) == "double[100]");
    }

    SECTION("Qualified element types") {
        auto constInt = Type::getInt(true);
        CHECK(Type::getArray(constInt, 8)->toString() == "const int[8]");
        CHECK(std::string(Type::getArray(constInt, 8)->name()) == "const int[8]");

        auto volChar = Type::getChar(false, true);
        CHECK(Type::getArray(volChar, 16)->toString() == "volatile char[16]");

        auto constVolDouble = Type::getDouble(true, true);
        CHECK(Type::getArray(constVolDouble, 4)->toString() == "const volatile double[4]");
    }
}

TEST_CASE("ArrayType: Structural equality and inequality", "[types][array]") {
    auto int10_a = Type::getArray(Type::getInt(), 10);
    auto int10_b = Type::getArray(Type::getInt(), 10);
    auto int5    = Type::getArray(Type::getInt(), 5);
    auto float10 = Type::getArray(Type::getFloat(), 10);
    auto char10  = Type::getArray(Type::getChar(), 10);
    auto constInt10 = Type::getArray(Type::getInt(true), 10);

    SECTION("Identical element type and identical size are equal") {
        CHECK(*int10_a == *int10_b);
        CHECK_FALSE(*int10_a != *int10_b);
        CHECK(int10_a->equals(*int10_b));
    }

    SECTION("Different sizes with same element type are unequal") {
        CHECK(*int10_a != *int5);
        CHECK_FALSE(*int10_a == *int5);
        CHECK_FALSE(int10_a->equals(*int5));
    }

    SECTION("Different element types with identical size are unequal") {
        CHECK(*int10_a != *float10);
        CHECK(*int10_a != *char10);
        CHECK_FALSE(*int10_a == *float10);
        CHECK_FALSE(*int10_a == *char10);
    }

    SECTION("Const-qualified element vs unqualified element are unequal") {
        CHECK(*int10_a != *constInt10);
        CHECK_FALSE(*int10_a == *constInt10);
    }

    SECTION("Array type compared with primitive type is unequal") {
        CHECK(*int10_a != *Type::getInt());
        CHECK(*int10_a != *Type::getFloat());
        CHECK_FALSE(*int10_a == *Type::getInt());
    }
}

TEST_CASE("ArrayType: Qualifier transformations", "[types][array]") {
    auto arr = Type::getArray(Type::getInt(), 10);

    SECTION("withConst produces array with const element type") {
        auto constArr = arr->withConst(true);
        REQUIRE(constArr != nullptr);
        CHECK(constArr->isArray());
        CHECK(constArr->isConst());
        CHECK(constArr->toString() == "const int[10]");

        auto* concrete = dynamic_cast<const ArrayType*>(constArr.get());
        REQUIRE(concrete != nullptr);
        REQUIRE(concrete->elementType() != nullptr);
        CHECK(concrete->elementType()->isConst());
    }

    SECTION("withVolatile produces array with volatile element type") {
        auto volArr = arr->withVolatile(true);
        REQUIRE(volArr != nullptr);
        CHECK(volArr->isArray());
        CHECK(volArr->isVolatile());
        CHECK(volArr->toString() == "volatile int[10]");
    }
}
