#pragma once

#include "ast/statement.h"
#include "ast/expression.h"
#include "ast/storage_class.h"
#include "types/type.h"

#include <memory>
#include <string>

// =============================================================================
// VarDecl — AST Node for C Variable Declarations (Day 37, 38, 42)
// =============================================================================
//
// Represents C variable declarations with an optional initializer:
//   int x;
//   static int y = 10;
//   extern char letter;
//   register float price;
//   const volatile double value;
//
class VarDecl : public Statement {
public:
    VarDecl(std::shared_ptr<Type> type,
            std::string name,
            std::unique_ptr<Expression> initializer = nullptr,
            SourceLocation loc = {},
            StorageClass storageClass = StorageClass::None);
    ~VarDecl() override;

    // Type accessors
    const Type* type() const { return m_type.get(); }
    const std::shared_ptr<Type>& typeShared() const { return m_type; }
    const std::shared_ptr<Type>& getType() const { return m_type; }
    TypeKind typeKind() const { return m_type ? m_type->kind() : TypeKind::Unknown; }
    bool isConst() const { return m_type && m_type->isConst(); }
    bool isVolatile() const { return m_type && m_type->isVolatile(); }

    // Storage class accessors (Day 42)
    StorageClass storageClass() const { return m_storageClass; }
    StorageClass getStorageClass() const { return m_storageClass; }
    bool isStatic() const { return m_storageClass == StorageClass::Static; }
    bool isExtern() const { return m_storageClass == StorageClass::Extern; }
    bool isRegister() const { return m_storageClass == StorageClass::Register; }
    void setStorageClass(StorageClass sc) { m_storageClass = sc; }

    // Identifier name accessors
    const std::string& name() const { return m_name; }
    const std::string& getName() const { return m_name; }

    // Initializer expression accessors
    bool hasInitializer() const { return m_initializer != nullptr; }
    const Expression* initializer() const { return m_initializer.get(); }
    Expression* initializer() { return m_initializer.get(); }
    const Expression* getInitializer() const { return m_initializer.get(); }

    std::unique_ptr<Expression> takeInitializer();
    void setInitializer(std::unique_ptr<Expression> initializer);

    // Formatted textual representation
    std::string toString() const override;

private:
    std::shared_ptr<Type> m_type;
    std::string m_name;
    std::unique_ptr<Expression> m_initializer;
    StorageClass m_storageClass{StorageClass::None};
};
