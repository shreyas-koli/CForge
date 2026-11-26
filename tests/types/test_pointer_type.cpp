#include <catch2/catch.hpp>

#include "types/type.h"

// =============================================================================
// DAY 57 TESTS — C Pointer Types
// =============================================================================

TEST_CASE("PointerType: Construction, identification, and category queries", "[types][pointer]") {
    SECTION("Construct via PointerType direct constructor") {
        auto ptr = std::make_shared<PointerType>(Type::getInt());
        REQUIRE(ptr != nullptr);

        CHECK(ptr->kind() == TypeKind::Pointer);
        CHECK(ptr->getKind() == TypeKind::Pointer);
        CHECK(ptr->isPointer());
        CHECK(ptr->sizeInBytes() == sizeof(void*));
        CHECK(ptr->size() == sizeof(void*));

        CHECK_FALSE(ptr->isPrimitive());
        CHECK_FALSE(ptr->isArray());
        CHECK_FALSE(ptr->isInteger());
        CHECK_FALSE(ptr->isFloatingPoint());
        CHECK_FALSE(ptr->isArithmetic());
        CHECK_FALSE(ptr->isVoid());
        CHECK_FALSE(ptr->isChar());
        CHECK_FALSE(ptr->isInt());
        CHECK_FALSE(ptr->isFloat());
        CHECK_FALSE(ptr->isDouble());
        CHECK_FALSE(ptr->isUnknown());
    }

    SECTION("Construct via Type::getPointer factory") {
        auto ptr = Type::getPointer(Type::getChar());
        REQUIRE(ptr != nullptr);
        CHECK(ptr->kind() == TypeKind::Pointer);
        CHECK(ptr->isPointer());
        CHECK(typeKindToString(TypeKind::Pointer) == "pointer");
    }
}

TEST_CASE("PointerType: Pointee retrieval and recursive nesting", "[types][pointer]") {
    SECTION("Single-level pointer to primitive types") {
        auto intPtr = Type::getPointer(Type::getInt());
        REQUIRE(intPtr->pointeeType() != nullptr);
        CHECK(intPtr->pointeeType()->isInt());
        CHECK(intPtr->pointeeTypeShared() == Type::getInt());
        CHECK(intPtr->getPointeeType() == Type::getInt());
        CHECK(intPtr->toString() == "int*");
        CHECK(std::string(intPtr->name()) == "int*");

        auto charPtr = Type::getPointer(Type::getChar());
        REQUIRE(charPtr->pointeeType() != nullptr);
        CHECK(charPtr->pointeeType()->isChar());
        CHECK(charPtr->toString() == "char*");

        auto floatPtr = Type::getPointer(Type::getFloat());
        REQUIRE(floatPtr->pointeeType() != nullptr);
        CHECK(floatPtr->pointeeType()->isFloat());
        CHECK(floatPtr->toString() == "float*");

        auto doublePtr = Type::getPointer(Type::getDouble());
        REQUIRE(doublePtr->pointeeType() != nullptr);
        CHECK(doublePtr->pointeeType()->isDouble());
        CHECK(doublePtr->toString() == "double*");

        auto voidPtr = Type::getPointer(Type::getVoid());
        REQUIRE(voidPtr->pointeeType() != nullptr);
        CHECK(voidPtr->pointeeType()->isVoid());
        CHECK(voidPtr->toString() == "void*");
    }

    SECTION("Two-level pointer: int**") {
        auto level1 = Type::getPointer(Type::getInt());
        auto level2 = Type::getPointer(level1);

        REQUIRE(level2 != nullptr);
        CHECK(level2->isPointer());
        CHECK(level2->toString() == "int**");

        auto* inner = dynamic_cast<const PointerType*>(level2->pointeeType());
        REQUIRE(inner != nullptr);
        CHECK(inner->isPointer());
        CHECK(inner->toString() == "int*");
        REQUIRE(inner->pointeeType() != nullptr);
        CHECK(inner->pointeeType()->isInt());
    }

    SECTION("Three-level pointer: int***") {
        auto l1 = Type::getPointer(Type::getInt());
        auto l2 = Type::getPointer(l1);
        auto l3 = Type::getPointer(l2);

        REQUIRE(l3 != nullptr);
        CHECK(l3->isPointer());
        CHECK(l3->toString() == "int***");

        auto* p2 = dynamic_cast<const PointerType*>(l3->pointeeType());
        REQUIRE(p2 != nullptr);
        CHECK(p2->toString() == "int**");

        auto* p1 = dynamic_cast<const PointerType*>(p2->pointeeType());
        REQUIRE(p1 != nullptr);
        CHECK(p1->toString() == "int*");
        REQUIRE(p1->pointeeType() != nullptr);
        CHECK(p1->pointeeType()->isInt());
    }
}

TEST_CASE("PointerType: Structural equality and qualifiers", "[types][pointer]") {
    auto intPtrA  = Type::getPointer(Type::getInt());
    auto intPtrB  = Type::getPointer(Type::getInt());
    auto charPtr  = Type::getPointer(Type::getChar());
    auto intPtr2  = Type::getPointer(intPtrA);
    auto intPtr3  = Type::getPointer(intPtr2);
    auto ptrToConstInt = Type::getPointer(Type::getInt(true));
    auto constPtrToInt = Type::getPointer(Type::getInt(), true, false);

    SECTION("Identical pointer types are structurally equal") {
        CHECK(*intPtrA == *intPtrB);
        CHECK(intPtrA->equals(*intPtrB));
        CHECK(*intPtr2 == *Type::getPointer(Type::getPointer(Type::getInt())));
    }

    SECTION("Different pointee types and indirection depths are distinct") {
        CHECK(*intPtrA != *charPtr);
        CHECK(*intPtrA != *intPtr2);
        CHECK(*intPtr2 != *intPtr3);
        CHECK(*intPtrA != *Type::getInt());
        CHECK(*intPtrA != *Type::getArray(Type::getInt(), 4));
    }

    SECTION("Pointee const vs top-level pointer const are distinct") {
        CHECK_FALSE(ptrToConstInt->isConst());
        REQUIRE(ptrToConstInt->pointeeType() != nullptr);
        CHECK(ptrToConstInt->pointeeType()->isConst());
        CHECK(ptrToConstInt->toString() == "const int*");

        CHECK(constPtrToInt->isConst());
        REQUIRE(constPtrToInt->pointeeType() != nullptr);
        CHECK_FALSE(constPtrToInt->pointeeType()->isConst());
        CHECK(constPtrToInt->toString() == "int* const");

        CHECK(*ptrToConstInt != *intPtrA);
        CHECK(*constPtrToInt != *intPtrA);
        CHECK(*ptrToConstInt != *constPtrToInt);

        auto viaWithConst = intPtrA->withConst(true);
        REQUIRE(viaWithConst != nullptr);
        CHECK(viaWithConst->isConst());
        CHECK(*viaWithConst == *constPtrToInt);

        auto viaWithVolatile = intPtrA->withVolatile(true);
        REQUIRE(viaWithVolatile != nullptr);
        CHECK(viaWithVolatile->isVolatile());
        CHECK(viaWithVolatile->toString() == "int* volatile");
    }
}
