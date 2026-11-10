#include <catch2/catch.hpp>

#include "types/conversions.h"
#include "types/type.h"

#include <vector>

// =============================================================================
// DAY 39 TESTS — Type Compatibility & Conversions
// =============================================================================

TEST_CASE("TypeConversions: Complete 5x5 primitive conversion matrix", "[types][conversions][matrix]") {
    auto tChar   = Type::getChar();
    auto tInt    = Type::getInt();
    auto tFloat  = Type::getFloat();
    auto tDouble = Type::getDouble();
    auto tVoid   = Type::getVoid();

    std::vector<std::shared_ptr<Type>> allTypes = {tChar, tInt, tFloat, tDouble, tVoid};

    // Expected 5x5 matrix:
    // Row = Source, Column = Destination
    // Order: char, int, float, double, void
    ConversionKind expected[5][5] = {
        // dst:   char                      int                       float                     double                    void
        /* char */   {ConversionKind::Identity, ConversionKind::Implicit, ConversionKind::Implicit, ConversionKind::Implicit, ConversionKind::Invalid},
        /* int */    {ConversionKind::Implicit, ConversionKind::Identity, ConversionKind::Implicit, ConversionKind::Implicit, ConversionKind::Invalid},
        /* float */  {ConversionKind::Implicit, ConversionKind::Implicit, ConversionKind::Identity, ConversionKind::Implicit, ConversionKind::Invalid},
        /* double */ {ConversionKind::Implicit, ConversionKind::Implicit, ConversionKind::Implicit, ConversionKind::Identity, ConversionKind::Invalid},
        /* void */   {ConversionKind::Invalid,  ConversionKind::Invalid,  ConversionKind::Invalid,  ConversionKind::Invalid,  ConversionKind::Identity},
    };

    for (std::size_t r = 0; r < allTypes.size(); ++r) {
        for (std::size_t c = 0; c < allTypes.size(); ++c) {
            auto src = allTypes[r];
            auto dst = allTypes[c];
            ConversionKind exp = expected[r][c];

            DYNAMIC_SECTION("Converting " << src->toString() << " to " << dst->toString()) {
                CHECK(getConversionKind(*src, *dst) == exp);
                CHECK(getConversionKind(src.get(), dst.get()) == exp);
                CHECK(getConversionKind(src, dst) == exp);

                if (exp == ConversionKind::Identity) {
                    CHECK(isIdentityConversion(*src, *dst));
                    CHECK_FALSE(isStrictlyImplicit(*src, *dst));
                    CHECK(isImplicitlyConvertible(*src, *dst));
                } else if (exp == ConversionKind::Implicit) {
                    CHECK_FALSE(isIdentityConversion(*src, *dst));
                    CHECK(isStrictlyImplicit(*src, *dst));
                    CHECK(isImplicitlyConvertible(*src, *dst));
                } else {
                    CHECK_FALSE(isIdentityConversion(*src, *dst));
                    CHECK_FALSE(isStrictlyImplicit(*src, *dst));
                    CHECK_FALSE(isImplicitlyConvertible(*src, *dst));
                }
            }
        }
    }
}

