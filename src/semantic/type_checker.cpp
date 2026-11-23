#include "semantic/type_checker.h"
#include "ast/call_expr.h"

// =============================================================================
// TypeChecker Implementation (Day 49)
// =============================================================================

TypeChecker::TypeChecker()
    : m_ownedTable(std::make_unique<SymbolTable>())
    , m_table(*m_ownedTable)
{}

TypeChecker::TypeChecker(SymbolTable& table)
    : m_ownedTable(nullptr)
    , m_table(table)
{}

void TypeChecker::clear() {
    m_diagnostics.clear();
    m_table.clear();
}

void TypeChecker::reportError(const std::string& message, SourceLocation loc, const std::string& id) {
    m_diagnostics.emplace_back(
        id,
        message,
        loc
    );
}

// -----------------------------------------------------------------------------
// Common Type Computation (C17 §6.3.1.8 Usual Arithmetic Conversions)
// -----------------------------------------------------------------------------

std::shared_ptr<Type> TypeChecker::commonType(const Type& t1, const Type& t2) {
    if (!t1.isArithmetic() || !t2.isArithmetic()) {
        return nullptr;
    }
    if (t1.isDouble() || t2.isDouble()) {
        return Type::getDouble();
    }
    if (t1.isFloat() || t2.isFloat()) {
        return Type::getFloat();
    }
    // Both are integer types (char or int); integer promotion promotes char to int
    return Type::getInt();
}

std::shared_ptr<Type> TypeChecker::commonType(const std::shared_ptr<Type>& t1,
                                              const std::shared_ptr<Type>& t2) {
    if (!t1 || !t2) {
        return nullptr;
    }
    return commonType(*t1, *t2);
}

bool TypeChecker::isAssignable(const Type& source, const Type& destination) {
    return isImplicitlyConvertible(source, destination);
}

bool TypeChecker::isAssignable(const std::shared_ptr<Type>& source,
                               const std::shared_ptr<Type>& destination) {
    if (!source || !destination) {
        return false;
    }
    return isImplicitlyConvertible(*source, *destination);
}

bool TypeChecker::isStrictlyImplicitConversion(const Type& source, const Type& destination) {
    return isStrictlyImplicit(source, destination);
}

bool TypeChecker::isStrictlyImplicitConversion(const std::shared_ptr<Type>& source,
                                               const std::shared_ptr<Type>& destination) {
    if (!source || !destination) {
        return false;
    }
    return isStrictlyImplicit(*source, *destination);
}

// -----------------------------------------------------------------------------
// Expression Type Inference
// -----------------------------------------------------------------------------

std::shared_ptr<Type> TypeChecker::inferType(const Expression& expr) {
    if (const auto* lit = dynamic_cast<const LiteralExpression*>(&expr)) {
        return inferLiteral(*lit);
    }
    if (const auto* id = dynamic_cast<const IdentifierExpression*>(&expr)) {
        return inferIdentifier(*id);
    }
    if (const auto* bin = dynamic_cast<const BinaryExpr*>(&expr)) {
        return inferBinary(*bin);
    }
    if (const auto* unary = dynamic_cast<const UnaryExpr*>(&expr)) {
        return inferUnary(*unary);
    }
    if (const auto* assign = dynamic_cast<const AssignExpr*>(&expr)) {
        return inferAssign(*assign);
    }
    if (const auto* cond = dynamic_cast<const ConditionalExpr*>(&expr)) {
        return inferConditional(*cond);
    }
    if (const auto* call = dynamic_cast<const CallExpr*>(&expr)) {
        return inferCall(*call);
    }
    return nullptr;
}

std::shared_ptr<Type> TypeChecker::inferType(const ASTNode& node) {
    if (const auto* expr = dynamic_cast<const Expression*>(&node)) {
        return inferType(*expr);
    }
    return nullptr;
}

