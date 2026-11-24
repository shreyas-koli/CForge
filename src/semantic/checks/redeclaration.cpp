#include "semantic/checks/redeclaration.h"

#include "ast/do_while_stmt.h"
#include "ast/for_stmt.h"
#include "ast/if_stmt.h"
#include "ast/while_stmt.h"

#include <sstream>

// =============================================================================
// RedeclarationChecker Implementation (Day 47)
// =============================================================================

RedeclarationChecker::RedeclarationChecker()
    : m_ownedTable(std::make_unique<SymbolTable>())
    , m_table(*m_ownedTable)
{}

RedeclarationChecker::RedeclarationChecker(SymbolTable& table)
    : m_ownedTable(nullptr)
    , m_table(table)
{}

void RedeclarationChecker::clear() {
    m_diagnostics.clear();
    m_table.clear();
}

void RedeclarationChecker::reportDuplicate(const std::string& name,
                                           SourceLocation loc,
                                           const Symbol* existing) {
    SourceLocation prevLoc = existing ? existing->location() : SourceLocation{};
    bool hasPrev = existing && (prevLoc.line > 0 || prevLoc.hasFilename());
    m_diagnostics.emplace_back(
        name,
        "duplicate declaration of '" + name + "'",
        loc,
        prevLoc,
        hasPrev,
        DiagnosticSeverity::Error
    );
}

void RedeclarationChecker::reportWarning(const std::string& message, SourceLocation loc, const std::string& id) {
    m_diagnostics.emplace_back(id, message, loc, SourceLocation{}, false, DiagnosticSeverity::Warning);
}

void RedeclarationChecker::reportError(const std::string& message, SourceLocation loc, const std::string& id) {
    m_diagnostics.emplace_back(id, message, loc, SourceLocation{}, false, DiagnosticSeverity::Error);
}

bool RedeclarationChecker::check(const Program& program) {
    return checkProgram(program);
}

bool RedeclarationChecker::check(const ASTNode& node) {
    if (const auto* prog = dynamic_cast<const Program*>(&node)) {
        return check(*prog);
    }
    if (const auto* fn = dynamic_cast<const Function*>(&node)) {
        return check(*fn);
    }
    if (const auto* comp = dynamic_cast<const CompoundStmt*>(&node)) {
        return check(*comp);
    }
    if (const auto* var = dynamic_cast<const VarDecl*>(&node)) {
        return check(*var);
    }
    if (const auto* varLegacy = dynamic_cast<const VariableDeclaration*>(&node)) {
        return check(*varLegacy);
    }
    if (const auto* stmt = dynamic_cast<const Statement*>(&node)) {
        return check(*stmt);
    }
    return true;
}

bool RedeclarationChecker::check(const Statement& stmt) {
    return checkStatement(stmt);
}

bool RedeclarationChecker::check(const CompoundStmt& compound) {
    return checkCompoundStmt(compound);
}

bool RedeclarationChecker::check(const Function& fn) {
    return checkFunction(fn);
}

bool RedeclarationChecker::check(const VarDecl& varDecl) {
    return checkVarDecl(varDecl);
}

bool RedeclarationChecker::check(const VariableDeclaration& varDecl) {
    return checkVariableDeclaration(varDecl);
}

bool RedeclarationChecker::checkProgram(const Program& program) {
    bool ok = true;
    for (const auto& stmt : program.statements()) {
        if (stmt) {
            if (!checkStatement(*stmt)) {
                ok = false;
            }
        }
    }
    return ok;
}

bool RedeclarationChecker::checkStatement(const Statement& stmt) {
    if (const auto* var = dynamic_cast<const VarDecl*>(&stmt)) {
        return checkVarDecl(*var);
    }
    if (const auto* varLegacy = dynamic_cast<const VariableDeclaration*>(&stmt)) {
        return checkVariableDeclaration(*varLegacy);
    }
    if (const auto* compound = dynamic_cast<const CompoundStmt*>(&stmt)) {
        return checkCompoundStmt(*compound);
    }
    if (const auto* fn = dynamic_cast<const Function*>(&stmt)) {
        return checkFunction(*fn);
    }
    if (const auto* ifStmt = dynamic_cast<const IfStmt*>(&stmt)) {
        bool ok = true;
        if (ifStmt->thenBranch()) {
            ok = checkStatement(*ifStmt->thenBranch()) && ok;
        }
        if (ifStmt->hasElse() && ifStmt->elseBranch()) {
            ok = checkStatement(*ifStmt->elseBranch()) && ok;
        }
        return ok;
    }
    if (const auto* whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) {
        if (whileStmt->body()) {
            return checkStatement(*whileStmt->body());
        }
        return true;
    }
    if (const auto* doWhileStmt = dynamic_cast<const DoWhileStmt*>(&stmt)) {
        if (doWhileStmt->body()) {
            return checkStatement(*doWhileStmt->body());
        }
        return true;
    }
    if (const auto* forStmt = dynamic_cast<const ForStmt*>(&stmt)) {
        if (forStmt->body()) {
            return checkStatement(*forStmt->body());
        }
        return true;
    }
    return true;
}