TEST_CASE("TypeConversions: Identity conversions", "[types][conversions][identity]") {
    SECTION("char -> char") {
        auto t = Type::getChar();
        CHECK(getConversionKind(*t, *t) == ConversionKind::Identity);
        CHECK(isIdentityConversion(*t, *t));
        CHECK(isImplicitlyConvertible(*t, *t));
        CHECK_FALSE(isStrictlyImplicit(*t, *t));
        CHECK_FALSE(isLossyConversion(*t, *t));
    }

    SECTION("int -> int") {
        auto t = Type::getInt();
        CHECK(getConversionKind(*t, *t) == ConversionKind::Identity);
        CHECK(isIdentityConversion(*t, *t));
        CHECK(isImplicitlyConvertible(*t, *t));
        CHECK_FALSE(isStrictlyImplicit(*t, *t));
        CHECK_FALSE(isLossyConversion(*t, *t));
    }

    SECTION("float -> float") {
        auto t = Type::getFloat();
        CHECK(getConversionKind(*t, *t) == ConversionKind::Identity);
        CHECK(isIdentityConversion(*t, *t));
        CHECK(isImplicitlyConvertible(*t, *t));
        CHECK_FALSE(isStrictlyImplicit(*t, *t));
        CHECK_FALSE(isLossyConversion(*t, *t));
    }

    SECTION("double -> double") {
        auto t = Type::getDouble();
        CHECK(getConversionKind(*t, *t) == ConversionKind::Identity);
        CHECK(isIdentityConversion(*t, *t));
        CHECK(isImplicitlyConvertible(*t, *t));
        CHECK_FALSE(isStrictlyImplicit(*t, *t));
        CHECK_FALSE(isLossyConversion(*t, *t));
    }

    SECTION("void -> void") {
        auto t = Type::getVoid();
        CHECK(getConversionKind(*t, *t) == ConversionKind::Identity);
        CHECK(isIdentityConversion(*t, *t));
        CHECK(isImplicitlyConvertible(*t, *t));
        CHECK_FALSE(isStrictlyImplicit(*t, *t));
        CHECK_FALSE(isLossyConversion(*t, *t));
    }
}

TEST_CASE("TypeConversions: char to other numeric types", "[types][conversions][char]") {
    auto tChar   = Type::getChar();
    auto tInt    = Type::getInt();
    auto tFloat  = Type::getFloat();
    auto tDouble = Type::getDouble();

    SECTION("char -> int (integer promotion, lossless)") {
        CHECK(getConversionKind(*tChar, *tInt) == ConversionKind::Implicit);
        CHECK(isImplicitlyConvertible(*tChar, *tInt));
        CHECK(isStrictlyImplicit(*tChar, *tInt));
        CHECK(isIntegerPromotion(*tChar, *tInt));
        CHECK(isWideningConversion(*tChar, *tInt));
        CHECK_FALSE(isLossyConversion(*tChar, *tInt));
    }

    SECTION("char -> float (lossless widening)") {
        CHECK(getConversionKind(*tChar, *tFloat) == ConversionKind::Implicit);
        CHECK(isImplicitlyConvertible(*tChar, *tFloat));
        CHECK_FALSE(isIntegerPromotion(*tChar, *tFloat));
        CHECK(isWideningConversion(*tChar, *tFloat));
        CHECK_FALSE(isLossyConversion(*tChar, *tFloat));
    }

    SECTION("char -> double (lossless widening)") {
        CHECK(getConversionKind(*tChar, *tDouble) == ConversionKind::Implicit);
        CHECK(isImplicitlyConvertible(*tChar, *tDouble));
        CHECK_FALSE(isIntegerPromotion(*tChar, *tDouble));
        CHECK(isWideningConversion(*tChar, *tDouble));
        CHECK_FALSE(isLossyConversion(*tChar, *tDouble));
    }
}

TEST_CASE("TypeConversions: int to other numeric types", "[types][conversions][int]") {
    auto tChar   = Type::getChar();
    auto tInt    = Type::getInt();
    auto tFloat  = Type::getFloat();
    auto tDouble = Type::getDouble();

    SECTION("int -> char (narrowing integer width, lossy)") {
        CHECK(getConversionKind(*tInt, *tChar) == ConversionKind::Implicit);
        CHECK(isImplicitlyConvertible(*tInt, *tChar));
        CHECK(isLossyConversion(*tInt, *tChar));
        CHECK_FALSE(isWideningConversion(*tInt, *tChar));
        CHECK_FALSE(isIntegerPromotion(*tInt, *tChar));
    }

    SECTION("int -> float (32-bit int to 24-bit mantissa float, potentially lossy)") {
        CHECK(getConversionKind(*tInt, *tFloat) == ConversionKind::Implicit);
        CHECK(isImplicitlyConvertible(*tInt, *tFloat));
        CHECK(isLossyConversion(*tInt, *tFloat));
        CHECK_FALSE(isWideningConversion(*tInt, *tFloat));
    }

    SECTION("int -> double (32-bit int to 53-bit mantissa double, lossless)") {
        CHECK(getConversionKind(*tInt, *tDouble) == ConversionKind::Implicit);
        CHECK(isImplicitlyConvertible(*tInt, *tDouble));
        CHECK_FALSE(isLossyConversion(*tInt, *tDouble));
        CHECK(isWideningConversion(*tInt, *tDouble));
    }
}

