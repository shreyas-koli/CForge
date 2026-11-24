#include "semantic/checks/return_check.h"

#include "ast/literal_expression.h"

// =============================================================================
// ReturnChecker Implementation (Day 51)
// =============================================================================

ReturnChecker::ReturnChecker()
    : m_ownedTable(std::make_unique<SymbolTable>())
    , m_table(*m_ownedTable)
    , m_typeChecker(m_table)
    , m_currentFunction(nullptr)
{}

ReturnChecker::ReturnChecker(SymbolTable& table)
    : m_ownedTable(nullptr)
    , m_table(table)
    , m_typeChecker(table)
    , m_currentFunction(nullptr)
{}

void ReturnChecker::clear() {
    m_diagnostics.clear();
    m_table.clear();
    m_typeChecker.clear();
    m_currentFunction = nullptr;
}

void ReturnChecker::reportError(const std::string& message, SourceLocation loc, const std::string& id) {
    m_diagnostics.emplace_back(
        id,
        message,
        loc,
        SourceLocation{},
        false,
        DiagnosticSeverity::Error
    );
}

void ReturnChecker::reportWarning(const std::string& message, SourceLocation loc, const std::string& id) {
    m_diagnostics.emplace_back(
        id,
        message,
        loc,
        SourceLocation{},
        false,
        DiagnosticSeverity::Warning
    );
}

// -----------------------------------------------------------------------------
// Core Validation Logic
// -----------------------------------------------------------------------------

