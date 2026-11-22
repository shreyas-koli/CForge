#include "semantic/symbol_table.h"

#include <algorithm>
#include <sstream>
#include <unordered_set>

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
// Symbol Implementation (Day 45 & Day 46)
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
    , m_scopeLevel(0)
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
// SymbolTable Implementation (Day 45 & Day 46)
// =============================================================================

SymbolTable::SymbolTable() {
    m_scopes.emplace_back(); // Base scope: Global scope at index 0 (level 0)
}

// -----------------------------------------------------------------------------
// Scope Management (Day 46)
// -----------------------------------------------------------------------------

void SymbolTable::pushScope() {
    m_scopes.emplace_back();
}

bool SymbolTable::popScope() {
    if (m_scopes.size() <= 1) {
        return false; // Cannot pop the global scope
    }
    m_scopes.pop_back();
    return true;
}

std::size_t SymbolTable::scopeDepth() const noexcept {
    return m_scopes.size();
}

std::size_t SymbolTable::currentScopeLevel() const noexcept {
    return m_scopes.empty() ? 0 : m_scopes.size() - 1;
}

bool SymbolTable::isGlobalScope() const noexcept {
    return m_scopes.size() <= 1;
}

void SymbolTable::resetToGlobalScope() noexcept {
    if (m_scopes.size() > 1) {
        m_scopes.resize(1);
    }
}

// -----------------------------------------------------------------------------
// Insertion (Targets Current Innermost Scope)
// -----------------------------------------------------------------------------

bool SymbolTable::insert(const Symbol& symbol) {
    if (m_scopes.empty()) {
        m_scopes.emplace_back();
    }
    auto& curScope = m_scopes.back();
    if (curScope.symbols.find(symbol.name()) != curScope.symbols.end()) {
        return false; // Duplicate declaration within the same scope rejected
    }
    Symbol symCopy = symbol;
    symCopy.setScopeLevel(currentScopeLevel());
    curScope.order.push_back(symbol.name());
    curScope.symbols.emplace(symbol.name(), std::move(symCopy));
    return true;
}

bool SymbolTable::insert(Symbol&& symbol) {
    if (m_scopes.empty()) {
        m_scopes.emplace_back();
    }
    auto& curScope = m_scopes.back();
    if (curScope.symbols.find(symbol.name()) != curScope.symbols.end()) {
        return false; // Duplicate declaration within the same scope rejected
    }
    std::string name = symbol.name();
    symbol.setScopeLevel(currentScopeLevel());
    curScope.order.push_back(name);
    curScope.symbols.emplace(std::move(name), std::move(symbol));
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
    if (m_scopes.empty()) {
        m_scopes.emplace_back();
    }
    auto& curScope = m_scopes.back();
    auto it = curScope.symbols.find(symbol.name());
    if (it != curScope.symbols.end()) {
        Symbol symCopy = symbol;
        symCopy.setScopeLevel(currentScopeLevel());
        it->second = std::move(symCopy);
    } else {
        insert(symbol);
    }
}

// -----------------------------------------------------------------------------
// Lookup (Innermost to Outermost Resolution)
// -----------------------------------------------------------------------------

const Symbol* SymbolTable::lookup(const std::string& name) const {
    for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
        auto symIt = it->symbols.find(name);
        if (symIt != it->symbols.end()) {
            return &symIt->second;
        }
    }
    return nullptr;
}

Symbol* SymbolTable::lookup(const std::string& name) {
    for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
        auto symIt = it->symbols.find(name);
        if (symIt != it->symbols.end()) {
            return &symIt->second;
        }
    }
    return nullptr;
}

const Symbol* SymbolTable::lookupCurrentScope(const std::string& name) const {
    if (m_scopes.empty()) return nullptr;
    auto it = m_scopes.back().symbols.find(name);
    return it != m_scopes.back().symbols.end() ? &it->second : nullptr;
}

Symbol* SymbolTable::lookupCurrentScope(const std::string& name) {
    if (m_scopes.empty()) return nullptr;
    auto it = m_scopes.back().symbols.find(name);
    return it != m_scopes.back().symbols.end() ? &it->second : nullptr;
}

const Symbol* SymbolTable::lookupGlobal(const std::string& name) const {
    if (m_scopes.empty()) return nullptr;
    auto it = m_scopes.front().symbols.find(name);
    return it != m_scopes.front().symbols.end() ? &it->second : nullptr;
}