TEST_CASE("TypeConversions: float to integer and floating-point types", "[types][conversions][float]") {
    auto tChar   = Type::getChar();
    auto tInt    = Type::getInt();
    auto tFloat  = Type::getFloat();
    auto tDouble = Type::getDouble();

    SECTION("float -> char (truncation of fraction, lossy)") {
        CHECK(getConversionKind(*tFloat, *tChar) == ConversionKind::Implicit);
        CHECK(isImplicitlyConvertible(*tFloat, *tChar));
        CHECK(isLossyConversion(*tFloat, *tChar));
        CHECK_FALSE(isWideningConversion(*tFloat, *tChar));
    }

    SECTION("float -> int (truncation of fraction, lossy)") {
        CHECK(getConversionKind(*tFloat, *tInt) == ConversionKind::Implicit);
        CHECK(isImplicitlyConvertible(*tFloat, *tInt));
        CHECK(isLossyConversion(*tFloat, *tInt));
        CHECK_FALSE(isWideningConversion(*tFloat, *tInt));
    }

    SECTION("float -> double (32-bit to 64-bit float widening, lossless)") {
        CHECK(getConversionKind(*tFloat, *tDouble) == ConversionKind::Implicit);
        CHECK(isImplicitlyConvertible(*tFloat, *tDouble));
        CHECK_FALSE(isLossyConversion(*tFloat, *tDouble));
        CHECK(isWideningConversion(*tFloat, *tDouble));
    }
}

TEST_CASE("TypeConversions: double to integer and floating-point types", "[types][conversions][double]") {
    auto tChar   = Type::getChar();
    auto tInt    = Type::getInt();
    auto tFloat  = Type::getFloat();
    auto tDouble = Type::getDouble();

    SECTION("double -> char (truncation of fraction, lossy)") {
        CHECK(getConversionKind(*tDouble, *tChar) == ConversionKind::Implicit);
        CHECK(isImplicitlyConvertible(*tDouble, *tChar));
        CHECK(isLossyConversion(*tDouble, *tChar));
        CHECK_FALSE(isWideningConversion(*tDouble, *tChar));
    }

    SECTION("double -> int (truncation of fraction, lossy)") {
        CHECK(getConversionKind(*tDouble, *tInt) == ConversionKind::Implicit);
        CHECK(isImplicitlyConvertible(*tDouble, *tInt));
        CHECK(isLossyConversion(*tDouble, *tInt));
        CHECK_FALSE(isWideningConversion(*tDouble, *tInt));
    }

    SECTION("double -> float (narrowing 64-bit to 32-bit float, lossy)") {
        CHECK(getConversionKind(*tDouble, *tFloat) == ConversionKind::Implicit);
        CHECK(isImplicitlyConvertible(*tDouble, *tFloat));
        CHECK(isLossyConversion(*tDouble, *tFloat));
        CHECK_FALSE(isWideningConversion(*tDouble, *tFloat));
    }
}

