#pragma once

#include "types/type.h"

#include <memory>
#include <string>

// =============================================================================
// ConversionKind — CForge Type Conversion Categories (Day 39)
// =============================================================================
//
// Classifies type compatibility and conversion rules between C types.
//
// Categories:
//   Identity — Source and destination types are equivalent.
//   Implicit — An implicit conversion is permitted (e.g. numeric promotion, demotion).
//   Invalid  — Conversion is not permitted in this project's current policy.
//
enum class ConversionKind {
    Identity,
    Implicit,
    Invalid,
};

// String conversions for diagnostics and testing
const char* conversionKindName(ConversionKind kind);
std::string conversionKindToString(ConversionKind kind);

// =============================================================================
// Core Conversion Queries
// =============================================================================

// Primary conversion classification between source and destination types
ConversionKind getConversionKind(const Type& source, const Type& destination);
ConversionKind getConversionKind(const Type* source, const Type* destination);
ConversionKind getConversionKind(const std::shared_ptr<Type>& source,
                                 const std::shared_ptr<Type>& destination);

// Convenience boolean query: returns true if conversion is allowed (Identity or Implicit)
bool isImplicitlyConvertible(const Type& source, const Type& destination);
bool isImplicitlyConvertible(const Type* source, const Type* destination);
bool isImplicitlyConvertible(const std::shared_ptr<Type>& source,
                             const std::shared_ptr<Type>& destination);

// Convenience boolean query for strict identity (equivalent types)
bool isIdentityConversion(const Type& source, const Type& destination);
bool isIdentityConversion(const Type* source, const Type* destination);
bool isIdentityConversion(const std::shared_ptr<Type>& source,
                          const std::shared_ptr<Type>& destination);

// Convenience boolean query for strictly implicit (permitted non-identity) conversions
bool isStrictlyImplicit(const Type& source, const Type& destination);
bool isStrictlyImplicit(const Type* source, const Type* destination);
bool isStrictlyImplicit(const std::shared_ptr<Type>& source,
                        const std::shared_ptr<Type>& destination);

// =============================================================================
// Fine-Grained Numeric Conversion Attributes
// =============================================================================

// Returns true if conversion from source to destination is potentially lossy
// (e.g. float/double to char/int, double to float, int to char, or int to float)
bool isLossyConversion(const Type& source, const Type& destination);
bool isLossyConversion(const Type* source, const Type* destination);
bool isLossyConversion(const std::shared_ptr<Type>& source,
                       const std::shared_ptr<Type>& destination);

// Returns true if conversion is an integer promotion (char -> int)
bool isIntegerPromotion(const Type& source, const Type& destination);
bool isIntegerPromotion(const Type* source, const Type* destination);
bool isIntegerPromotion(const std::shared_ptr<Type>& source,
                        const std::shared_ptr<Type>& destination);

// Returns true if conversion is a lossless promotion/widening
bool isWideningConversion(const Type& source, const Type& destination);
bool isWideningConversion(const Type* source, const Type* destination);
bool isWideningConversion(const std::shared_ptr<Type>& source,
                          const std::shared_ptr<Type>& destination);