Symbol* SymbolTable::lookupGlobal(const std::string& name) {
    if (m_scopes.empty()) return nullptr;
    auto it = m_scopes.front().symbols.find(name);
    return it != m_scopes.front().symbols.end() ? &it->second : nullptr;
}

bool SymbolTable::contains(const std::string& name) const {
    return lookup(name) != nullptr;
}

bool SymbolTable::containsCurrentScope(const std::string& name) const {
    return lookupCurrentScope(name) != nullptr;
}

bool SymbolTable::containsGlobal(const std::string& name) const {
    return lookupGlobal(name) != nullptr;
}

// -----------------------------------------------------------------------------
// Removal (From Current Scope)
// -----------------------------------------------------------------------------

bool SymbolTable::remove(const std::string& name) {
    if (m_scopes.empty()) return false;
    auto& curScope = m_scopes.back();
    auto it = curScope.symbols.find(name);
    if (it == curScope.symbols.end()) {
        return false;
    }
    curScope.symbols.erase(it);
    curScope.order.erase(
        std::remove(curScope.order.begin(), curScope.order.end(), name),
        curScope.order.end()
    );
    return true;
}

// -----------------------------------------------------------------------------
// Size and Collection Queries
// -----------------------------------------------------------------------------

std::size_t SymbolTable::size() const noexcept {
    std::size_t total = 0;
    for (const auto& s : m_scopes) {
        total += s.symbols.size();
    }
    return total;
}

std::size_t SymbolTable::currentScopeSize() const noexcept {
    return m_scopes.empty() ? 0 : m_scopes.back().symbols.size();
}

std::size_t SymbolTable::globalScopeSize() const noexcept {
    return m_scopes.empty() ? 0 : m_scopes.front().symbols.size();
}

bool SymbolTable::empty() const noexcept {
    return size() == 0;
}

void SymbolTable::clear() noexcept {
    m_scopes.clear();
    m_scopes.emplace_back(); // Reset to single empty global scope
}

const std::unordered_map<std::string, Symbol>& SymbolTable::symbols() const noexcept {
    static const std::unordered_map<std::string, Symbol> emptySymbols;
    return m_scopes.empty() ? emptySymbols : m_scopes.back().symbols;
}

const std::vector<std::string>& SymbolTable::order() const noexcept {
    static const std::vector<std::string> emptyOrder;
    return m_scopes.empty() ? emptyOrder : m_scopes.back().order;
}

const SymbolTable::Scope& SymbolTable::currentScope() const {
    return m_scopes.back();
}

const SymbolTable::Scope& SymbolTable::scopeAt(std::size_t index) const {
    return m_scopes.at(index);
}

std::vector<const Symbol*> SymbolTable::getAllSymbols() const {
    std::vector<const Symbol*> result;
    for (const auto& scope : m_scopes) {
        for (const auto& name : scope.order) {
            auto it = scope.symbols.find(name);
            if (it != scope.symbols.end()) {
                result.push_back(&it->second);
            }
        }
    }
    return result;
}

std::vector<const Symbol*> SymbolTable::getAllVisibleSymbols() const {
    std::vector<const Symbol*> result;
    std::unordered_set<std::string> seen;
    for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
        for (const auto& name : it->order) {
            if (seen.insert(name).second) {
                auto symIt = it->symbols.find(name);
                if (symIt != it->symbols.end()) {
                    result.push_back(&symIt->second);
                }
            }
        }
    }
    return result;
}

// -----------------------------------------------------------------------------
// AST Population (Global Declarations Only)
// -----------------------------------------------------------------------------

void SymbolTable::populate(const Program& program) {
    for (const auto& stmt : program.statements()) {
        if (!stmt) continue;
        if (const auto* fn = dynamic_cast<const Function*>(stmt.get())) {
            if (!insert(*fn)) {
                // If a prototype was inserted earlier and this is a definition,
                // update the existing symbol's definition status and AST node.
                auto* existing = lookupCurrentScope(fn->name());
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
                auto* existing = lookupCurrentScope(fn->name());
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
    ss << "SymbolTable(" << size() << " symbols) {\n";
    for (std::size_t level = 0; level < m_scopes.size(); ++level) {
        for (const auto& name : m_scopes[level].order) {
            auto it = m_scopes[level].symbols.find(name);
            if (it != m_scopes[level].symbols.end()) {
                ss << "  " << it->second.toString() << "\n";
            }
        }
    }
    ss << "}";
    return ss.str();
}
