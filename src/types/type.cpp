#include "types/type.h"

#include <unordered_map>

// =============================================================================
// Canonical Flyweight Singletons for Primitive Types
// =============================================================================
static const auto s_voidType   = std::make_shared<PrimitiveType>(TypeKind::Void);
static const auto s_charType   = std::make_shared<PrimitiveType>(TypeKind::Char);
static const auto s_intType    = std::make_shared<PrimitiveType>(TypeKind::Int);
static const auto s_floatType  = std::make_shared<PrimitiveType>(TypeKind::Float);
static const auto s_doubleType = std::make_shared<PrimitiveType>(TypeKind::Double);

// =============================================================================
// Type Implementation
// =============================================================================

Type::Type(TypeKind kind)
    : m_kind(kind) {
}

std::size_t Type::size() const {
    return 0;
}

std::string Type::toString() const {
    return typeKindToString(m_kind);
}

const char* Type::name() const {
    return typeKindName(m_kind);
}

bool Type::equals(const Type& other) const {
    return m_kind == other.m_kind;
}

std::shared_ptr<Type> Type::getVoid() {
    return s_voidType;
}

std::shared_ptr<Type> Type::getChar() {
    return s_charType;
}

std::shared_ptr<Type> Type::getInt() {
    return s_intType;
}

std::shared_ptr<Type> Type::getFloat() {
    return s_floatType;
}

std::shared_ptr<Type> Type::getDouble() {
    return s_doubleType;
}

std::shared_ptr<Type> Type::getPrimitive(TypeKind kind) {
    switch (kind) {
        case TypeKind::Void:   return s_voidType;
        case TypeKind::Char:   return s_charType;
        case TypeKind::Int:    return s_intType;
        case TypeKind::Float:  return s_floatType;
        case TypeKind::Double: return s_doubleType;
        default:               return nullptr;
    }
}

std::shared_ptr<Type> Type::fromKeyword(const std::string& keyword) {
    return primitiveTypeFromKeyword(keyword);
}

std::shared_ptr<Type> Type::fromToken(const Token& token) {
    return primitiveTypeFromToken(token);
}

// =============================================================================
// PrimitiveType Implementation
// =============================================================================

PrimitiveType::PrimitiveType(TypeKind kind)
    : Type(kind) {
}

std::size_t PrimitiveType::size() const {
    switch (kind()) {
        case TypeKind::Void:   return 0;
        case TypeKind::Char:   return 1;
        case TypeKind::Int:    return 4;
        case TypeKind::Float:  return 4;
        case TypeKind::Double: return 8;
        default:               return 0;
    }
}

std::string PrimitiveType::toString() const {
    return typeKindToString(kind());
}

const char* PrimitiveType::name() const {
    return typeKindName(kind());
}

bool PrimitiveType::equals(const Type& other) const {
    if (this == &other) {
        return true;
    }
    return other.isPrimitive() && (kind() == other.kind());
}

// =============================================================================
// Helper Functions for Keyword <-> Type Mapping
// =============================================================================

bool isPrimitiveTypeKind(TypeKind kind) {
    switch (kind) {
        case TypeKind::Void:
        case TypeKind::Char:
        case TypeKind::Int:
        case TypeKind::Float:
        case TypeKind::Double:
            return true;
        default:
            return false;
    }
}

const char* typeKindName(TypeKind kind) {
    switch (kind) {
        case TypeKind::Void:   return "void";
        case TypeKind::Char:   return "char";
        case TypeKind::Int:    return "int";
        case TypeKind::Float:  return "float";
        case TypeKind::Double: return "double";
        default:               return "<unknown>";
    }
}

std::string typeKindToString(TypeKind kind) {
    return typeKindName(kind);
}

TypeKind typeKindFromKeyword(const std::string& name) {
    static const std::unordered_map<std::string, TypeKind> keywordMap = {
        {"void",   TypeKind::Void},
        {"char",   TypeKind::Char},
        {"int",    TypeKind::Int},
        {"float",  TypeKind::Float},
        {"double", TypeKind::Double},
    };

    auto it = keywordMap.find(name);
    if (it != keywordMap.end()) {
        return it->second;
    }
    return TypeKind::Unknown;
}

bool isPrimitiveTypeKeyword(const std::string& name) {
    return isPrimitiveTypeKind(typeKindFromKeyword(name));
}

TypeKind typeKindFromToken(const Token& token) {
    if (token.type != TokenType::Keyword) {
        return TypeKind::Unknown;
    }
    return typeKindFromKeyword(token.lexeme);
}

bool isPrimitiveTypeToken(const Token& token) {
    return isPrimitiveTypeKind(typeKindFromToken(token));
}

std::shared_ptr<Type> getPrimitiveType(TypeKind kind) {
    return Type::getPrimitive(kind);
}

std::shared_ptr<Type> getPrimitiveType(const std::string& keyword) {
    return primitiveTypeFromKeyword(keyword);
}

std::shared_ptr<Type> primitiveTypeFromKeyword(const std::string& name) {
    TypeKind kind = typeKindFromKeyword(name);
    if (isPrimitiveTypeKind(kind)) {
        return getPrimitiveType(kind);
    }
    return nullptr;
}

std::shared_ptr<Type> primitiveTypeFromToken(const Token& token) {
    TypeKind kind = typeKindFromToken(token);
    if (isPrimitiveTypeKind(kind)) {
        return getPrimitiveType(kind);
    }
    return nullptr;
}