TEST_CASE("TypeConversions: All conversions involving void", "[types][conversions][void]") {
    auto tVoid   = Type::getVoid();
    auto tChar   = Type::getChar();
    auto tInt    = Type::getInt();
    auto tFloat  = Type::getFloat();
    auto tDouble = Type::getDouble();

    SECTION("void as source to numeric types is Invalid") {
        CHECK(getConversionKind(*tVoid, *tChar) == ConversionKind::Invalid);
        CHECK(getConversionKind(*tVoid, *tInt) == ConversionKind::Invalid);
        CHECK(getConversionKind(*tVoid, *tFloat) == ConversionKind::Invalid);
        CHECK(getConversionKind(*tVoid, *tDouble) == ConversionKind::Invalid);

        CHECK_FALSE(isImplicitlyConvertible(*tVoid, *tChar));
        CHECK_FALSE(isImplicitlyConvertible(*tVoid, *tInt));
        CHECK_FALSE(isImplicitlyConvertible(*tVoid, *tFloat));
        CHECK_FALSE(isImplicitlyConvertible(*tVoid, *tDouble));
    }

    SECTION("numeric types as source to void is Invalid") {
        CHECK(getConversionKind(*tChar, *tVoid) == ConversionKind::Invalid);
        CHECK(getConversionKind(*tInt, *tVoid) == ConversionKind::Invalid);
        CHECK(getConversionKind(*tFloat, *tVoid) == ConversionKind::Invalid);
        CHECK(getConversionKind(*tDouble, *tVoid) == ConversionKind::Invalid);

        CHECK_FALSE(isImplicitlyConvertible(*tChar, *tVoid));
        CHECK_FALSE(isImplicitlyConvertible(*tInt, *tVoid));
        CHECK_FALSE(isImplicitlyConvertible(*tFloat, *tVoid));
        CHECK_FALSE(isImplicitlyConvertible(*tDouble, *tVoid));
    }

    SECTION("void to void is Identity") {
        CHECK(getConversionKind(*tVoid, *tVoid) == ConversionKind::Identity);
        CHECK(isIdentityConversion(*tVoid, *tVoid));
        CHECK_FALSE(isStrictlyImplicit(*tVoid, *tVoid));
        CHECK_FALSE(isLossyConversion(*tVoid, *tVoid));
    }
}

TEST_CASE("TypeConversions: const and non-const variants", "[types][conversions][const]") {
    auto tInt        = Type::getInt(false);
    auto tConstInt   = Type::getInt(true);
    auto tChar       = Type::getChar(false);
    auto tConstChar  = Type::getChar(true);
    auto tFloat      = Type::getFloat(false);
    auto tConstFloat = Type::getFloat(true);
    auto tVoid       = Type::getVoid(false);
    auto tConstVoid  = Type::getVoid(true);

    SECTION("const does not alter Identity conversion category") {
        // const int -> int
        CHECK(getConversionKind(*tConstInt, *tInt) == ConversionKind::Identity);
        CHECK(isIdentityConversion(*tConstInt, *tInt));

        // int -> const int
        CHECK(getConversionKind(*tInt, *tConstInt) == ConversionKind::Identity);
        CHECK(isIdentityConversion(*tInt, *tConstInt));

        // const int -> const int
        CHECK(getConversionKind(*tConstInt, *tConstInt) == ConversionKind::Identity);
        CHECK(isIdentityConversion(*tConstInt, *tConstInt));

        // const void -> void
        CHECK(getConversionKind(*tConstVoid, *tVoid) == ConversionKind::Identity);
        CHECK(getConversionKind(*tVoid, *tConstVoid) == ConversionKind::Identity);
        CHECK(getConversionKind(*tConstVoid, *tConstVoid) == ConversionKind::Identity);
    }

    SECTION("const does not alter Implicit numeric conversion category") {
        // const char -> int
        CHECK(getConversionKind(*tConstChar, *tInt) == ConversionKind::Implicit);
        CHECK(isIntegerPromotion(*tConstChar, *tInt));
        CHECK_FALSE(isLossyConversion(*tConstChar, *tInt));

        // char -> const int
        CHECK(getConversionKind(*tChar, *tConstInt) == ConversionKind::Implicit);
        CHECK(isIntegerPromotion(*tChar, *tConstInt));

        // const char -> const int
        CHECK(getConversionKind(*tConstChar, *tConstInt) == ConversionKind::Implicit);
        CHECK(isIntegerPromotion(*tConstChar, *tConstInt));

        // const float -> int
        CHECK(getConversionKind(*tConstFloat, *tInt) == ConversionKind::Implicit);
        CHECK(isLossyConversion(*tConstFloat, *tInt));

        // float -> const int
        CHECK(getConversionKind(*tFloat, *tConstInt) == ConversionKind::Implicit);
        CHECK(isLossyConversion(*tFloat, *tConstInt));
    }

    SECTION("const does not alter Invalid void conversion category") {
        CHECK(getConversionKind(*tConstVoid, *tInt) == ConversionKind::Invalid);
        CHECK(getConversionKind(*tInt, *tConstVoid) == ConversionKind::Invalid);
        CHECK(getConversionKind(*tConstVoid, *tConstInt) == ConversionKind::Invalid);
        CHECK(getConversionKind(*tConstInt, *tConstVoid) == ConversionKind::Invalid);
    }
}

