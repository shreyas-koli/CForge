#include "types/conversions.h"

// =============================================================================
// String Representations
// =============================================================================

const char* conversionKindName(ConversionKind kind) {
    switch (kind) {
        case ConversionKind::Identity: return "Identity";
        case ConversionKind::Implicit: return "Implicit";
        case ConversionKind::Invalid:  return "Invalid";
        default:                       return "<unknown>";
    }
}

std::string conversionKindToString(ConversionKind kind) {
    return conversionKindName(kind);
}

// =============================================================================
// Core Conversion Queries
// =============================================================================

ConversionKind getConversionKind(const Type& source, const Type& destination) {
    TypeKind srcKind = source.kind();
    TypeKind dstKind = destination.kind();

    // Unknown types cannot participate in conversions
    if (srcKind == TypeKind::Unknown || dstKind == TypeKind::Unknown) {
        return ConversionKind::Invalid;
    }

    // Identity: source and destination types have identical underlying kinds
    // (const qualification does not alter underlying numeric category)
    if (srcKind == dstKind) {
        return ConversionKind::Identity;
    }

    // void cannot participate in conversions with other types
    if (srcKind == TypeKind::Void || dstKind == TypeKind::Void) {
        return ConversionKind::Invalid;
    }

    // All remaining pairs among char, int, float, and double are valid implicit conversions
    if (isPrimitiveTypeKind(srcKind) && isPrimitiveTypeKind(dstKind)) {
        return ConversionKind::Implicit;
    }

    return ConversionKind::Invalid;
}

ConversionKind getConversionKind(const Type* source, const Type* destination) {
    if (!source || !destination) {
        return ConversionKind::Invalid;
    }
    return getConversionKind(*source, *destination);
}

ConversionKind getConversionKind(const std::shared_ptr<Type>& source,
                                 const std::shared_ptr<Type>& destination) {
    return getConversionKind(source.get(), destination.get());
}

bool isImplicitlyConvertible(const Type& source, const Type& destination) {
    ConversionKind kind = getConversionKind(source, destination);
    return kind == ConversionKind::Identity || kind == ConversionKind::Implicit;
}

bool isImplicitlyConvertible(const Type* source, const Type* destination) {
    if (!source || !destination) {
        return false;
    }
    return isImplicitlyConvertible(*source, *destination);
}

bool isImplicitlyConvertible(const std::shared_ptr<Type>& source,
                             const std::shared_ptr<Type>& destination) {
    return isImplicitlyConvertible(source.get(), destination.get());
}

bool isIdentityConversion(const Type& source, const Type& destination) {
    return getConversionKind(source, destination) == ConversionKind::Identity;
}

bool isIdentityConversion(const Type* source, const Type* destination) {
    return source && destination && isIdentityConversion(*source, *destination);
}

bool isIdentityConversion(const std::shared_ptr<Type>& source,
                          const std::shared_ptr<Type>& destination) {
    return isIdentityConversion(source.get(), destination.get());
}

bool isStrictlyImplicit(const Type& source, const Type& destination) {
    return getConversionKind(source, destination) == ConversionKind::Implicit;
}

bool isStrictlyImplicit(const Type* source, const Type* destination) {
    return source && destination && isStrictlyImplicit(*source, *destination);
}

bool isStrictlyImplicit(const std::shared_ptr<Type>& source,
                        const std::shared_ptr<Type>& destination) {
    return isStrictlyImplicit(source.get(), destination.get());
}

// =============================================================================
// Fine-Grained Numeric Conversion Attributes
// =============================================================================

bool isLossyConversion(const Type& source, const Type& destination) {
    if (getConversionKind(source, destination) != ConversionKind::Implicit) {
        return false;
    }

    TypeKind srcKind = source.kind();
    TypeKind dstKind = destination.kind();

    // Floating-point to integer (truncation of fraction)
    if ((srcKind == TypeKind::Float || srcKind == TypeKind::Double) &&
        (dstKind == TypeKind::Char || dstKind == TypeKind::Int)) {
        return true;
    }

    // Double to float (narrowing floating precision from 64-bit to 32-bit)
    if (srcKind == TypeKind::Double && dstKind == TypeKind::Float) {
        return true;
    }

    // Int to char (narrowing integer width from 32-bit to 8-bit)
    if (srcKind == TypeKind::Int && dstKind == TypeKind::Char) {
        return true;
    }

    // Int (32-bit, 31-bit magnitude) to float (24-bit mantissa)
    if (srcKind == TypeKind::Int && dstKind == TypeKind::Float) {
        return true;
    }

    return false;
}

bool isLossyConversion(const Type* source, const Type* destination) {
    return source && destination && isLossyConversion(*source, *destination);
}

bool isLossyConversion(const std::shared_ptr<Type>& source,
                       const std::shared_ptr<Type>& destination) {
    return isLossyConversion(source.get(), destination.get());
}

bool isIntegerPromotion(const Type& source, const Type& destination) {
    return source.kind() == TypeKind::Char && destination.kind() == TypeKind::Int;
}

bool isIntegerPromotion(const Type* source, const Type* destination) {
    return source && destination && isIntegerPromotion(*source, *destination);
}

bool isIntegerPromotion(const std::shared_ptr<Type>& source,
                        const std::shared_ptr<Type>& destination) {
    return isIntegerPromotion(source.get(), destination.get());
}

bool isWideningConversion(const Type& source, const Type& destination) {
    if (getConversionKind(source, destination) != ConversionKind::Implicit) {
        return false;
    }
    return !isLossyConversion(source, destination);
}

bool isWideningConversion(const Type* source, const Type* destination) {
    return source && destination && isWideningConversion(*source, *destination);
}

bool isWideningConversion(const std::shared_ptr<Type>& source,
                          const std::shared_ptr<Type>& destination) {
    return isWideningConversion(source.get(), destination.get());
}
