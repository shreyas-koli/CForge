#pragma once

#include "ast/ast_node.h"
#include "ast/function.h"
#include "ast/program.h"
#include "ast/storage_class.h"
#include "ast/var_decl.h"
#include "ast/variable_declaration.h"
#include "types/type.h"

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// =============================================================================
// SymbolKind — CForge Symbol Classification (Day 45)
// =============================================================================
//
// Identifies the primary category of a declared program entity:
//   - Variable: Variable declaration (VarDecl / VariableDeclaration)
//   - Function: Function prototype or definition (Function / FunctionDecl)
//
enum class SymbolKind {
    Variable,
    Function,
};

const char* symbolKindToString(SymbolKind kind);

// =============================================================================
// Symbol — Metadata Entry for a Declared Identifier (Day 45)
// =============================================================================
//
// Represents a single entry in the symbol table, associating an identifier name
// with its declared type, kind (variable vs function), source location,
// storage class, and AST node reference.
//
class Symbol {
public:
    Symbol() = default;

    // Primary constructor
    Symbol(std::string name,
           SymbolKind kind,
           std::shared_ptr<Type> type,
           SourceLocation loc = {},
           StorageClass storageClass = StorageClass::None,
           const ASTNode* astNode = nullptr);

    // Identifier name accessors
    const std::string& name() const { return m_name; }
    const std::string& getName() const { return m_name; }
    void setName(std::string name) { m_name = std::move(name); }

    // Symbol kind accessors
    SymbolKind kind() const { return m_kind; }
    SymbolKind getKind() const { return m_kind; }
    void setKind(SymbolKind kind) { m_kind = kind; }
    bool isVariable() const { return m_kind == SymbolKind::Variable; }
    bool isFunction() const { return m_kind == SymbolKind::Function; }

    // Type accessors
    const Type* type() const { return m_type.get(); }
    const Type* getTypePtr() const { return m_type.get(); }
    const std::shared_ptr<Type>& typeShared() const { return m_type; }
    const std::shared_ptr<Type>& getType() const { return m_type; }
    const std::shared_ptr<Type>& getTypeShared() const { return m_type; }
    TypeKind typeKind() const { return m_type ? m_type->kind() : TypeKind::Unknown; }
    void setType(std::shared_ptr<Type> type) { m_type = std::move(type); }

    // Qualifier queries
    bool isConst() const { return m_type && m_type->isConst(); }
    bool isVolatile() const { return m_type && m_type->isVolatile(); }

    // Storage class accessors
    StorageClass storageClass() const { return m_storageClass; }
    StorageClass getStorageClass() const { return m_storageClass; }
    void setStorageClass(StorageClass sc) { m_storageClass = sc; }
    bool isStatic() const { return m_storageClass == StorageClass::Static; }
    bool isExtern() const { return m_storageClass == StorageClass::Extern; }
    bool isRegister() const { return m_storageClass == StorageClass::Register; }

    // Source location accessors
    SourceLocation location() const { return m_location; }
    SourceLocation getLocation() const { return m_location; }
    void setLocation(SourceLocation loc) { m_location = loc; }

    // AST node reference
    const ASTNode* astNode() const { return m_astNode; }
    const ASTNode* getASTNode() const { return m_astNode; }
    void setASTNode(const ASTNode* node) { m_astNode = node; }

    // Function-specific accessors
    const Type* returnType() const { return m_type.get(); }
    const std::shared_ptr<Type>& returnTypeShared() const { return m_type; }
    const std::shared_ptr<Type>& getReturnType() const { return m_type; }
    TypeKind returnTypeKind() const { return m_type ? m_type->kind() : TypeKind::Unknown; }

    bool isDefinition() const { return m_isDefinition; }
    bool isDeclaration() const { return !m_isDefinition; }
    bool isPrototype() const { return !m_isDefinition; }
    void setIsDefinition(bool isDef) { m_isDefinition = isDef; }

    ParameterListKind parameterListKind() const { return m_paramListKind; }
    ParameterListKind getParameterListKind() const { return m_paramListKind; }
    void setParameterListKind(ParameterListKind kind) { m_paramListKind = kind; }
    bool hasVoidParameterList() const { return m_paramListKind == ParameterListKind::Void; }
    bool hasUnspecifiedParameters() const { return m_paramListKind == ParameterListKind::Unspecified; }