bool ReturnChecker::validateReturn(const ReturnStmt& ret,
                                   const Type& expectedType,
                                   const std::string& fnName) {
    SourceLocation loc = ret.location();

    // Rule D & E: Void function checks
    if (expectedType.isVoid()) {
        if (ret.hasExpression() && ret.expression()) {
            std::string msg = fnName.empty()
                ? "void function should not return a value"
                : "void function '" + fnName + "' should not return a value";
            reportError(msg, loc, fnName);
            return false;
        }
        return true;
    }

    // Rule F: Non-void function with empty return
    if (!ret.hasExpression() || !ret.expression()) {
        std::string msg = fnName.empty()
            ? "non-void function should return a value"
            : "non-void function '" + fnName + "' should return a value";
        reportError(msg, loc, fnName);
        return false;
    }

    const auto* expr = ret.expression();
    SourceLocation exprLoc = expr->location();
    if (exprLoc.line == 0) {
        exprLoc = loc;
    }

    // Check string literal return (Rule C)
    if (const auto* lit = dynamic_cast<const LiteralExpression*>(expr)) {
        if (lit->kind() == LiteralKind::String) {
            std::string msg = fnName.empty()
                ? "incompatible return type: cannot convert string literal to '" + expectedType.toString() + "'"
                : "incompatible return type in function '" + fnName + "': cannot convert string literal to '" + expectedType.toString() + "'";
            reportError(msg, exprLoc, fnName);
            return false;
        }
    }

    // Infer expression type using TypeChecker (Requirement 4)
    auto exprType = m_typeChecker.inferType(*expr);
    if (!exprType) {
        // If type inference failed (e.g. undeclared identifier or invalid op),
        // avoid cascading diagnostics (Requirement 7).
        return false;
    }

    // Check implicit conversion compatibility using Day 39 APIs (Requirement 5)
    if (!isImplicitlyConvertible(*exprType, expectedType)) {
        std::string msg = fnName.empty()
            ? "incompatible return type: cannot convert '" + exprType->toString() + "' to '" + expectedType.toString() + "'"
            : "incompatible return type in function '" + fnName + "': cannot convert '" + exprType->toString() + "' to '" + expectedType.toString() + "'";
        reportError(msg, exprLoc, fnName);
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
// Direct Validation Helpers
// -----------------------------------------------------------------------------

bool ReturnChecker::checkReturn(const ReturnStmt& ret,
                                const Type& expectedReturnType,
                                const std::string& fnName) {
    return validateReturn(ret, expectedReturnType, fnName);
}

bool ReturnChecker::checkReturn(const ReturnStmt& ret,
                                const std::shared_ptr<Type>& expectedReturnType,
                                const std::string& fnName) {
    if (!expectedReturnType) {
        Type defaultInt(TypeKind::Int);
        return validateReturn(ret, defaultInt, fnName);
    }
    return validateReturn(ret, *expectedReturnType, fnName);
}

bool ReturnChecker::checkReturn(const ReturnStmt& ret,
                                const Function& fn) {
    const Type* declType = fn.declaredReturnType();
    if (!declType) {
        Type defaultInt(TypeKind::Int);
        return validateReturn(ret, defaultInt, fn.name());
    }
    return validateReturn(ret, *declType, fn.name());
}

// -----------------------------------------------------------------------------
// Check Entry Points
// -----------------------------------------------------------------------------

bool ReturnChecker::check(const Program& program) {
    m_table.populate(program);
    return checkProgram(program);
}

bool ReturnChecker::check(const ASTNode& node) {
    if (const auto* prog = dynamic_cast<const Program*>(&node)) {
        return check(*prog);
    }
    if (const auto* fn = dynamic_cast<const Function*>(&node)) {
        return check(*fn);
    }
    if (const auto* comp = dynamic_cast<const CompoundStmt*>(&node)) {
        return check(*comp);
    }
    if (const auto* ret = dynamic_cast<const ReturnStmt*>(&node)) {
        return check(*ret);
    }
    if (const auto* ifStmt = dynamic_cast<const IfStmt*>(&node)) {
        return checkIfStmt(*ifStmt);
    }
    if (const auto* whileStmt = dynamic_cast<const WhileStmt*>(&node)) {
        return checkWhileStmt(*whileStmt);
    }
    if (const auto* doWhileStmt = dynamic_cast<const DoWhileStmt*>(&node)) {
        return checkDoWhileStmt(*doWhileStmt);
    }
    if (const auto* forStmt = dynamic_cast<const ForStmt*>(&node)) {
        return checkForStmt(*forStmt);
    }
    if (const auto* var = dynamic_cast<const VarDecl*>(&node)) {
        return check(*var);
    }
    if (const auto* varLegacy = dynamic_cast<const VariableDeclaration*>(&node)) {
        return check(*varLegacy);
    }
    if (const auto* stmt = dynamic_cast<const Statement*>(&node)) {
        return checkStatement(*stmt);
    }
    return true;
}

bool ReturnChecker::check(const Function& fn) {
    return checkFunction(fn);
}

bool ReturnChecker::check(const Statement& stmt) {
    return checkStatement(stmt);
}

bool ReturnChecker::check(const CompoundStmt& compound) {
    return checkCompoundStmt(compound);
}

bool ReturnChecker::check(const IfStmt& ifStmt) {
    return checkIfStmt(ifStmt);
}

bool ReturnChecker::check(const WhileStmt& whileStmt) {
    return checkWhileStmt(whileStmt);
}

bool ReturnChecker::check(const DoWhileStmt& doWhileStmt) {
    return checkDoWhileStmt(doWhileStmt);
}

bool ReturnChecker::check(const ForStmt& forStmt) {
    return checkForStmt(forStmt);
}

bool ReturnChecker::check(const ReturnStmt& ret) {
    return checkReturnStmt(ret);
}

bool ReturnChecker::check(const VarDecl& varDecl) {
    return checkVarDecl(varDecl);
}

bool ReturnChecker::check(const VariableDeclaration& varDecl) {
    return checkVariableDeclaration(varDecl);
}

// -----------------------------------------------------------------------------
// Internal Traversal
// -----------------------------------------------------------------------------

bool ReturnChecker::checkReturnStmt(const ReturnStmt& ret) {
    if (!m_currentFunction) {
        reportError("return statement outside of a function", ret.location());
        return false;
    }
    const Type* declType = m_currentFunction->declaredReturnType();
    if (!declType) {
        Type defaultInt(TypeKind::Int);
        return validateReturn(ret, defaultInt, m_currentFunction->name());
    }
    return validateReturn(ret, *declType, m_currentFunction->name());
}

bool ReturnChecker::checkProgram(const Program& program) {
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

bool ReturnChecker::checkFunction(const Function& fn) {
    const std::string& name = fn.name();
    const Symbol* existing = m_table.lookupCurrentScope(name);
    bool ok = true;

    if (existing == nullptr) {
        m_table.insert(fn);
    } else if (existing->isFunction() && !existing->isDefinition() && fn.isDefinition()) {
        auto* nonConstSym = m_table.lookupCurrentScope(name);
        if (nonConstSym) {
            nonConstSym->setIsDefinition(true);
            nonConstSym->setASTNode(&fn);
        }
    }

    const Function* prevFn = m_currentFunction;
    m_currentFunction = &fn;

    if (fn.isDefinition() || fn.parameterCount() > 0) {
        ScopeGuard guard(m_table);

        for (std::size_t i = 0; i < fn.parameterCount(); ++i) {
            if (const auto* param = fn.parameter(i)) {
                if (param->hasName()) {
                    m_table.insert(*param);
                }
            }
        }

        if (fn.isDefinition()) {
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

    m_currentFunction = prevFn;
    return ok;
}

bool ReturnChecker::checkStatement(const Statement& stmt) {
    if (const auto* ret = dynamic_cast<const ReturnStmt*>(&stmt)) {
        return checkReturnStmt(*ret);
    }
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
        return checkIfStmt(*ifStmt);
    }
    if (const auto* whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) {
        return checkWhileStmt(*whileStmt);
    }
    if (const auto* doWhileStmt = dynamic_cast<const DoWhileStmt*>(&stmt)) {
        return checkDoWhileStmt(*doWhileStmt);
    }
    if (const auto* forStmt = dynamic_cast<const ForStmt*>(&stmt)) {
        return checkForStmt(*forStmt);
    }
    return true;
}

bool ReturnChecker::checkCompoundStmt(const CompoundStmt& compound) {
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

bool ReturnChecker::checkIfStmt(const IfStmt& ifStmt) {
    bool ok = true;
    if (ifStmt.thenBranch() && !checkStatement(*ifStmt.thenBranch())) {
        ok = false;
    }
    if (ifStmt.elseBranch() && !checkStatement(*ifStmt.elseBranch())) {
        ok = false;
    }
    return ok;
}

bool ReturnChecker::checkWhileStmt(const WhileStmt& whileStmt) {
    if (whileStmt.body()) {
        return checkStatement(*whileStmt.body());
    }
    return true;
}

bool ReturnChecker::checkDoWhileStmt(const DoWhileStmt& doWhileStmt) {
    if (doWhileStmt.body()) {
        return checkStatement(*doWhileStmt.body());
    }
    return true;
}

bool ReturnChecker::checkForStmt(const ForStmt& forStmt) {
    if (forStmt.body()) {
        return checkStatement(*forStmt.body());
    }
    return true;
}

bool ReturnChecker::checkVarDecl(const VarDecl& varDecl) {
    m_table.insert(varDecl);
    return true;
}

bool ReturnChecker::checkVariableDeclaration(const VariableDeclaration& varDecl) {
    m_table.insert(varDecl);
    return true;
}

// -----------------------------------------------------------------------------
// Missing-Return Analysis (Structural)
// -----------------------------------------------------------------------------

bool ReturnChecker::hasReturn(const Statement& stmt) {
    if (dynamic_cast<const ReturnStmt*>(&stmt)) {
        return true;
    }
    if (const auto* comp = dynamic_cast<const CompoundStmt*>(&stmt)) {
        return hasReturn(*comp);
    }
    if (const auto* ifStmt = dynamic_cast<const IfStmt*>(&stmt)) {
        bool thenHas = ifStmt->thenBranch() && hasReturn(*ifStmt->thenBranch());
        bool elseHas = ifStmt->elseBranch() && hasReturn(*ifStmt->elseBranch());
        return thenHas || elseHas;
    }
    if (const auto* whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) {
        return whileStmt->body() && hasReturn(*whileStmt->body());
    }
    if (const auto* doWhileStmt = dynamic_cast<const DoWhileStmt*>(&stmt)) {
        return doWhileStmt->body() && hasReturn(*doWhileStmt->body());
    }
    if (const auto* forStmt = dynamic_cast<const ForStmt*>(&stmt)) {
        return forStmt->body() && hasReturn(*forStmt->body());
    }
    return false;
}

bool ReturnChecker::hasReturn(const CompoundStmt& compound) {
    for (const auto& stmt : compound.statements()) {
        if (stmt && hasReturn(*stmt)) {
            return true;
        }
    }
    return false;
}

bool ReturnChecker::hasReturn(const Function& fn) {
    for (const auto& stmt : fn.body()) {
        if (stmt && hasReturn(*stmt)) {
            return true;
        }
    }
    return false;
}

bool ReturnChecker::checkMissingReturn(const Function& fn) {
    if (!fn.isDefinition() || fn.isReturnVoid()) {
        return true;
    }
    // Main function in C99/C17 has an implicit 'return 0;' at end of body (§5.1.2.2.3)
    if (fn.name() == "main") {
        return true;
    }
    if (!hasReturn(fn)) {
        reportError("non-void function '" + fn.name() + "' does not return a value",
                    fn.location(), fn.name());
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
// Convenience Checking Functions
// -----------------------------------------------------------------------------

std::vector<Diagnostic> checkReturns(const Program& program) {
    ReturnChecker checker;
    checker.check(program);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkReturns(const Program& program, SymbolTable& table) {
    ReturnChecker checker(table);
    checker.check(program);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkReturns(const Function& fn) {
    ReturnChecker checker;
    checker.check(fn);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkReturns(const Function& fn, SymbolTable& table) {
    ReturnChecker checker(table);
    checker.check(fn);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkReturns(const ReturnStmt& ret, const Type& expectedType, const std::string& fnName) {
    ReturnChecker checker;
    checker.checkReturn(ret, expectedType, fnName);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkReturns(const ReturnStmt& ret, const Type& expectedType, SymbolTable& table, const std::string& fnName) {
    ReturnChecker checker(table);
    checker.checkReturn(ret, expectedType, fnName);
    return checker.diagnostics();
}
