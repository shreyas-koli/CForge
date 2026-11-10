#include "types/type.h"

#include <unordered_map>

// =============================================================================
// Canonical Flyweight Singletons for Primitive Types
// =============================================================================
static const auto s_voidType        = std::make_shared<PrimitiveType>(TypeKind::Void, false);
static const auto s_charType        = std::make_shared<PrimitiveType>(TypeKind::Char, false);
static const auto s_intType         = std::make_shared<PrimitiveType>(TypeKind::Int, false);
static const auto s_floatType       = std::make_shared<PrimitiveType>(TypeKind::Float, false);
static const auto s_doubleType      = std::make_shared<PrimitiveType>(TypeKind::Double, false);

static const auto s_constVoidType   = std::make_shared<PrimitiveType>(TypeKind::Void, true);
static const auto s_constCharType   = std::make_shared<PrimitiveType>(TypeKind::Char, true);
static const auto s_constIntType    = std::make_shared<PrimitiveType>(TypeKind::Int, true);
static const auto s_constFloatType  = std::make_shared<PrimitiveType>(TypeKind::Float, true);
static const auto s_constDoubleType = std::make_shared<PrimitiveType>(TypeKind::Double, true);

// =============================================================================
// Type Implementation
// =============================================================================

Type::Type(TypeKind kind, bool isConst)
    : m_kind(kind)
    , m_isConst(isConst) {
}

std::size_t Type::size() const {
    return 0;
}

std::string Type::toString() const {
    if (m_isConst) {
        return "const " + typeKindToString(m_kind);
    }
    return typeKindToString(m_kind);
}

const char* Type::name() const {
    if (m_isConst) {
        switch (m_kind) {
            case TypeKind::Void:   return "const void";
            case TypeKind::Char:   return "const char";
            case TypeKind::Int:    return "const int";
            case TypeKind::Float:  return "const float";
            case TypeKind::Double: return "const double";
            default:               return "<unknown>";
        }
    }
    return typeKindName(m_kind);
}

bool Type::equals(const Type& other) const {
    return m_kind == other.m_kind && m_isConst == other.m_isConst;
}

std::shared_ptr<Type> Type::withConst(bool isConst) const {
    if (isPrimitive()) {
        return getPrimitive(m_kind, isConst);
    }
    return nullptr;
}

std::shared_ptr<Type> Type::getVoid(bool isConst) {
    return isConst ? s_constVoidType : s_voidType;
}

std::shared_ptr<Type> Type::getChar(bool isConst) {
    return isConst ? s_constCharType : s_charType;
}

std::shared_ptr<Type> Type::getInt(bool isConst) {
    return isConst ? s_constIntType : s_intType;
}

std::shared_ptr<Type> Type::getFloat(bool isConst) {
    return isConst ? s_constFloatType : s_floatType;
}

std::shared_ptr<Type> Type::getDouble(bool isConst) {
    return isConst ? s_constDoubleType : s_doubleType;
}

std::shared_ptr<Type> Type::getPrimitive(TypeKind kind, bool isConst) {
    switch (kind) {
        case TypeKind::Void:   return isConst ? s_constVoidType : s_voidType;
        case TypeKind::Char:   return isConst ? s_constCharType : s_charType;
        case TypeKind::Int:    return isConst ? s_constIntType : s_intType;
        case TypeKind::Float:  return isConst ? s_constFloatType : s_floatType;
        case TypeKind::Double: return isConst ? s_constDoubleType : s_doubleType;
        default:               return nullptr;
    }
}

std::shared_ptr<Type> Type::fromKeyword(const std::string& keyword, bool isConst) {
    return primitiveTypeFromKeyword(keyword, isConst);
}

std::shared_ptr<Type> Type::fromToken(const Token& token, bool isConst) {
    return primitiveTypeFromToken(token, isConst);
}

// =============================================================================
// PrimitiveType Implementation
// =============================================================================

PrimitiveType::PrimitiveType(TypeKind kind, bool isConst)
    : Type(kind, isConst) {
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
    return Type::toString();
}

const char* PrimitiveType::name() const {
    return Type::name();
}

bool PrimitiveType::equals(const Type& other) const {
    if (this == &other) {
        return true;
    }
    return other.isPrimitive() && (kind() == other.kind()) && (isConst() == other.isConst());
}

std::shared_ptr<Type> PrimitiveType::withConst(bool isConst) const {
    return Type::getPrimitive(kind(), isConst);
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

std::shared_ptr<Type> getPrimitiveType(TypeKind kind, bool isConst) {
    return Type::getPrimitive(kind, isConst);
}

std::shared_ptr<Type> getPrimitiveType(const std::string& keyword, bool isConst) {
    return primitiveTypeFromKeyword(keyword, isConst);
}

std::shared_ptr<Type> primitiveTypeFromKeyword(const std::string& name, bool isConst) {
    std::string cleanName = name;
    bool explicitConst = isConst;
    if (cleanName.rfind("const ", 0) == 0) {
        explicitConst = true;
        cleanName = cleanName.substr(6);
    }
    TypeKind kind = typeKindFromKeyword(cleanName);
    if (isPrimitiveTypeKind(kind)) {
        return getPrimitiveType(kind, explicitConst);
    }
    return nullptr;
}

std::shared_ptr<Type> primitiveTypeFromToken(const Token& token, bool isConst) {
    TypeKind kind = typeKindFromToken(token);
    if (isPrimitiveTypeKind(kind)) {
        return getPrimitiveType(kind, isConst);
    }
    return nullptr;
}
