#include "semantic/checks/undeclared.h"
#include "ast/call_expr.h"

// =============================================================================
// UndeclaredChecker Implementation (Day 48)
// =============================================================================

UndeclaredChecker::UndeclaredChecker()
    : m_ownedTable(std::make_unique<SymbolTable>())
    , m_table(*m_ownedTable)
{}

UndeclaredChecker::UndeclaredChecker(SymbolTable& table)
    : m_ownedTable(nullptr)
    , m_table(table)
{}

void UndeclaredChecker::clear() {
    m_diagnostics.clear();
    m_table.clear();
}

void UndeclaredChecker::reportUndeclared(const std::string& name, SourceLocation loc) {
    m_diagnostics.emplace_back(
        name,
        "use of undeclared identifier '" + name + "'",
        loc,
        SourceLocation{},
        false,
        DiagnosticSeverity::Error
    );
}

void UndeclaredChecker::reportWarning(const std::string& message, SourceLocation loc, const std::string& id) {
    m_diagnostics.emplace_back(id, message, loc, SourceLocation{}, false, DiagnosticSeverity::Warning);
}

void UndeclaredChecker::reportError(const std::string& message, SourceLocation loc, const std::string& id) {
    m_diagnostics.emplace_back(id, message, loc, SourceLocation{}, false, DiagnosticSeverity::Error);
}

// -----------------------------------------------------------------------------
// Check Entry Points
// -----------------------------------------------------------------------------

bool UndeclaredChecker::check(const Program& program) {
    return checkProgram(program);
}

bool UndeclaredChecker::check(const ASTNode& node) {
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
    if (const auto* stmt = dynamic_cast<const Statement*>(&node)) {
        return checkStatement(*stmt);
    }
    if (const auto* expr = dynamic_cast<const Expression*>(&node)) {
        return checkExpression(*expr);
    }
    return true;
}

bool UndeclaredChecker::check(const Statement& stmt) {
    return checkStatement(stmt);
}

bool UndeclaredChecker::check(const Expression& expr) {
    return checkExpression(expr);
}

bool UndeclaredChecker::check(const CompoundStmt& compound) {
    return checkCompoundStmt(compound);
}

bool UndeclaredChecker::check(const Function& fn) {
    return checkFunction(fn);
}

bool UndeclaredChecker::check(const VarDecl& varDecl) {
    return checkVarDecl(varDecl);
}

bool UndeclaredChecker::check(const VariableDeclaration& varDecl) {
    return checkVariableDeclaration(varDecl);
}

bool UndeclaredChecker::check(const IdentifierExpression& idExpr) {
    return checkIdentifier(idExpr);
}

bool UndeclaredChecker::check(const AssignExpr& assign) {
    return checkAssignExpr(assign);
}

bool UndeclaredChecker::check(const BinaryExpr& binary) {
    return checkBinaryExpr(binary);
}

bool UndeclaredChecker::check(const UnaryExpr& unary) {
    return checkUnaryExpr(unary);
}

bool UndeclaredChecker::check(const ConditionalExpr& cond) {
    return checkConditionalExpr(cond);
}

bool UndeclaredChecker::check(const CallExpr& call) {
    return checkCallExpr(call);
}

bool UndeclaredChecker::checkCallExpr(const CallExpr& call) {
    bool ok = true;
    // Callee function identifier resolution and callable verification are owned by CallChecker
    // to prevent duplicate undeclared-identifier diagnostics when multiple semantic
    // checkers run. For non-identifier callee expressions (e.g. nested calls or complex
    // expressions), recursively traverse.
    if (call.callee() && !dynamic_cast<const IdentifierExpression*>(call.callee())) {
        if (!checkExpression(*call.callee())) {
            ok = false;
        }
    }
    for (const auto& arg : call.arguments()) {
        if (arg && !checkExpression(*arg)) {
            ok = false;
        }
    }
    return ok;
}

