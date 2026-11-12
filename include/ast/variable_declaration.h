#pragma once

#include "ast/statement.h"
#include "ast/expression.h"
#include "types/type.h"

#include <memory>
#include <string>
#include <ostream>

// =============================================================================
// TypeRef — Dual-Compatibility View for Function Parameter Types (Day 40)
// =============================================================================
//
// Provides seamless compatibility for VariableDeclaration::type():
// 1. Acts as a pointer-like view to Type (operator->, operator const Type*, != nullptr)
// 2. Acts as a string value for legacy tests (operator== std::string / const char*)
//
class TypeRef {
public:
    TypeRef(const Type* type, const std::string& name)
        : m_type(type), m_name(name) {}

    // Pointer-like Type accessors
    const Type* get() const noexcept { return m_type; }
    const Type* operator->() const noexcept { return m_type; }
    const Type& operator*() const noexcept { return *m_type; }
    operator const Type*() const noexcept { return m_type; }

    // Pointer comparisons
    bool operator==(const Type* other) const noexcept { return m_type == other; }
    bool operator!=(const Type* other) const noexcept { return m_type != other; }
    friend bool operator==(const Type* lhs, const TypeRef& rhs) noexcept { return lhs == rhs.m_type; }
    friend bool operator!=(const Type* lhs, const TypeRef& rhs) noexcept { return lhs != rhs.m_type; }
    bool operator==(std::nullptr_t) const noexcept { return m_type == nullptr; }
    bool operator!=(std::nullptr_t) const noexcept { return m_type != nullptr; }
    friend bool operator==(std::nullptr_t, const TypeRef& rhs) noexcept { return rhs.m_type == nullptr; }
    friend bool operator!=(std::nullptr_t, const TypeRef& rhs) noexcept { return rhs.m_type != nullptr; }

    // String conversions and comparisons
    const std::string& str() const noexcept { return m_name; }
    operator const std::string&() const noexcept { return m_name; }

    bool operator==(const std::string& other) const noexcept { return m_name == other; }
    bool operator!=(const std::string& other) const noexcept { return m_name != other; }
    bool operator==(const char* other) const noexcept { return m_name == other; }
    bool operator!=(const char* other) const noexcept { return m_name != other; }

    friend bool operator==(const char* lhs, const TypeRef& rhs) noexcept { return rhs == lhs; }
    friend bool operator!=(const char* lhs, const TypeRef& rhs) noexcept { return rhs != lhs; }
    friend bool operator==(const std::string& lhs, const TypeRef& rhs) noexcept { return rhs == lhs; }
    friend bool operator!=(const std::string& lhs, const TypeRef& rhs) noexcept { return rhs != lhs; }

    // Stream output helper
    friend std::ostream& operator<<(std::ostream& os, const TypeRef& tr) {
        return os << tr.m_name;
    }

    // String concatenation helpers
    friend std::string operator+(const TypeRef& tr, const std::string& rhs) { return tr.str() + rhs; }
    friend std::string operator+(const TypeRef& tr, const char* rhs) { return tr.str() + rhs; }
    friend std::string operator+(const std::string& lhs, const TypeRef& tr) { return lhs + tr.str(); }
    friend std::string operator+(const char* lhs, const TypeRef& tr) { return lhs + tr.str(); }

private:
    const Type* m_type{nullptr};
    const std::string& m_name;
};

// =============================================================================
// VariableDeclaration — AST Node for Function Parameters & Variable Declarations
// =============================================================================
//
// Represents function parameters and variable declarations:
//   int a
//   const char ch
//   double result
//
// Stores both the strongly-typed Type representation (Day 36, Day 38, Day 40)
// and preserves the textual type name for legacy AST compatibility.
//
class VariableDeclaration : public Statement {
public:
    // Legacy / string-based constructor (preserves Day 18 tests)
    VariableDeclaration(std::string name,
                         std::string type,
                         std::unique_ptr<Expression> initializer = nullptr,
                         SourceLocation loc = {});

    // Modern / Type-based constructor (Day 40)
    VariableDeclaration(std::shared_ptr<Type> type,
                         std::string name,
                         std::unique_ptr<Expression> initializer = nullptr,
                         SourceLocation loc = {});

    // Convenience constructor (name, shared_ptr<Type>)
    VariableDeclaration(std::string name,
                         std::shared_ptr<Type> type,
                         std::unique_ptr<Expression> initializer = nullptr,
                         SourceLocation loc = {});

    ~VariableDeclaration() override;

    // Identifier name accessors
    const std::string& name() const { return m_name; }
    const std::string& getName() const { return m_name; }
    bool hasName() const { return !m_name.empty(); }

    // Type accessors
    TypeRef type() const { return TypeRef(m_type.get(), m_typeName); }
    const std::string& typeName() const { return m_typeName; }
    const std::string& getTypeName() const { return m_typeName; }

    const Type* declaredType() const { return m_type.get(); }
    const Type* getTypePtr() const { return m_type.get(); }
    const std::shared_ptr<Type>& getType() const { return m_type; }
    const std::shared_ptr<Type>& typeShared() const { return m_type; }
    const std::shared_ptr<Type>& getTypeShared() const { return m_type; }
    TypeKind typeKind() const { return m_type ? m_type->kind() : TypeKind::Unknown; }
    bool isConst() const { return m_type && m_type->isConst(); }

    // Initializer expression accessors
    bool hasInitializer() const { return m_initializer != nullptr; }
    const Expression* initializer() const { return m_initializer.get(); }
    Expression* initializer() { return m_initializer.get(); }
    const Expression* getInitializer() const { return m_initializer.get(); }

    std::unique_ptr<Expression> takeInitializer();
    void setInitializer(std::unique_ptr<Expression> initializer);

    std::string toString() const override;

private:
    std::shared_ptr<Type> m_type;
    std::string m_typeName;
    std::string m_name;
    std::unique_ptr<Expression> m_initializer;
};
