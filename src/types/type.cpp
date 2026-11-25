#include "types/type.h"

#include <array>
#include <unordered_map>

// =============================================================================
// Canonical Flyweight Singletons for Primitive Types (Day 36, Day 38, Day 42)
// =============================================================================
//
// 5 primitive types x 2 const states x 2 volatile states = 20 flyweights.
// Indices:
//   Kind: Void(0), Char(1), Int(2), Float(3), Double(4)
//   Const: 0 (false), 1 (true)
//   Volatile: 0 (false), 1 (true)
//
static const auto s_primitives = []() {
    std::array<std::array<std::array<std::shared_ptr<PrimitiveType>, 2>, 2>, 5> table;
    const TypeKind kinds[5] = {TypeKind::Void, TypeKind::Char, TypeKind::Int, TypeKind::Float, TypeKind::Double};
    for (std::size_t k = 0; k < 5; ++k) {
        for (int c = 0; c < 2; ++c) {
            for (int v = 0; v < 2; ++v) {
                table[k][c][v] = std::make_shared<PrimitiveType>(kinds[k], c != 0, v != 0);
            }
        }
    }
    return table;
}();

static inline std::size_t kindToIndex(TypeKind kind) {
    switch (kind) {
        case TypeKind::Void:   return 0;
        case TypeKind::Char:   return 1;
        case TypeKind::Int:    return 2;
        case TypeKind::Float:  return 3;
        case TypeKind::Double: return 4;
        default:               return 0;
    }
}

// =============================================================================
// Type Implementation
// =============================================================================

Type::Type(TypeKind kind, bool isConst, bool isVolatile)
    : m_kind(kind)
    , m_isConst(isConst)
    , m_isVolatile(isVolatile) {
}

std::size_t Type::size() const {
    return 0;
}

std::string Type::toString() const {
    std::string prefix;
    if (m_isConst && m_isVolatile) {
        prefix = "const volatile ";
    } else if (m_isConst) {
        prefix = "const ";
    } else if (m_isVolatile) {
        prefix = "volatile ";
    }
    return prefix + typeKindToString(m_kind);
}

const char* Type::name() const {
    if (m_isConst && m_isVolatile) {
        switch (m_kind) {
            case TypeKind::Void:   return "const volatile void";
            case TypeKind::Char:   return "const volatile char";
            case TypeKind::Int:    return "const volatile int";
            case TypeKind::Float:  return "const volatile float";
            case TypeKind::Double: return "const volatile double";
            default:               return "<unknown>";
        }
    }
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
    if (m_isVolatile) {
        switch (m_kind) {
            case TypeKind::Void:   return "volatile void";
            case TypeKind::Char:   return "volatile char";
            case TypeKind::Int:    return "volatile int";
            case TypeKind::Float:  return "volatile float";
            case TypeKind::Double: return "volatile double";
            default:               return "<unknown>";
        }
    }
    return typeKindName(m_kind);
}

bool Type::equals(const Type& other) const {
    return m_kind == other.m_kind && m_isConst == other.m_isConst && m_isVolatile == other.m_isVolatile;
}

std::shared_ptr<Type> Type::withConst(bool isConst) const {
    if (isPrimitive()) {
        return getPrimitive(m_kind, isConst, m_isVolatile);
    }
    return nullptr;
}

std::shared_ptr<Type> Type::withVolatile(bool isVolatile) const {
    if (isPrimitive()) {
        return getPrimitive(m_kind, m_isConst, isVolatile);
    }
    return nullptr;
}

std::shared_ptr<Type> Type::getVoid(bool isConst, bool isVolatile) {
    return s_primitives[0][isConst ? 1 : 0][isVolatile ? 1 : 0];
}

std::shared_ptr<Type> Type::getChar(bool isConst, bool isVolatile) {
    return s_primitives[1][isConst ? 1 : 0][isVolatile ? 1 : 0];
}

std::shared_ptr<Type> Type::getInt(bool isConst, bool isVolatile) {
    return s_primitives[2][isConst ? 1 : 0][isVolatile ? 1 : 0];
}

std::shared_ptr<Type> Type::getFloat(bool isConst, bool isVolatile) {
    return s_primitives[3][isConst ? 1 : 0][isVolatile ? 1 : 0];
}

std::shared_ptr<Type> Type::getDouble(bool isConst, bool isVolatile) {
    return s_primitives[4][isConst ? 1 : 0][isVolatile ? 1 : 0];
}

std::shared_ptr<Type> Type::getPrimitive(TypeKind kind, bool isConst, bool isVolatile) {
    if (!isPrimitiveTypeKind(kind)) {
        return nullptr;
    }
    return s_primitives[kindToIndex(kind)][isConst ? 1 : 0][isVolatile ? 1 : 0];
}

std::shared_ptr<Type> Type::fromKeyword(const std::string& keyword, bool isConst, bool isVolatile) {
    return primitiveTypeFromKeyword(keyword, isConst, isVolatile);
}