std::shared_ptr<Type> TypeChecker::inferLiteral(const LiteralExpression& lit) {
    switch (lit.kind()) {
        case LiteralKind::Integer:
            return Type::getInt();

        case LiteralKind::Char:
            return Type::getChar();

        case LiteralKind::Float: {
            const std::string& val = lit.value();
            if (!val.empty()) {
                char suffix = val.back();
                if (suffix == 'f' || suffix == 'F') {
                    return Type::getFloat();
                }
                if (suffix == 'l' || suffix == 'L') {
                    reportError("unsupported literal suffix '" + std::string(1, suffix) + "' ('long double' is not supported)", lit.location());
                    return nullptr;
                }
            }
            return Type::getDouble();
        }

        case LiteralKind::String:
            // CForge does not currently support pointer or string types
            return nullptr;

        default:
            return nullptr;
    }
}

std::shared_ptr<Type> TypeChecker::inferIdentifier(const IdentifierExpression& id) {
    const Symbol* sym = m_table.lookup(id.name());
    if (!sym) {
        // Do not report type error for undeclared identifier to avoid cascading diagnostics
        return nullptr;
    }
    return sym->getType();
}

std::shared_ptr<Type> TypeChecker::inferBinary(const BinaryExpr& bin) {
    if (!bin.left() || !bin.right()) {
        return nullptr;
    }

    auto leftType = inferType(*bin.left());
    auto rightType = inferType(*bin.right());
    if (!leftType || !rightType) {
        return nullptr;
    }

    const std::string& op = bin.op();

    // 1. Modulo operator (%) requires integer operands
    if (op == "%") {
        if (!leftType->isInteger() || !rightType->isInteger()) {
            reportError("invalid operands to binary operator '%' (have '" +
                        leftType->toString() + "' and '" + rightType->toString() + "')",
                        bin.location());
            return nullptr;
        }
        return Type::getInt();
    }

    // 2. Bitwise operators (&, |, ^, <<, >>) require integer operands
    if (op == "&" || op == "|" || op == "^" || op == "<<" || op == ">>") {
        if (!leftType->isInteger() || !rightType->isInteger()) {
            reportError("invalid operands to binary operator '" + op + "' (have '" +
                        leftType->toString() + "' and '" + rightType->toString() + "')",
                        bin.location());
            return nullptr;
        }
        return Type::getInt();
    }

    // 3. Relational comparisons (<, <=, >, >=) require arithmetic operands and yield int
    if (op == "<" || op == "<=" || op == ">" || op == ">=") {
        if (!leftType->isArithmetic() || !rightType->isArithmetic()) {
            reportError("invalid operands to comparison operator '" + op + "' (have '" +
                        leftType->toString() + "' and '" + rightType->toString() + "')",
                        bin.location());
            return nullptr;
        }
        return Type::getInt();
    }

    // 4. Equality operators (==, !=) require compatible arithmetic operands and yield int
    if (op == "==" || op == "!=") {
        if (!leftType->isArithmetic() || !rightType->isArithmetic()) {
            reportError("comparison between incompatible types ('" +
                        leftType->toString() + "' and '" + rightType->toString() + "')",
                        bin.location());
            return nullptr;
        }
        return Type::getInt();
    }

    // 5. Logical operators (&&, ||) require scalar operands and yield int
    if (op == "&&" || op == "||") {
        if (!leftType->isArithmetic() || !rightType->isArithmetic()) {
            reportError("invalid operands to logical operator '" + op + "' (have '" +
                        leftType->toString() + "' and '" + rightType->toString() + "')",
                        bin.location());
            return nullptr;
        }
        return Type::getInt();
    }

    // 6. Arithmetic operators (+, -, *, /) require arithmetic operands
    if (op == "+" || op == "-" || op == "*" || op == "/") {
        auto common = commonType(*leftType, *rightType);
        if (!common) {
            reportError("invalid operands to binary operator '" + op + "' (have '" +
                        leftType->toString() + "' and '" + rightType->toString() + "')",
                        bin.location());
            return nullptr;
        }
        return common;
    }

    reportError("unsupported binary operator '" + op + "'", bin.location());
    return nullptr;
}

