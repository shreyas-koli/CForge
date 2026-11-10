#pragma once

#include "lexer/token.h"

#include <cstddef>
#include <memory>
#include <string>

// =============================================================================
// TypeKind — CForge Type Classification (Day 36)
// =============================================================================
//
// Identifies the primary category or primitive type.
// Designed to be extensible in subsequent days for composite types:
// Pointers, Arrays, Functions, Structs, etc.
//
enum class TypeKind {
    Unknown, // Invalid or non-type token/specifier
    Void,
    Char,
    Int,
    Float,
    Double,
    // Future kinds:
    // Pointer,
    // Array,
    // Function,
    // Struct,
    // Union,
    // Enum,
};

// =============================================================================
// Type — Base Class for all C Types in CForge
// =============================================================================
//
// Abstract representation of a C type. Provides common kind and category
// queries, stringification, structural equality, and canonical factory
// methods for primitive types.
// methods for primitive types. Supports type qualifiers (const).
//
class Type {
public:
    explicit Type(TypeKind kind);
    explicit Type(TypeKind kind, bool isConst = false);
    virtual ~Type() = default;

    TypeKind kind() const { return m_kind; }
    TypeKind getKind() const { return m_kind; }

    // Qualifier queries (Day 38)
    bool isConst() const { return m_isConst; }

    // Kind queries
    bool isVoid() const { return m_kind == TypeKind::Void; }
    bool isChar() const { return m_kind == TypeKind::Char; }
    bool isInt() const { return m_kind == TypeKind::Int; }
    bool isFloat() const { return m_kind == TypeKind::Float; }
    bool isDouble() const { return m_kind == TypeKind::Double; }
    bool isUnknown() const { return m_kind == TypeKind::Unknown; }

    // Category queries
    virtual bool isPrimitive() const { return false; }
    bool isInteger() const { return isChar() || isInt(); }
    bool isFloatingPoint() const { return isFloat() || isDouble(); }
    bool isArithmetic() const { return isInteger() || isFloatingPoint(); }

    // Standard size in bytes (target-dependent default)
    virtual std::size_t size() const;

    // Display name / stringification (e.g. "int", "char", "void")
    // Display name / stringification (e.g. "int", "char", "void", "const int")
    virtual std::string toString() const;
    virtual const char* name() const;

    // Structural equivalence comparison
    virtual bool equals(const Type& other) const;

    bool operator==(const Type& other) const { return equals(other); }
    bool operator!=(const Type& other) const { return !equals(other); }

    // Canonical primitive type instances (Flyweight / singleton pattern)
    static std::shared_ptr<Type> getVoid();
    static std::shared_ptr<Type> getChar();
    static std::shared_ptr<Type> getInt();
    static std::shared_ptr<Type> getFloat();
    static std::shared_ptr<Type> getDouble();
    static std::shared_ptr<Type> getVoid(bool isConst = false);
    static std::shared_ptr<Type> getChar(bool isConst = false);
    static std::shared_ptr<Type> getInt(bool isConst = false);
    static std::shared_ptr<Type> getFloat(bool isConst = false);
    static std::shared_ptr<Type> getDouble(bool isConst = false);

    // Factory by TypeKind or keyword/token
    static std::shared_ptr<Type> getPrimitive(TypeKind kind);
    static std::shared_ptr<Type> fromKeyword(const std::string& keyword);
    static std::shared_ptr<Type> fromToken(const Token& token);
    static std::shared_ptr<Type> getPrimitive(TypeKind kind, bool isConst = false);
    static std::shared_ptr<Type> fromKeyword(const std::string& keyword, bool isConst = false);
    static std::shared_ptr<Type> fromToken(const Token& token, bool isConst = false);

    // Qualifier transformation helpers
    virtual std::shared_ptr<Type> withConst(bool isConst = true) const;
    std::shared_ptr<Type> asConst() const { return withConst(true); }

private:
    TypeKind m_kind;
    bool m_isConst{false};
};

// =============================================================================
// PrimitiveType — Concrete Class for C Primitive Types
// =============================================================================
class PrimitiveType : public Type {
public:
    explicit PrimitiveType(TypeKind kind);
    explicit PrimitiveType(TypeKind kind, bool isConst = false);
    ~PrimitiveType() override = default;

    bool isPrimitive() const override { return true; }
    std::size_t size() const override;
    std::string toString() const override;
    const char* name() const override;
    bool equals(const Type& other) const override;
    std::shared_ptr<Type> withConst(bool isConst = true) const override;
};

// =============================================================================
// Helper Functions for Keyword <-> Type Mapping
// =============================================================================

// Returns true if kind is one of Void, Char, Int, Float, Double
bool isPrimitiveTypeKind(TypeKind kind);

// Returns human-readable name of a TypeKind (e.g. "int", "void")
const char* typeKindName(TypeKind kind);
std::string typeKindToString(TypeKind kind);

// Check if a string matches any primitive type keyword ("int", "char", "float", "double", "void")
bool isPrimitiveTypeKeyword(const std::string& name);

// Check if a Token is a Keyword representing a primitive type
bool isPrimitiveTypeToken(const Token& token);

// Resolve keyword string to TypeKind, or TypeKind::Unknown if not a primitive keyword
TypeKind typeKindFromKeyword(const std::string& name);

// Resolve Token to TypeKind, or TypeKind::Unknown if not a primitive keyword token
TypeKind typeKindFromToken(const Token& token);

// Factory helpers returning canonical shared_ptr<Type>, or nullptr if not primitive
std::shared_ptr<Type> getPrimitiveType(TypeKind kind);
std::shared_ptr<Type> getPrimitiveType(const std::string& keyword);
std::shared_ptr<Type> primitiveTypeFromKeyword(const std::string& name);
std::shared_ptr<Type> primitiveTypeFromToken(const Token& token);
std::shared_ptr<Type> getPrimitiveType(TypeKind kind, bool isConst = false);
std::shared_ptr<Type> getPrimitiveType(const std::string& keyword, bool isConst = false);
std::shared_ptr<Type> primitiveTypeFromKeyword(const std::string& name, bool isConst = false);
std::shared_ptr<Type> primitiveTypeFromToken(const Token& token, bool isConst = false);