std::shared_ptr<Type> Type::fromToken(const Token& token, bool isConst, bool isVolatile) {
    return primitiveTypeFromToken(token, isConst, isVolatile);
}

// =============================================================================
// PrimitiveType Implementation
// =============================================================================

PrimitiveType::PrimitiveType(TypeKind kind, bool isConst, bool isVolatile)
    : Type(kind, isConst, isVolatile) {
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
    return other.isPrimitive() && (kind() == other.kind()) && (isConst() == other.isConst()) && (isVolatile() == other.isVolatile());
}

std::shared_ptr<Type> PrimitiveType::withConst(bool isConst) const {
    return Type::getPrimitive(kind(), isConst, isVolatile());
}

std::shared_ptr<Type> PrimitiveType::withVolatile(bool isVolatile) const {
    return Type::getPrimitive(kind(), isConst(), isVolatile);
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
        case TypeKind::Array:  return "array";
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

std::shared_ptr<Type> getPrimitiveType(TypeKind kind, bool isConst, bool isVolatile) {
    return Type::getPrimitive(kind, isConst, isVolatile);
}

std::shared_ptr<Type> getPrimitiveType(const std::string& keyword, bool isConst, bool isVolatile) {
    return primitiveTypeFromKeyword(keyword, isConst, isVolatile);
}

std::shared_ptr<Type> primitiveTypeFromKeyword(const std::string& name, bool isConst, bool isVolatile) {
    std::string cleanName = name;
    bool explicitConst = isConst;
    bool explicitVolatile = isVolatile;
    bool matched = true;
    while (matched) {
        if (cleanName.rfind("const ", 0) == 0) {
            explicitConst = true;
            cleanName = cleanName.substr(6);
        } else if (cleanName.rfind("volatile ", 0) == 0) {
            explicitVolatile = true;
            cleanName = cleanName.substr(9);
        } else {
            matched = false;
        }
    }
    TypeKind kind = typeKindFromKeyword(cleanName);
    if (isPrimitiveTypeKind(kind)) {
        return getPrimitiveType(kind, explicitConst, explicitVolatile);
    }
    return nullptr;
}

std::shared_ptr<Type> primitiveTypeFromToken(const Token& token, bool isConst, bool isVolatile) {
    TypeKind kind = typeKindFromToken(token);
    if (isPrimitiveTypeKind(kind)) {
        return getPrimitiveType(kind, isConst, isVolatile);
    }
    return nullptr;
}

// =============================================================================
// ArrayType Implementation (Day 54)
// =============================================================================

ArrayType::ArrayType(std::shared_ptr<Type> elementType, std::size_t size)
    : Type(TypeKind::Array,
           elementType ? elementType->isConst() : false,
           elementType ? elementType->isVolatile() : false)
    , m_elementType(std::move(elementType))
    , m_size(size) {
    if (m_elementType) {
        m_cachedString = m_elementType->toString() + "[" + std::to_string(m_size) + "]";
    } else {
        m_cachedString = "<unknown>[" + std::to_string(m_size) + "]";
    }
}

std::size_t ArrayType::sizeInBytes() const noexcept {
    return m_elementType ? m_size * m_elementType->size() : 0;
}

std::size_t ArrayType::size() const {
    return sizeInBytes();
}

std::string ArrayType::toString() const {
    return m_cachedString;
}

const char* ArrayType::name() const {
    return m_cachedString.c_str();
}

bool ArrayType::equals(const Type& other) const {
    if (this == &other) {
        return true;
    }
    if (other.kind() != TypeKind::Array) {
        return false;
    }
    const auto* otherArr = dynamic_cast<const ArrayType*>(&other);
    if (!otherArr) {
        return false;
    }
    if (m_size != otherArr->m_size) {
        return false;
    }
    if (isConst() != otherArr->isConst() || isVolatile() != otherArr->isVolatile()) {
        return false;
    }
    if (!m_elementType && !otherArr->m_elementType) {
        return true;
    }
    if (!m_elementType || !otherArr->m_elementType) {
        return false;
    }
    return m_elementType->equals(*otherArr->m_elementType);
}

std::shared_ptr<Type> ArrayType::withConst(bool isConst) const {
    if (!m_elementType) {
        return nullptr;
    }
    return std::make_shared<ArrayType>(m_elementType->withConst(isConst), m_size);
}

std::shared_ptr<Type> ArrayType::withVolatile(bool isVolatile) const {
    if (!m_elementType) {
        return nullptr;
    }
    return std::make_shared<ArrayType>(m_elementType->withVolatile(isVolatile), m_size);
}

std::shared_ptr<ArrayType> Type::getArray(std::shared_ptr<Type> elementType, std::size_t size) {
    return std::make_shared<ArrayType>(std::move(elementType), size);
}