    std::size_t parameterCount() const { return m_paramTypes.size(); }
    const std::vector<std::shared_ptr<Type>>& parameterTypes() const { return m_paramTypes; }
    const std::vector<std::shared_ptr<Type>>& getParameterTypes() const { return m_paramTypes; }
    const Type* parameter(std::size_t index) const {
        return index < m_paramTypes.size() ? m_paramTypes[index].get() : nullptr;
    }
    const std::shared_ptr<Type>& parameterShared(std::size_t index) const {
        static const std::shared_ptr<Type> nullType{nullptr};
        return index < m_paramTypes.size() ? m_paramTypes[index] : nullType;
    }
    const std::vector<std::string>& parameterNames() const { return m_paramNames; }
    const std::string& parameterName(std::size_t index) const {
        static const std::string empty;
        return index < m_paramNames.size() ? m_paramNames[index] : empty;
    }

    void addParameter(std::shared_ptr<Type> paramType, std::string paramName = "") {
        m_paramTypes.push_back(std::move(paramType));
        m_paramNames.push_back(std::move(paramName));
    }
    void setParameterTypes(std::vector<std::shared_ptr<Type>> paramTypes) {
        m_paramTypes = std::move(paramTypes);
    }
    void setParameterNames(std::vector<std::string> paramNames) {
        m_paramNames = std::move(paramNames);
    }

    // Factory methods
    static Symbol createVariable(std::string name,
                                 std::shared_ptr<Type> type,
                                 SourceLocation loc = {},
                                 StorageClass sc = StorageClass::None,
                                 const ASTNode* astNode = nullptr);

    static Symbol createFunction(std::string name,
                                 std::shared_ptr<Type> returnType,
                                 std::vector<std::shared_ptr<Type>> paramTypes = {},
                                 bool isDefinition = false,
                                 ParameterListKind paramListKind = ParameterListKind::Unspecified,
                                 SourceLocation loc = {},
                                 StorageClass sc = StorageClass::None,
                                 const ASTNode* astNode = nullptr);

    // Factory methods from AST nodes
    static Symbol fromVarDecl(const VarDecl& varDecl);
    static Symbol fromFunction(const Function& fn);
    static Symbol fromVariableDeclaration(const VariableDeclaration& varDecl);

    // Textual representation
    std::string toString() const;

private:
    std::string m_name;
    SymbolKind m_kind{SymbolKind::Variable};
    std::shared_ptr<Type> m_type{nullptr};
    SourceLocation m_location{};
    StorageClass m_storageClass{StorageClass::None};
    const ASTNode* m_astNode{nullptr};

    // Function-specific metadata
    std::vector<std::shared_ptr<Type>> m_paramTypes;
    std::vector<std::string> m_paramNames;
    ParameterListKind m_paramListKind{ParameterListKind::Unspecified};
    bool m_isDefinition{false};
};

// =============================================================================
// SymbolTable — Global Symbol Table Data Structure (Day 45)
// =============================================================================
//
// Maps identifier names to declared type/kind information for global declarations.
// Provides insertion, lookup, iteration, and AST walking over Program,
// VarDecl, and FunctionDecl nodes.
//
// Note: Handles global declarations only; nested scope management is Day 46.
//
class SymbolTable {
public:
    SymbolTable() = default;
    ~SymbolTable() = default;

    // Insertion
    bool insert(const Symbol& symbol);
    bool insert(Symbol&& symbol);
    bool insert(const VarDecl& varDecl);
    bool insert(const Function& fnDecl);
    bool insert(const VariableDeclaration& varDecl);
    void insertOrAssign(const Symbol& symbol);

    // Lookup
    const Symbol* lookup(const std::string& name) const;
    Symbol* lookup(const std::string& name);
    const Symbol* find(const std::string& name) const { return lookup(name); }
    Symbol* find(const std::string& name) { return lookup(name); }
    const Symbol* get(const std::string& name) const { return lookup(name); }
    Symbol* get(const std::string& name) { return lookup(name); }

    // Existence queries
    bool contains(const std::string& name) const;
    bool has(const std::string& name) const { return contains(name); }

    // Removal
    bool remove(const std::string& name);

    // Size queries
    std::size_t size() const noexcept { return m_symbols.size(); }
    bool empty() const noexcept { return m_symbols.empty(); }
    void clear() noexcept;

    // Access to symbol collection
    const std::unordered_map<std::string, Symbol>& symbols() const noexcept { return m_symbols; }
    const std::vector<std::string>& order() const noexcept { return m_insertionOrder; }
    std::vector<const Symbol*> getAllSymbols() const;

    // AST population (global declarations only)
    void populate(const Program& program);
    void populate(const Program* program);
    void populate(const std::vector<std::unique_ptr<Statement>>& statements);
    void populate(const ASTNode& node);
    void populate(const ASTNode* node);

    // Textual dump
    std::string toString() const;

private:
    std::unordered_map<std::string, Symbol> m_symbols;
    std::vector<std::string> m_insertionOrder;
};