std::shared_ptr<Type> TypeChecker::inferUnary(const UnaryExpr& unary) {
    if (!unary.operand()) {
        return nullptr;
    }

    auto operandType = inferType(*unary.operand());
    if (!operandType) {
        return nullptr;
    }

    const std::string& op = unary.op();

    // 1. Unary plus and minus (+, -)
    if (op == "+" || op == "-") {
        if (!operandType->isArithmetic()) {
            reportError("invalid argument type '" + operandType->toString() +
                        "' to unary operator '" + op + "'", unary.location());
            return nullptr;
        }
        if (operandType->isDouble()) return Type::getDouble();
        if (operandType->isFloat()) return Type::getFloat();
        // Char undergoes integer promotion to int
        return Type::getInt();
    }

    // 2. Bitwise NOT (~) requires integer operand
    if (op == "~") {
        if (!operandType->isInteger()) {
            reportError("invalid argument type '" + operandType->toString() +
                        "' to unary operator '~'", unary.location());
            return nullptr;
        }
        return Type::getInt();
    }

    // 3. Logical NOT (!) yields int
    if (op == "!") {
        if (!operandType->isArithmetic()) {
            reportError("invalid argument type '" + operandType->toString() +
                        "' to unary operator '!'", unary.location());
            return nullptr;
        }
        return Type::getInt();
    }

    // 4. Prefix increment and decrement (++, --)
    if (op == "++" || op == "--") {
        if (!operandType->isArithmetic()) {
            reportError("invalid argument type '" + operandType->toString() +
                        "' to unary operator '" + op + "'", unary.location());
            return nullptr;
        }
        if (operandType->isConst()) {
            reportError("cannot modify constant value of type '" + operandType->toString() + "'",
                        unary.location());
            return nullptr;
        }
        if (!dynamic_cast<const IdentifierExpression*>(unary.operand())) {
            reportError("expression is not assignable", unary.location());
            return nullptr;
        }
        return Type::getPrimitive(operandType->kind());
    }

    reportError("unsupported unary operator '" + op + "'", unary.location());
    return nullptr;
}

std::shared_ptr<Type> TypeChecker::inferAssign(const AssignExpr& assign) {
    if (!assign.left() || !assign.right()) {
        return nullptr;
    }

    // Left operand must be an lvalue
    const auto* idLeft = dynamic_cast<const IdentifierExpression*>(assign.left());
    if (!idLeft) {
        reportError("expression is not assignable", assign.location());
        return nullptr;
    }

    auto leftType = inferType(*assign.left());
    auto rightType = inferType(*assign.right());
    if (!leftType || !rightType) {
        return nullptr;
    }

    // Cannot assign to const-qualified variable
    if (leftType->isConst()) {
        reportError("cannot assign to variable with const-qualified type '" +
                    leftType->toString() + "'", assign.location(), idLeft->name());
        return nullptr;
    }

    const std::string& op = assign.op();

    // Direct assignment: '='
    if (op == "=") {
        if (!isImplicitlyConvertible(*rightType, *leftType)) {
            reportError("incompatible types in assignment (cannot convert '" +
                        rightType->toString() + "' to '" + leftType->toString() + "')",
                        assign.location(), idLeft->name());
            return nullptr;
        }
        return Type::getPrimitive(leftType->kind());
    }

    // Compound assignments: '+=', '-=', '*=', '/='
    if (op == "+=" || op == "-=" || op == "*=" || op == "/=") {
        if (!leftType->isArithmetic() || !rightType->isArithmetic()) {
            reportError("invalid operands to compound assignment '" + op + "' ('" +
                        leftType->toString() + "' and '" + rightType->toString() + "')",
                        assign.location(), idLeft->name());
            return nullptr;
        }
        auto resultType = commonType(*leftType, *rightType);
        if (!resultType || !isImplicitlyConvertible(*resultType, *leftType)) {
            reportError("incompatible types in compound assignment '" + op + "' ('" +
                        rightType->toString() + "' to '" + leftType->toString() + "')",
                        assign.location(), idLeft->name());
            return nullptr;
        }
        return Type::getPrimitive(leftType->kind());
    }

    reportError("unsupported assignment operator '" + op + "'", assign.location());
    return nullptr;
}

