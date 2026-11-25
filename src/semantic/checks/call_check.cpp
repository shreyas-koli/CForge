#include "semantic/checks/call_check.h"

#include "ast/assign_expr.h"
#include "ast/binary_expr.h"
#include "ast/conditional_expr.h"
#include "ast/do_while_stmt.h"
#include "ast/for_stmt.h"
#include "ast/if_stmt.h"
#include "ast/index_expr.h"
#include "ast/literal_expression.h"
#include "ast/return_stmt.h"
#include "ast/unary_expr.h"
#include "ast/while_stmt.h"

#include <algorithm>

// =============================================================================
// CallChecker Implementation (Day 50)
// =============================================================================

CallChecker::CallChecker()
    : m_ownedTable(std::make_unique<SymbolTable>())
    , m_table(*m_ownedTable)
    , m_typeChecker(m_table)
{}

CallChecker::CallChecker(SymbolTable& table)
    : m_ownedTable(nullptr)
    , m_table(table)
    , m_typeChecker(table)
{}

void CallChecker::clear() {
    m_diagnostics.clear();
    m_table.clear();
    m_typeChecker.clear();
}

void CallChecker::reportError(const std::string& message, SourceLocation loc, const std::string& id) {
    m_diagnostics.emplace_back(
        id,
        message,
        loc,
        SourceLocation{},
        false,
        DiagnosticSeverity::Error
    );
}