bool RedeclarationChecker::checkCompoundStmt(const CompoundStmt& compound) {
    ScopeGuard guard(m_table);
    bool ok = true;
    for (const auto& stmt : compound.statements()) {
        if (stmt) {
            if (!checkStatement(*stmt)) {
                ok = false;
            }
        }
    }
    return ok;
}

bool RedeclarationChecker::checkFunction(const Function& fn) {
    const std::string& name = fn.name();
    const Symbol* existing = m_table.lookupCurrentScope(name);
    bool ok = true;

    if (existing != nullptr) {
        if (existing->isVariable()) {
            reportDuplicate(name, fn.location(), existing);
            ok = false;
        } else if (existing->isFunction()) {
            if (existing->isDefinition() && fn.isDefinition()) {
                SourceLocation prevLoc = existing->location();
                m_diagnostics.emplace_back(
                    name,
                    "redefinition of function '" + name + "'",
                    fn.location(),
                    prevLoc,
                    prevLoc.line > 0
                );
                ok = false;
            } else if (!existing->isDefinition() && fn.isDefinition()) {
                auto* nonConstSym = m_table.lookupCurrentScope(name);
                if (nonConstSym) {
                    nonConstSym->setIsDefinition(true);
                    nonConstSym->setASTNode(&fn);
                }
            }
        }
    } else {
        m_table.insert(fn);
    }

    // Function definition scope handling
    if (fn.isDefinition() || fn.parameterCount() > 0) {
        ScopeGuard guard(m_table);

        for (std::size_t i = 0; i < fn.parameterCount(); ++i) {
            if (const auto* param = fn.parameter(i)) {
                if (param->hasName()) {
                    if (!checkVariableDeclaration(*param)) {
                        ok = false;
                    }
                }
            }
        }

        if (fn.isDefinition()) {
            // In C (ISO/IEC 9899 §6.2.1 p4), function parameters and declarations
            // in the outermost function body block share the exact same scope.
            if (fn.body().size() == 1 && dynamic_cast<const CompoundStmt*>(fn.body()[0].get())) {
                const auto* outerCompound = dynamic_cast<const CompoundStmt*>(fn.body()[0].get());
                for (const auto& stmt : outerCompound->statements()) {
                    if (stmt) {
                        if (!checkStatement(*stmt)) {
                            ok = false;
                        }
                    }
                }
            } else {
                for (const auto& stmt : fn.body()) {
                    if (stmt) {
                        if (!checkStatement(*stmt)) {
                            ok = false;
                        }
                    }
                }
            }
        }
    }

    return ok;
}

bool RedeclarationChecker::checkVarDecl(const VarDecl& varDecl) {
    const std::string& name = varDecl.name();
    const Symbol* existing = m_table.lookupCurrentScope(name);
    if (existing != nullptr) {
        reportDuplicate(name, varDecl.location(), existing);
        return false;
    }
    m_table.insert(varDecl);
    return true;
}

bool RedeclarationChecker::checkVariableDeclaration(const VariableDeclaration& varDecl) {
    if (!varDecl.hasName()) {
        return true;
    }
    const std::string& name = varDecl.name();
    const Symbol* existing = m_table.lookupCurrentScope(name);
    if (existing != nullptr) {
        reportDuplicate(name, varDecl.location(), existing);
        return false;
    }
    m_table.insert(varDecl);
    return true;
}

// =============================================================================
// Convenience Checking Functions
// =============================================================================

std::vector<Diagnostic> checkRedeclarations(const Program& program) {
    RedeclarationChecker checker;
    checker.check(program);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkRedeclarations(const Program& program, SymbolTable& table) {
    RedeclarationChecker checker(table);
    checker.check(program);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkRedeclarations(const Statement& stmt) {
    RedeclarationChecker checker;
    checker.check(stmt);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkRedeclarations(const Statement& stmt, SymbolTable& table) {
    RedeclarationChecker checker(table);
    checker.check(stmt);
    return checker.diagnostics();
}