std::shared_ptr<Type> TypeChecker::inferConditional(const ConditionalExpr& cond) {
    if (!cond.condition() || !cond.thenExpr() || !cond.elseExpr()) {
        return nullptr;
    }

    auto condType = inferType(*cond.condition());
    auto thenType = inferType(*cond.thenExpr());
    auto elseType = inferType(*cond.elseExpr());
    if (!condType || !thenType || !elseType) {
        return nullptr;
    }

    // Condition must be scalar/arithmetic
    if (!condType->isArithmetic()) {
        reportError("condition in conditional expression must have scalar type (found '" +
                    condType->toString() + "')", cond.location());
        return nullptr;
    }

    // If both branches are void, result is void
    if (thenType->isVoid() && elseType->isVoid()) {
        return Type::getVoid();
    }

    // If both branches are arithmetic, usual arithmetic conversions apply
    if (thenType->isArithmetic() && elseType->isArithmetic()) {
        auto common = commonType(*thenType, *elseType);
        if (!common) {
            reportError("incompatible operand types in conditional expression ('" +
                        thenType->toString() + "' and '" + elseType->toString() + "')",
                        cond.location());
            return nullptr;
        }
        return common;
    }

    // Incompatible branches
    reportError("incompatible operand types in conditional expression ('" +
                thenType->toString() + "' and '" + elseType->toString() + "')",
                cond.location());
    return nullptr;
}

std::shared_ptr<Type> TypeChecker::inferCall(const CallExpr& call) {
    const std::string& name = call.calleeName();
    if (name.empty()) {
        return nullptr;
    }
    const Symbol* sym = m_table.lookup(name);
    if (!sym || !sym->isFunction()) {
        return nullptr;
    }
    return sym->returnTypeShared();
}

bool TypeChecker::checkCall(const CallExpr& call) {
    bool ok = true;
    for (const auto& arg : call.arguments()) {
        if (arg) {
            if (!inferType(*arg)) {
                ok = false;
            }
        }
    }
    return ok;
}

// -----------------------------------------------------------------------------
// Validation Entry Points
// -----------------------------------------------------------------------------

bool TypeChecker::check(const Program& program) {
    return checkProgram(program);
}

bool TypeChecker::check(const ASTNode& node) {
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
        return check(*expr);
    }
    return true;
}

bool TypeChecker::check(const Statement& stmt) {
    return checkStatement(stmt);
}

bool TypeChecker::check(const Expression& expr) {
    inferType(expr);
    return !hasErrors();
}

bool TypeChecker::check(const CompoundStmt& compound) {
    return checkCompoundStmt(compound);
}

bool TypeChecker::check(const Function& fn) {
    return checkFunction(fn);
}

bool TypeChecker::check(const VarDecl& varDecl) {
    return checkVarDecl(varDecl);
}

bool TypeChecker::check(const VariableDeclaration& varDecl) {
    return checkVariableDeclaration(varDecl);
}

bool TypeChecker::check(const IfStmt& ifStmt) {
    return checkIfStmt(ifStmt);
}

bool TypeChecker::check(const WhileStmt& whileStmt) {
    return checkWhileStmt(whileStmt);
}

bool TypeChecker::check(const DoWhileStmt& doWhileStmt) {
    return checkDoWhileStmt(doWhileStmt);
}

bool TypeChecker::check(const ForStmt& forStmt) {
    return checkForStmt(forStmt);
}

// -----------------------------------------------------------------------------
// Internal Statement Traversal
// -----------------------------------------------------------------------------

