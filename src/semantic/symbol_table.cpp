#include "semantic/symbol_table.h"

#include <algorithm>
#include <sstream>

// =============================================================================
// SymbolKind String Representation (Day 45)
// =============================================================================
const char* symbolKindToString(SymbolKind kind) {
    switch (kind) {
        case SymbolKind::Variable: return "Variable";
        case SymbolKind::Function: return "Function";
        default:                   return "Unknown";
    }
}

// =============================================================================
// Symbol Implementation (Day 45)
// =============================================================================

Symbol::Symbol(std::string name,
               SymbolKind kind,
               std::shared_ptr<Type> type,
               SourceLocation loc,
               StorageClass storageClass,
               const ASTNode* astNode)
    : m_name(std::move(name))
    , m_kind(kind)
    , m_type(std::move(type))
    , m_location(loc)
    , m_storageClass(storageClass)
    , m_astNode(astNode)
{}

Symbol Symbol::createVariable(std::string name,
                             std::shared_ptr<Type> type,
                             SourceLocation loc,
                             StorageClass sc,
                             const ASTNode* astNode) {
    return Symbol(std::move(name), SymbolKind::Variable, std::move(type), loc, sc, astNode);
}

Symbol Symbol::createFunction(std::string name,
                             std::shared_ptr<Type> returnType,
                             std::vector<std::shared_ptr<Type>> paramTypes,
                             bool isDefinition,
                             ParameterListKind paramListKind,
                             SourceLocation loc,
                             StorageClass sc,
                             const ASTNode* astNode) {
    Symbol sym(std::move(name), SymbolKind::Function, std::move(returnType), loc, sc, astNode);
    sym.m_paramTypes = std::move(paramTypes);
    sym.m_isDefinition = isDefinition;
    sym.m_paramListKind = paramListKind;
    return sym;
}

Symbol Symbol::fromVarDecl(const VarDecl& varDecl) {
    return Symbol(varDecl.name(),
                  SymbolKind::Variable,
                  varDecl.typeShared(),
                  varDecl.location(),
                  varDecl.storageClass(),
                  &varDecl);
}

Symbol Symbol::fromVariableDeclaration(const VariableDeclaration& varDecl) {
    return Symbol(varDecl.name(),
                  SymbolKind::Variable,
                  varDecl.typeShared(),
                  varDecl.location(),
                  varDecl.storageClass(),
                  &varDecl);
}

Symbol Symbol::fromFunction(const Function& fn) {
    Symbol sym(fn.name(),
               SymbolKind::Function,
               fn.returnTypeShared(),
               fn.location(),
               fn.storageClass(),
               &fn);

    sym.m_isDefinition = fn.isDefinition();
    sym.m_paramListKind = fn.parameterListKind();

    std::size_t count = fn.parameterCount();
    sym.m_paramTypes.reserve(count);
    sym.m_paramNames.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        if (const auto* param = fn.parameter(i)) {
            sym.m_paramTypes.push_back(param->typeShared());
            sym.m_paramNames.push_back(param->name());
        }
    }

    return sym;
}

std::string Symbol::toString() const {
    std::ostringstream ss;
    ss << "Symbol(" << m_name << ": " << symbolKindToString(m_kind);
    if (m_storageClass != StorageClass::None) {
        ss << ", storage=" << storageClassToString(m_storageClass);
    }
    if (m_kind == SymbolKind::Variable) {
        ss << ", type=" << (m_type ? m_type->toString() : "unknown");
    } else if (m_kind == SymbolKind::Function) {
        ss << ", return=" << (m_type ? m_type->toString() : "void");
        ss << ", params=(";
        if (m_paramListKind == ParameterListKind::Void) {
            ss << "void";
        } else {
            for (std::size_t i = 0; i < m_paramTypes.size(); ++i) {
                if (i > 0) ss << ", ";
                ss << (m_paramTypes[i] ? m_paramTypes[i]->toString() : "unknown");
                if (i < m_paramNames.size() && !m_paramNames[i].empty()) {
                    ss << " " << m_paramNames[i];
                }
            }
        }
        ss << ")";
        if (m_isDefinition) {
            ss << " [def]";
        } else {
            ss << " [proto]";
        }
    }
    ss << ")";
    return ss.str();
}

// =============================================================================
// SymbolTable Implementation (Day 45)
// =============================================================================

bool SymbolTable::insert(const Symbol& symbol) {
    if (contains(symbol.name())) {
        return false;
    }
    m_symbols.emplace(symbol.name(), symbol);
    m_insertionOrder.push_back(symbol.name());
    return true;
}

bool SymbolTable::insert(Symbol&& symbol) {
    if (contains(symbol.name())) {
        return false;
    }
    std::string name = symbol.name();
    m_symbols.emplace(name, std::move(symbol));
    m_insertionOrder.push_back(std::move(name));
    return true;
}