TEST_CASE("TypeConversions: Invalid, unsupported, and edge cases", "[types][conversions][errors]") {
    auto tInt = Type::getInt();
    PrimitiveType tUnknown(TypeKind::Unknown);

    SECTION("Null pointer safety") {
        CHECK(getConversionKind(nullptr, tInt.get()) == ConversionKind::Invalid);
        CHECK(getConversionKind(tInt.get(), nullptr) == ConversionKind::Invalid);
        CHECK(getConversionKind(nullptr, nullptr) == ConversionKind::Invalid);

        CHECK_FALSE(isImplicitlyConvertible(nullptr, tInt.get()));
        CHECK_FALSE(isImplicitlyConvertible(tInt.get(), nullptr));
        CHECK_FALSE(isImplicitlyConvertible(nullptr, nullptr));

        CHECK_FALSE(isIdentityConversion(nullptr, tInt.get()));
        CHECK_FALSE(isStrictlyImplicit(nullptr, tInt.get()));
        CHECK_FALSE(isLossyConversion(nullptr, tInt.get()));
        CHECK_FALSE(isIntegerPromotion(nullptr, tInt.get()));
        CHECK_FALSE(isWideningConversion(nullptr, tInt.get()));

        std::shared_ptr<Type> nullShared;
        CHECK(getConversionKind(nullShared, tInt) == ConversionKind::Invalid);
        CHECK_FALSE(isImplicitlyConvertible(nullShared, tInt));
    }

    SECTION("Unknown type instances are Invalid") {
        CHECK(getConversionKind(tUnknown, *tInt) == ConversionKind::Invalid);
        CHECK(getConversionKind(*tInt, tUnknown) == ConversionKind::Invalid);
        CHECK(getConversionKind(tUnknown, tUnknown) == ConversionKind::Invalid);

        CHECK_FALSE(isImplicitlyConvertible(tUnknown, *tInt));
        CHECK_FALSE(isImplicitlyConvertible(*tInt, tUnknown));
        CHECK_FALSE(isImplicitlyConvertible(tUnknown, tUnknown));
    }
}

TEST_CASE("TypeConversions: String representations and names", "[types][conversions][strings]") {
    CHECK(std::string(conversionKindName(ConversionKind::Identity)) == "Identity");
    CHECK(std::string(conversionKindName(ConversionKind::Implicit)) == "Implicit");
    CHECK(std::string(conversionKindName(ConversionKind::Invalid)) == "Invalid");

    CHECK(conversionKindToString(ConversionKind::Identity) == "Identity");
    CHECK(conversionKindToString(ConversionKind::Implicit) == "Implicit");
    CHECK(conversionKindToString(ConversionKind::Invalid) == "Invalid");
}