bool TypeChecker::checkProgram(const Program& program) {
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

bool TypeChecker::checkStatement(const Statement& stmt) {
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

bool TypeChecker::checkCompoundStmt(const CompoundStmt& compound) {
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

bool TypeChecker::checkFunction(const Function& fn) {
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

    if (fn.isDefinition() || fn.parameterCount() > 0) {
        ScopeGuard guard(m_table);

        for (std::size_t i = 0; i < fn.parameterCount(); ++i) {
            if (const auto* param = fn.parameter(i)) {
                if (param->hasName()) {
                    m_table.insert(*param);
                    if (param->hasInitializer() && param->initializer()) {
                        if (!check(*param->initializer())) {
                            ok = false;
                        }
                    }
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

    return ok;
}

bool TypeChecker::checkVarDecl(const VarDecl& varDecl) {
    m_table.insert(varDecl);

    if (varDecl.hasInitializer() && varDecl.initializer()) {
        auto initType = inferType(*varDecl.initializer());
        if (!initType) {
            return false;
        }

        if (!isImplicitlyConvertible(*initType, *varDecl.type())) {
            reportError("incompatible types in initialization (cannot convert '" +
                        initType->toString() + "' to '" + varDecl.type()->toString() + "')",
                        varDecl.location(), varDecl.name());
            return false;
        }
    }
    return true;
}

bool TypeChecker::checkVariableDeclaration(const VariableDeclaration& varDecl) {
    if (varDecl.hasName()) {
        m_table.insert(varDecl);
    }

    if (varDecl.hasInitializer() && varDecl.initializer()) {
        auto initType = inferType(*varDecl.initializer());
        if (!initType) {
            return false;
        }

        if (varDecl.declaredType() && !isImplicitlyConvertible(*initType, *varDecl.declaredType())) {
            reportError("incompatible types in initialization (cannot convert '" +
                        initType->toString() + "' to '" + varDecl.declaredType()->toString() + "')",
                        varDecl.location(), varDecl.name());
            return false;
        }
    }
    return true;
}

bool TypeChecker::checkIfStmt(const IfStmt& ifStmt) {
    bool ok = true;
    if (ifStmt.condition()) {
        auto condType = inferType(*ifStmt.condition());
        if (condType && !condType->isArithmetic()) {
            reportError("condition in if statement must have scalar type (found '" +
                        condType->toString() + "')", ifStmt.location());
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

bool TypeChecker::checkWhileStmt(const WhileStmt& whileStmt) {
    bool ok = true;
    if (whileStmt.condition()) {
        auto condType = inferType(*whileStmt.condition());
        if (condType && !condType->isArithmetic()) {
            reportError("condition in while statement must have scalar type (found '" +
                        condType->toString() + "')", whileStmt.location());
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

bool TypeChecker::checkDoWhileStmt(const DoWhileStmt& doWhileStmt) {
    bool ok = true;
    if (doWhileStmt.body()) {
        if (!checkStatement(*doWhileStmt.body())) {
            ok = false;
        }
    }
    if (doWhileStmt.condition()) {
        auto condType = inferType(*doWhileStmt.condition());
        if (condType && !condType->isArithmetic()) {
            reportError("condition in do-while statement must have scalar type (found '" +
                        condType->toString() + "')", doWhileStmt.location());
            ok = false;
        }
    }
    return ok;
}

bool TypeChecker::checkForStmt(const ForStmt& forStmt) {
    bool ok = true;
    if (forStmt.initializer()) {
        if (!check(*forStmt.initializer())) {
            ok = false;
        }
    }
    if (forStmt.condition()) {
        auto condType = inferType(*forStmt.condition());
        if (condType && !condType->isArithmetic()) {
            reportError("condition in for statement must have scalar type (found '" +
                        condType->toString() + "')", forStmt.location());
            ok = false;
        }
    }
    if (forStmt.post()) {
        if (!check(*forStmt.post())) {
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

std::shared_ptr<Type> inferExpressionType(const Expression& expr) {
    TypeChecker checker;
    return checker.inferType(expr);
}

std::shared_ptr<Type> inferExpressionType(const Expression& expr, SymbolTable& table) {
    TypeChecker checker(table);
    return checker.inferType(expr);
}

std::vector<Diagnostic> checkTypes(const Expression& expr) {
    TypeChecker checker;
    checker.check(expr);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkTypes(const Expression& expr, SymbolTable& table) {
    TypeChecker checker(table);
    checker.check(expr);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkTypes(const Statement& stmt) {
    TypeChecker checker;
    checker.check(stmt);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkTypes(const Statement& stmt, SymbolTable& table) {
    TypeChecker checker(table);
    checker.check(stmt);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkTypes(const Program& program) {
    TypeChecker checker;
    checker.check(program);
    return checker.diagnostics();
}

std::vector<Diagnostic> checkTypes(const Program& program, SymbolTable& table) {
    TypeChecker checker(table);
    checker.check(program);
    return checker.diagnostics();
}