bool SymbolTable::insert(const VarDecl& varDecl) {
    return insert(Symbol::fromVarDecl(varDecl));
}

bool SymbolTable::insert(const Function& fnDecl) {
    return insert(Symbol::fromFunction(fnDecl));
}

bool SymbolTable::insert(const VariableDeclaration& varDecl) {
    return insert(Symbol::fromVariableDeclaration(varDecl));
}

void SymbolTable::insertOrAssign(const Symbol& symbol) {
    auto it = m_symbols.find(symbol.name());
    if (it != m_symbols.end()) {
        it->second = symbol;
    } else {
        insert(symbol);
    }
}

const Symbol* SymbolTable::lookup(const std::string& name) const {
    auto it = m_symbols.find(name);
    return it != m_symbols.end() ? &it->second : nullptr;
}

Symbol* SymbolTable::lookup(const std::string& name) {
    auto it = m_symbols.find(name);
    return it != m_symbols.end() ? &it->second : nullptr;
}

bool SymbolTable::contains(const std::string& name) const {
    return m_symbols.find(name) != m_symbols.end();
}

bool SymbolTable::remove(const std::string& name) {
    auto it = m_symbols.find(name);
    if (it == m_symbols.end()) {
        return false;
    }
    m_symbols.erase(it);
    m_insertionOrder.erase(
        std::remove(m_insertionOrder.begin(), m_insertionOrder.end(), name),
        m_insertionOrder.end()
    );
    return true;
}

void SymbolTable::clear() noexcept {
    m_symbols.clear();
    m_insertionOrder.clear();
}

std::vector<const Symbol*> SymbolTable::getAllSymbols() const {
    std::vector<const Symbol*> result;
    result.reserve(m_insertionOrder.size());
    for (const auto& name : m_insertionOrder) {
        auto it = m_symbols.find(name);
        if (it != m_symbols.end()) {
            result.push_back(&it->second);
        }
    }
    return result;
}

void SymbolTable::populate(const Program& program) {
    for (const auto& stmt : program.statements()) {
        if (!stmt) continue;
        if (const auto* fn = dynamic_cast<const Function*>(stmt.get())) {
            if (!insert(*fn)) {
                // If a prototype was inserted earlier and this is a definition,
                // update the existing symbol's definition status and AST node.
                auto* existing = lookup(fn->name());
                if (existing && existing->isFunction() && !existing->isDefinition() && fn->isDefinition()) {
                    existing->setIsDefinition(true);
                    existing->setASTNode(fn);
                }
            }
        } else if (const auto* var = dynamic_cast<const VarDecl*>(stmt.get())) {
            insert(*var);
        } else if (const auto* varLegacy = dynamic_cast<const VariableDeclaration*>(stmt.get())) {
            insert(*varLegacy);
        }
    }
}

void SymbolTable::populate(const Program* program) {
    if (program) {
        populate(*program);
    }
}

void SymbolTable::populate(const std::vector<std::unique_ptr<Statement>>& statements) {
    for (const auto& stmt : statements) {
        if (!stmt) continue;
        if (const auto* fn = dynamic_cast<const Function*>(stmt.get())) {
            if (!insert(*fn)) {
                auto* existing = lookup(fn->name());
                if (existing && existing->isFunction() && !existing->isDefinition() && fn->isDefinition()) {
                    existing->setIsDefinition(true);
                    existing->setASTNode(fn);
                }
            }
        } else if (const auto* var = dynamic_cast<const VarDecl*>(stmt.get())) {
            insert(*var);
        } else if (const auto* varLegacy = dynamic_cast<const VariableDeclaration*>(stmt.get())) {
            insert(*varLegacy);
        }
    }
}

void SymbolTable::populate(const ASTNode& node) {
    if (const auto* prog = dynamic_cast<const Program*>(&node)) {
        populate(*prog);
    } else if (const auto* fn = dynamic_cast<const Function*>(&node)) {
        insert(*fn);
    } else if (const auto* var = dynamic_cast<const VarDecl*>(&node)) {
        insert(*var);
    } else if (const auto* varLegacy = dynamic_cast<const VariableDeclaration*>(&node)) {
        insert(*varLegacy);
    }
}

void SymbolTable::populate(const ASTNode* node) {
    if (node) {
        populate(*node);
    }
}

std::string SymbolTable::toString() const {
    std::ostringstream ss;
    ss << "SymbolTable(" << m_symbols.size() << " symbols) {\n";
    for (const auto& name : m_insertionOrder) {
        auto it = m_symbols.find(name);
        if (it != m_symbols.end()) {
            ss << "  " << it->second.toString() << "\n";
        }
    }
    ss << "}";
    return ss.str();
}