void CallChecker::reportWarning(const std::string& message, SourceLocation loc, const std::string& id) {
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
// Check Entry Points
// -----------------------------------------------------------------------------

bool CallChecker::check(const Program& program) {
    m_table.populate(program);
    return checkProgram(program);
}

bool CallChecker::check(const ASTNode& node) {
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
    if (const auto* call = dynamic_cast<const CallExpr*>(&node)) {
        return check(*call);
    }
    if (const auto* expr = dynamic_cast<const Expression*>(&node)) {
        return check(*expr);
    }
    if (const auto* stmt = dynamic_cast<const Statement*>(&node)) {
        return check(*stmt);
    }
    return true;
}

bool CallChecker::check(const Statement& stmt) {
    return checkStatement(stmt);
}

bool CallChecker::check(const Expression& expr) {
    return checkExpression(expr);
}

bool CallChecker::check(const CompoundStmt& compound) {
    return checkCompoundStmt(compound);
}

bool CallChecker::check(const Function& fn) {
    return checkFunction(fn);
}

bool CallChecker::check(const VarDecl& varDecl) {
    return checkVarDecl(varDecl);
}

bool CallChecker::check(const VariableDeclaration& varDecl) {
    return checkVariableDeclaration(varDecl);
}

bool CallChecker::check(const CallExpr& call) {
    return checkCall(call);
}

bool CallChecker::checkCall(const CallExpr& call) {
    bool calleeOk = true;
    if (call.callee() && !dynamic_cast<const IdentifierExpression*>(call.callee())) {
        if (!checkExpression(*call.callee())) {
            calleeOk = false;
        }
    }

    const std::string& name = call.calleeName();
    SourceLocation loc = call.location();
    if (loc.line == 0 && call.callee()) {
        loc = call.callee()->location();
    }

    std::vector<const Expression*> rawArgs;
    rawArgs.reserve(call.argumentCount());
    for (const auto& arg : call.arguments()) {
        rawArgs.push_back(arg.get());
    }

    bool callOk = validateCall(name, rawArgs, loc);
    return calleeOk && callOk;
}

bool CallChecker::checkCall(const std::string& calleeName,
                            const std::vector<std::unique_ptr<Expression>>& args,
                            SourceLocation loc) {
    std::vector<const Expression*> rawArgs;
    rawArgs.reserve(args.size());
    for (const auto& arg : args) {
        rawArgs.push_back(arg.get());
    }
    return validateCall(calleeName, rawArgs, loc);
}

bool CallChecker::checkCall(const std::string& calleeName,
                            const std::vector<const Expression*>& args,
                            SourceLocation loc) {
    return validateCall(calleeName, args, loc);
}

// -----------------------------------------------------------------------------
// Validation Logic
// -----------------------------------------------------------------------------

bool CallChecker::validateCall(const std::string& calleeName,
                               const std::vector<const Expression*>& args,
                               SourceLocation loc) {
    // 1. Recursively validate argument expressions first (e.g. inner nested calls)
    bool argsWellFormed = true;
    for (const auto* arg : args) {
        if (arg) {
            if (!checkExpression(*arg)) {
                argsWellFormed = false;
            }
        }
    }

    if (calleeName.empty()) {
        reportError("called object is not a function", loc);
        return false;
    }

    // 2. Resolve the called function in the symbol table
    const Symbol* sym = m_table.lookup(calleeName);
    if (!sym) {
        // Undeclared function:
        // Do not report misleading argument errors if the function identifier is undeclared.
        reportError("use of undeclared identifier '" + calleeName + "'", loc, calleeName);
        return false;
    }

    // 3. Ensure the symbol is actually a function
    if (!sym->isFunction()) {
        // Calling a variable as if it were a function
        reportError("called object '" + calleeName + "' is not a function", loc, calleeName);
        return false;
    }

    // 4. Validate argument count against function signature
    std::size_t argCount = args.size();
    bool countValid = true;

    if (sym->hasVoidParameterList()) {
        // Function explicitly declared with (void), e.g. "void display(void);"
        if (argCount > 0) {
            reportError("too many arguments to function '" + calleeName + "'", loc, calleeName);
            countValid = false;
        }
    } else if (sym->parameterListKind() == ParameterListKind::Typed || sym->parameterCount() > 0) {
        std::size_t expectedCount = sym->parameterCount();
        if (argCount < expectedCount) {
            reportError("too few arguments to function '" + calleeName + "'", loc, calleeName);
            countValid = false;
        } else if (argCount > expectedCount) {
            reportError("too many arguments to function '" + calleeName + "'", loc, calleeName);
            countValid = false;
        }
    } else if (sym->hasUnspecifiedParameters()) {
        // Unspecified parameter list: "int f();" takes unspecified arguments (C17)
    }

    // If argument count mismatch occurred, do not report misleading type errors
    if (!countValid) {
        return false;
    }

    // 5. Validate argument types against expected parameter types
    bool typesValid = true;
    if (sym->parameterListKind() == ParameterListKind::Typed || sym->parameterCount() > 0) {
        std::size_t checkCount = std::min(argCount, sym->parameterCount());
        for (std::size_t i = 0; i < checkCount; ++i) {
            const auto* arg = args[i];
            if (!arg) continue;

            const auto& paramType = sym->parameterShared(i);
            if (!paramType) continue;

            // Check if argument is a string literal (currently unsupported type in CForge)
            if (const auto* lit = dynamic_cast<const LiteralExpression*>(arg)) {
                if (lit->kind() == LiteralKind::String) {
                    reportError("incompatible type for argument " + std::to_string(i + 1) +
                                " of '" + calleeName + "': cannot convert string literal to '" +
                                paramType->toString() + "'",
                                arg->location(), calleeName);
                    typesValid = false;
                    continue;
                }
            }

            // Infer argument type using TypeChecker
            auto argType = m_typeChecker.inferType(*arg);
            if (!argType) {
                // If type inference failed or returned nullptr, avoid cascading errors
                continue;
            }

            // Check conversion compatibility using Day 39 implicit conversions
            if (!isImplicitlyConvertible(*argType, *paramType)) {
                reportError("incompatible type for argument " + std::to_string(i + 1) +
                            " of '" + calleeName + "': cannot convert '" +
                            argType->toString() + "' to '" + paramType->toString() + "'",
                            arg->location(), calleeName);
                typesValid = false;
            }
        }
    }

    return countValid && typesValid && argsWellFormed;
}

// -----------------------------------------------------------------------------
// AST Traversal
// -----------------------------------------------------------------------------

bool CallChecker::checkProgram(const Program& program) {
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

bool CallChecker::checkStatement(const Statement& stmt) {
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
        if (ifStmt->condition() && !checkExpression(*ifStmt->condition())) ok = false;
        if (ifStmt->thenBranch() && !checkStatement(*ifStmt->thenBranch())) ok = false;
        if (ifStmt->elseBranch() && !checkStatement(*ifStmt->elseBranch())) ok = false;
        return ok;
    }
    if (const auto* whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) {
        bool ok = true;
        if (whileStmt->condition() && !checkExpression(*whileStmt->condition())) ok = false;
        if (whileStmt->body() && !checkStatement(*whileStmt->body())) ok = false;
        return ok;
    }
    if (const auto* doWhileStmt = dynamic_cast<const DoWhileStmt*>(&stmt)) {
        bool ok = true;
        if (doWhileStmt->body() && !checkStatement(*doWhileStmt->body())) ok = false;
        if (doWhileStmt->condition() && !checkExpression(*doWhileStmt->condition())) ok = false;
        return ok;
    }
    if (const auto* forStmt = dynamic_cast<const ForStmt*>(&stmt)) {
        bool ok = true;
        if (forStmt->hasInitializer() && !checkExpression(*forStmt->initializer())) ok = false;
        if (forStmt->hasCondition() && !checkExpression(*forStmt->condition())) ok = false;
        if (forStmt->hasPost() && !checkExpression(*forStmt->post())) ok = false;
        if (forStmt->body() && !checkStatement(*forStmt->body())) ok = false;
        return ok;
    }
    if (const auto* ret = dynamic_cast<const ReturnStmt*>(&stmt)) {
        if (ret->hasExpression() && ret->expression()) {
            return checkExpression(*ret->expression());
        }
        return true;
    }
    return true;
}

bool CallChecker::checkCompoundStmt(const CompoundStmt& compound) {
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

bool CallChecker::checkFunction(const Function& fn) {
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
            // Function parameters and outermost body block share the same scope
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

bool CallChecker::checkVarDecl(const VarDecl& varDecl) {
    bool ok = true;
    if (varDecl.hasInitializer() && varDecl.initializer()) {
        if (!checkExpression(*varDecl.initializer())) {
            ok = false;
        }
    }
    m_table.insert(varDecl);
    return ok;
}

bool CallChecker::checkVariableDeclaration(const VariableDeclaration& varDecl) {
    bool ok = true;
    if (varDecl.hasInitializer() && varDecl.initializer()) {
        if (!checkExpression(*varDecl.initializer())) {
            ok = false;
        }
    }
    m_table.insert(varDecl);
    return ok;
}

bool CallChecker::checkExpression(const Expression& expr) {
    if (const auto* call = dynamic_cast<const CallExpr*>(&expr)) {
        return checkCall(*call);
    }
    if (const auto* bin = dynamic_cast<const BinaryExpr*>(&expr)) {
        bool ok = true;
        if (bin->left() && !checkExpression(*bin->left())) ok = false;
        if (bin->right() && !checkExpression(*bin->right())) ok = false;
        return ok;
    }
    if (const auto* un = dynamic_cast<const UnaryExpr*>(&expr)) {
        return un->operand() ? checkExpression(*un->operand()) : true;
    }
    if (const auto* assign = dynamic_cast<const AssignExpr*>(&expr)) {
        bool ok = true;
        if (assign->left() && !checkExpression(*assign->left())) ok = false;
        if (assign->right() && !checkExpression(*assign->right())) ok = false;
        return ok;
    }
    if (const auto* cond = dynamic_cast<const ConditionalExpr*>(&expr)) {
        bool ok = true;
        if (cond->condition() && !checkExpression(*cond->condition())) ok = false;
        if (cond->thenExpr() && !checkExpression(*cond->thenExpr())) ok = false;
        if (cond->elseExpr() && !checkExpression(*cond->elseExpr())) ok = false;
        return ok;
    }
    if (const auto* idx = dynamic_cast<const IndexExpr*>(&expr)) {
        bool ok = true;
        if (idx->base() && !checkExpression(*idx->base())) ok = false;
        if (idx->index() && !checkExpression(*idx->index())) ok = false;
        return ok;
    }
    return true;
}

// -----------------------------------------------------------------------------
// Convenience Functions
// -----------------------------------------------------------------------------

std::vector<Diagnostic> checkCalls(const Expression& expr) {
    CallChecker checker;
    checker.check(expr);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkCalls(const Expression& expr, SymbolTable& table) {
    CallChecker checker(table);
    checker.check(expr);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkCalls(const Program& program) {
    CallChecker checker;
    checker.check(program);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkCalls(const Program& program, SymbolTable& table) {
    CallChecker checker(table);
    checker.check(program);
    return checker.diagnostics();
}