bool UndeclaredChecker::check(const IfStmt& ifStmt) {
    return checkIfStmt(ifStmt);
}

bool UndeclaredChecker::check(const WhileStmt& whileStmt) {
    return checkWhileStmt(whileStmt);
}

bool UndeclaredChecker::check(const DoWhileStmt& doWhileStmt) {
    return checkDoWhileStmt(doWhileStmt);
}

bool UndeclaredChecker::check(const ForStmt& forStmt) {
    return checkForStmt(forStmt);
}

// -----------------------------------------------------------------------------
// Internal Traversal Methods
// -----------------------------------------------------------------------------

bool UndeclaredChecker::checkProgram(const Program& program) {
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

bool UndeclaredChecker::checkStatement(const Statement& stmt) {
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
    if (const auto* ret = dynamic_cast<const ReturnStmt*>(&stmt)) {
        if (ret->hasExpression() && ret->expression()) {
            return checkExpression(*ret->expression());
        }
        return true;
    }
    // BreakStmt, ContinueStmt contain no identifiers
    return true;
}

bool UndeclaredChecker::checkExpression(const Expression& expr) {
    if (const auto* id = dynamic_cast<const IdentifierExpression*>(&expr)) {
        return checkIdentifier(*id);
    }
    if (const auto* assign = dynamic_cast<const AssignExpr*>(&expr)) {
        return checkAssignExpr(*assign);
    }
    if (const auto* binary = dynamic_cast<const BinaryExpr*>(&expr)) {
        return checkBinaryExpr(*binary);
    }
    if (const auto* unary = dynamic_cast<const UnaryExpr*>(&expr)) {
        return checkUnaryExpr(*unary);
    }
    if (const auto* cond = dynamic_cast<const ConditionalExpr*>(&expr)) {
        return checkConditionalExpr(*cond);
    }
    if (const auto* call = dynamic_cast<const CallExpr*>(&expr)) {
        return checkCallExpr(*call);
    }
    // LiteralExpression contains no identifiers
    return true;
}

bool UndeclaredChecker::checkCompoundStmt(const CompoundStmt& compound) {
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

bool UndeclaredChecker::checkFunction(const Function& fn) {
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

    // Function definition and parameter scope handling
    if (fn.isDefinition() || fn.parameterCount() > 0) {
        ScopeGuard guard(m_table);

        for (std::size_t i = 0; i < fn.parameterCount(); ++i) {
            if (const auto* param = fn.parameter(i)) {
                if (param->hasName()) {
                    m_table.insert(*param);
                    if (param->hasInitializer() && param->initializer()) {
                        if (!checkExpression(*param->initializer())) {
                            ok = false;
                        }
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

bool UndeclaredChecker::checkVarDecl(const VarDecl& varDecl) {
    // Register the declared identifier in the current scope
    m_table.insert(varDecl);

    // Validate the initializer expression if present
    if (varDecl.hasInitializer() && varDecl.initializer()) {
        return checkExpression(*varDecl.initializer());
    }
    return true;
}

bool UndeclaredChecker::checkVariableDeclaration(const VariableDeclaration& varDecl) {
    // Register the declared identifier in the current scope
    if (varDecl.hasName()) {
        m_table.insert(varDecl);
    }

    // Validate the initializer expression if present
    if (varDecl.hasInitializer() && varDecl.initializer()) {
        return checkExpression(*varDecl.initializer());
    }
    return true;
}

bool UndeclaredChecker::checkIdentifier(const IdentifierExpression& idExpr) {
    const std::string& name = idExpr.name();
    const Symbol* sym = m_table.lookup(name);
    if (sym == nullptr) {
        reportUndeclared(name, idExpr.location());
        return false;
    }
    return true;
}

bool UndeclaredChecker::checkAssignExpr(const AssignExpr& assign) {
    bool ok = true;
    if (assign.left()) {
        if (!checkExpression(*assign.left())) {
            ok = false;
        }
    }
    if (assign.right()) {
        if (!checkExpression(*assign.right())) {
            ok = false;
        }
    }
    return ok;
}

bool UndeclaredChecker::checkBinaryExpr(const BinaryExpr& binary) {
    bool ok = true;
    if (binary.left()) {
        if (!checkExpression(*binary.left())) {
            ok = false;
        }
    }
    if (binary.right()) {
        if (!checkExpression(*binary.right())) {
            ok = false;
        }
    }
    return ok;
}

bool UndeclaredChecker::checkUnaryExpr(const UnaryExpr& unary) {
    if (unary.operand()) {
        return checkExpression(*unary.operand());
    }
    return true;
}

bool UndeclaredChecker::checkConditionalExpr(const ConditionalExpr& cond) {
    bool ok = true;
    if (cond.condition()) {
        if (!checkExpression(*cond.condition())) {
            ok = false;
        }
    }
    if (cond.thenExpr()) {
        if (!checkExpression(*cond.thenExpr())) {
            ok = false;
        }
    }
    if (cond.elseExpr()) {
        if (!checkExpression(*cond.elseExpr())) {
            ok = false;
        }
    }
    return ok;
}

bool UndeclaredChecker::checkIfStmt(const IfStmt& ifStmt) {
    bool ok = true;
    if (ifStmt.condition()) {
        if (!checkExpression(*ifStmt.condition())) {
            ok = false;
        }
    }
    if (ifStmt.thenBranch()) {
        if (!checkStatement(*ifStmt.thenBranch())) {
            ok = false;
        }
    }
    if (ifStmt.hasElse() && ifStmt.elseBranch()) {
        if (!checkStatement(*ifStmt.elseBranch())) {
            ok = false;
        }
    }
    return ok;
}

bool UndeclaredChecker::checkWhileStmt(const WhileStmt& whileStmt) {
    bool ok = true;
    if (whileStmt.condition()) {
        if (!checkExpression(*whileStmt.condition())) {
            ok = false;
        }
    }
    if (whileStmt.body()) {
        if (!checkStatement(*whileStmt.body())) {
            ok = false;
        }
    }
    return ok;
}

bool UndeclaredChecker::checkDoWhileStmt(const DoWhileStmt& doWhileStmt) {
    bool ok = true;
    if (doWhileStmt.body()) {
        if (!checkStatement(*doWhileStmt.body())) {
            ok = false;
        }
    }
    if (doWhileStmt.condition()) {
        if (!checkExpression(*doWhileStmt.condition())) {
            ok = false;
        }
    }
    return ok;
}

bool UndeclaredChecker::checkForStmt(const ForStmt& forStmt) {
    bool ok = true;
    if (forStmt.initializer()) {
        if (!checkExpression(*forStmt.initializer())) {
            ok = false;
        }
    }
    if (forStmt.condition()) {
        if (!checkExpression(*forStmt.condition())) {
            ok = false;
        }
    }
    if (forStmt.post()) {
        if (!checkExpression(*forStmt.post())) {
            ok = false;
        }
    }
    if (forStmt.body()) {
        if (!checkStatement(*forStmt.body())) {
            ok = false;
        }
    }
    return ok;
}

// =============================================================================
// Convenience Checking Functions
// =============================================================================

std::vector<Diagnostic> checkUndeclared(const Program& program) {
    UndeclaredChecker checker;
    checker.check(program);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkUndeclared(const Program& program, SymbolTable& table) {
    UndeclaredChecker checker(table);
    checker.check(program);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkUndeclared(const Statement& stmt) {
    UndeclaredChecker checker;
    checker.check(stmt);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkUndeclared(const Statement& stmt, SymbolTable& table) {
    UndeclaredChecker checker(table);
    checker.check(stmt);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkUndeclared(const Expression& expr) {
    UndeclaredChecker checker;
    checker.check(expr);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkUndeclared(const Expression& expr, SymbolTable& table) {
    UndeclaredChecker checker(table);
    checker.check(expr);
    return checker.diagnostics();
}
