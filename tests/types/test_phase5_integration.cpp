#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/binary_expr.h"
#include "ast/break_stmt.h"
#include "ast/compound_stmt.h"
#include "ast/conditional_expr.h"
#include "ast/continue_stmt.h"
#include "ast/do_while_stmt.h"
#include "ast/for_stmt.h"
#include "ast/function.h"
#include "ast/identifier_expression.h"
#include "ast/if_stmt.h"
#include "ast/literal_expression.h"
#include "ast/printer.h"
#include "ast/program.h"
#include "ast/storage_class.h"
#include "ast/unary_expr.h"
#include "ast/var_decl.h"
#include "ast/variable_declaration.h"
#include "ast/while_stmt.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "types/conversions.h"
#include "types/type.h"

#include <memory>
#include <string>
#include <vector>

// Helper to construct a Parser from a source string
static Parser makeParser(const std::string& src) {
    Lexer lexer(src);
    return Parser(lexer.tokenize());
}

// Helper to format an ASTNode via ASTPrinter
static std::string dumpAST(const ASTNode* node) {
    ASTPrinter printer;
    return printer.print(node);
}

// =============================================================================
// DAY 44 — Phase 5 Comprehensive Integration Test Matrix
// Declarations & Types System Complete Verification (Days 36–44)
// =============================================================================

// =============================================================================
// Part A: Primitive Types Representation & Parsing Verification (Day 36)
// =============================================================================
TEST_CASE("Phase5Integration: Section A - Primitive Types comprehensive verification", "[phase5][types][primitive]") {
    SECTION("All 5 primitive type flyweights have correct properties and sizes") {
        struct TypeExpectation {
            std::shared_ptr<Type> type;
            TypeKind kind;
            const char* name;
            std::size_t size;
            bool isInt;
            bool isFloat;
            bool isArith;
            bool isVoid;
        };

        std::vector<TypeExpectation> expectations = {
            {Type::getVoid(),   TypeKind::Void,   "void",   0, false, false, false, true},
            {Type::getChar(),   TypeKind::Char,   "char",   1, true,  false, true,  false},
            {Type::getInt(),    TypeKind::Int,    "int",    4, true,  false, true,  false},
            {Type::getFloat(),  TypeKind::Float,  "float",  4, false, true,  true,  false},
            {Type::getDouble(), TypeKind::Double, "double", 8, false, true,  true,  false},
        };

        for (const auto& exp : expectations) {
            REQUIRE(exp.type != nullptr);
            CHECK(exp.type->kind() == exp.kind);
            CHECK(exp.type->getKind() == exp.kind);
            CHECK(std::string(exp.type->name()) == exp.name);
            CHECK(exp.type->toString() == exp.name);
            CHECK(exp.type->size() == exp.size);
            CHECK(exp.type->isInteger() == exp.isInt);
            CHECK(exp.type->isFloatingPoint() == exp.isFloat);
            CHECK(exp.type->isArithmetic() == exp.isArith);
            CHECK(exp.type->isVoid() == exp.isVoid);
            CHECK(exp.type->isPrimitive());
            CHECK_FALSE(exp.type->isConst());
            CHECK_FALSE(exp.type->isVolatile());
            CHECK_FALSE(exp.type->isQualified());

            // Flyweight singleton identity
            CHECK(exp.type == Type::getPrimitive(exp.kind));
            CHECK(exp.type == Type::fromKeyword(exp.name));
            CHECK(exp.type == getPrimitiveType(exp.name));
        }
    }

    SECTION("Declaration parsing for every primitive type") {
        std::vector<std::pair<std::string, TypeKind>> cases = {
            {"int var_i;", TypeKind::Int},
            {"char var_c;", TypeKind::Char},
            {"float var_f;", TypeKind::Float},
            {"double var_d;", TypeKind::Double},
            {"void var_v;", TypeKind::Void},
        };

        for (const auto& c : cases) {
            Parser parser = makeParser(c.first);
            auto decl = parser.parseVarDecl();
            REQUIRE(decl != nullptr);
            CHECK_FALSE(parser.hasError());
            CHECK(decl->typeKind() == c.second);
            CHECK(decl->type()->kind() == c.second);
            CHECK_FALSE(decl->isConst());
            CHECK_FALSE(decl->isVolatile());
            CHECK_FALSE(decl->hasInitializer());
        }
    }
}

// =============================================================================
// Part B: Const Qualifier Isolation and Syntactic Flexibility (Day 38)
// =============================================================================
TEST_CASE("Phase5Integration: Section B - Const Qualifier isolation and syntax", "[phase5][qualifier][const]") {
    SECTION("Const and non-const Type flyweights remain isolated") {
        auto tInt = Type::getInt();
        auto tConstInt = Type::getInt(true);

        REQUIRE(tInt != nullptr);
        REQUIRE(tConstInt != nullptr);
        CHECK(tInt != tConstInt);

        CHECK_FALSE(tInt->isConst());
        CHECK(tConstInt->isConst());
        CHECK(tConstInt->toString() == "const int");

        // Transformation round-trip
        CHECK(tInt->withConst(true) == tConstInt);
        CHECK(tConstInt->withConst(false) == tInt);
        CHECK(tInt->asConst() == tConstInt);
    }

    SECTION("Prefix const: 'const int x;' and 'const int x = 10;'") {
        Parser p1 = makeParser("const int x;");
        auto d1 = p1.parseVarDecl();
        REQUIRE(d1 != nullptr);
        CHECK(d1->isConst());
        CHECK(d1->type()->isConst());
        CHECK(d1->typeKind() == TypeKind::Int);
        CHECK_FALSE(d1->hasInitializer());

        Parser p2 = makeParser("const int x = 10;");
        auto d2 = p2.parseVarDecl();
        REQUIRE(d2 != nullptr);
        CHECK(d2->isConst());
        CHECK(d2->hasInitializer());
        auto* lit = dynamic_cast<const LiteralExpression*>(d2->initializer());
        REQUIRE(lit != nullptr);
        CHECK(lit->value() == "10");
    }

    SECTION("Reversed / trailing const: 'int const x;' and 'int const x = 20;'") {
        Parser p1 = makeParser("int const x;");
        auto d1 = p1.parseVarDecl();
        REQUIRE(d1 != nullptr);
        CHECK(d1->isConst());
        CHECK(d1->type()->isConst());
        CHECK(d1->typeKind() == TypeKind::Int);

        Parser p2 = makeParser("int const x = 20;");
        auto d2 = p2.parseVarDecl();
        REQUIRE(d2 != nullptr);
        CHECK(d2->isConst());
        CHECK(d2->hasInitializer());
    }

    SECTION("Ordinary non-const declarations remain unaffected") {
        Parser parser = makeParser("int plain = 99;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(decl->isConst());
        CHECK_FALSE(decl->type()->isConst());
        CHECK(decl->type() == Type::getInt().get());
    }
}

// =============================================================================
// Part C: Type Compatibility & Conversions (Day 39)
// =============================================================================
TEST_CASE("Phase5Integration: Section C - Type Compatibility and Conversions", "[phase5][conversions]") {
    SECTION("Identity conversions for all primitive types") {
        auto types = {Type::getChar(), Type::getInt(), Type::getFloat(), Type::getDouble(), Type::getVoid()};
        for (const auto& t : types) {
            CHECK(isIdentityConversion(*t, *t));
            CHECK(getConversionKind(*t, *t) == ConversionKind::Identity);
            CHECK_FALSE(isStrictlyImplicit(*t, *t));
            CHECK(isImplicitlyConvertible(*t, *t));
        }
    }

    SECTION("Implicit numeric conversions across all arithmetic pairs") {
        auto arith = {Type::getChar(), Type::getInt(), Type::getFloat(), Type::getDouble()};
        for (const auto& src : arith) {
            for (const auto& dst : arith) {
                if (src->kind() == dst->kind()) {
                    continue;
                }
                CHECK(getConversionKind(*src, *dst) == ConversionKind::Implicit);
                CHECK(isStrictlyImplicit(*src, *dst));
                CHECK(isImplicitlyConvertible(*src, *dst));
                CHECK_FALSE(isIdentityConversion(*src, *dst));
            }
        }
    }

    SECTION("Invalid conversions involving void") {
        auto nonVoid = {Type::getChar(), Type::getInt(), Type::getFloat(), Type::getDouble()};
        auto tVoid = Type::getVoid();

        for (const auto& nv : nonVoid) {
            CHECK(getConversionKind(*nv, *tVoid) == ConversionKind::Invalid);
            CHECK(getConversionKind(*tVoid, *nv) == ConversionKind::Invalid);
            CHECK_FALSE(isImplicitlyConvertible(*nv, *tVoid));
            CHECK_FALSE(isImplicitlyConvertible(*tVoid, *nv));
        }
    }

    SECTION("Const qualification compatibility behavior") {
        auto tInt = Type::getInt();
        auto tConstInt = Type::getInt(true);
        auto tFloat = Type::getFloat();

        // Qualified to non-qualified same primitive kind
        CHECK(getConversionKind(*tConstInt, *tInt) == ConversionKind::Identity);
        CHECK(getConversionKind(*tInt, *tConstInt) == ConversionKind::Identity);

        // Qualified arithmetic conversions
        CHECK(getConversionKind(*tConstInt, *tFloat) == ConversionKind::Implicit);
        CHECK(isStrictlyImplicit(*tConstInt, *tFloat));
        CHECK(isImplicitlyConvertible(*tConstInt, *tFloat));
    }
}

// =============================================================================
// Part D: Variable Declarations & Expression Initializers (Day 37)
// =============================================================================
TEST_CASE("Phase5Integration: Section D - Variable Declarations and initializers", "[phase5][decl][var]") {
    SECTION("Declarations with various expression initializer types") {
        // 1. Literal integer
        Parser p1 = makeParser("int a = 123;");
        auto d1 = p1.parseVarDecl();
        REQUIRE(d1 != nullptr);
        CHECK(dynamic_cast<const LiteralExpression*>(d1->initializer()) != nullptr);

        // 2. Identifier expression
        Parser p2 = makeParser("int b = a;");
        auto d2 = p2.parseVarDecl();
        REQUIRE(d2 != nullptr);
        auto* id = dynamic_cast<const IdentifierExpression*>(d2->initializer());
        REQUIRE(id != nullptr);
        CHECK(id->name() == "a");

        // 3. Binary expression: a + b * 2
        Parser p3 = makeParser("int c = a + b * 2;");
        auto d3 = p3.parseVarDecl();
        REQUIRE(d3 != nullptr);
        auto* bin = dynamic_cast<const BinaryExpr*>(d3->initializer());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");

        // 4. Unary expression: -x
        Parser p4 = makeParser("int d = -c;");
        auto d4 = p4.parseVarDecl();
        REQUIRE(d4 != nullptr);
        auto* un = dynamic_cast<const UnaryExpr*>(d4->initializer());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "-");

        // 5. Conditional expression: flag ? 1 : 0
        Parser p5 = makeParser("int e = flag ? 1 : 0;");
        auto d5 = p5.parseVarDecl();
        REQUIRE(d5 != nullptr);
        auto* cond = dynamic_cast<const ConditionalExpr*>(d5->initializer());
        REQUIRE(cond != nullptr);
    }
}

// =============================================================================
// Part E: Function Parameter Types (Day 40)
// =============================================================================
TEST_CASE("Phase5Integration: Section E - Function Parameter Types", "[phase5][decl][function][parameters]") {
    SECTION("Single and multiple typed parameters in prototype") {
        Parser parser = makeParser("int compute(int x, float y, double z, char tag);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(fn->isPrototype());
        CHECK(fn->isParameterListTyped());
        REQUIRE(fn->parameterCount() == 4);

        CHECK(fn->parameter(0)->name() == "x");
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Int);
        CHECK(fn->parameter(1)->name() == "y");
        CHECK(fn->parameter(1)->typeKind() == TypeKind::Float);
        CHECK(fn->parameter(2)->name() == "z");
        CHECK(fn->parameter(2)->typeKind() == TypeKind::Double);
        CHECK(fn->parameter(3)->name() == "tag");
        CHECK(fn->parameter(3)->typeKind() == TypeKind::Char);
    }

    SECTION("Unnamed parameters in prototype: 'int add(int, int);'") {
        Parser parser = makeParser("int add(int, int);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        REQUIRE(fn->parameterCount() == 2);
        CHECK(fn->parameter(0)->name().empty());
        CHECK(fn->parameter(0)->typeKind() == TypeKind::Int);
        CHECK(fn->parameter(1)->name().empty());
        CHECK(fn->parameter(1)->typeKind() == TypeKind::Int);
    }

    SECTION("C17 empty '()' vs explicit '(void)' distinction") {
        Parser p1 = makeParser("int legacy();");
        auto fn1 = p1.parseFunctionDeclaration();
        REQUIRE(fn1 != nullptr);
        CHECK(fn1->parameterListKind() == ParameterListKind::Unspecified);
        CHECK(fn1->isParameterListUnspecified());
        CHECK_FALSE(fn1->isParameterListVoid());

        Parser p2 = makeParser("int modern(void);");
        auto fn2 = p2.parseFunctionDeclaration();
        REQUIRE(fn2 != nullptr);
        CHECK(fn2->parameterListKind() == ParameterListKind::Void);
        CHECK(fn2->isParameterListVoid());
        CHECK_FALSE(fn2->isParameterListUnspecified());
    }

    SECTION("Function definitions require parameter names") {
        Parser pValid = makeParser("int validDef(int a, int b) {}");
        auto fnValid = pValid.parseFunctionDeclaration();
        REQUIRE(fnValid != nullptr);
        CHECK(fnValid->isDefinition());

        Parser pInvalid = makeParser("int invalidDef(int, int b) {}");
        auto fnInvalid = pInvalid.parseFunctionDeclaration();
        CHECK(fnInvalid == nullptr);
        CHECK(pInvalid.hasError());
        CHECK(pInvalid.errorMessage().find("Function definition requires parameter names") != std::string::npos);
    }
}

// =============================================================================
// Part F: Function Return Types (Day 41)
// =============================================================================
TEST_CASE("Phase5Integration: Section F - Function Return Types", "[phase5][decl][function][return]") {
    SECTION("All primitive return types in prototypes and definitions") {
        std::vector<std::pair<std::string, TypeKind>> signatures = {
            {"void doVoid(void);", TypeKind::Void},
            {"char doChar(void);", TypeKind::Char},
            {"int doInt(void);", TypeKind::Int},
            {"float doFloat(void);", TypeKind::Float},
            {"double doDouble(void);", TypeKind::Double},
        };

        for (const auto& sig : signatures) {
            Parser parser = makeParser(sig.first);
            auto fn = parser.parseFunctionDeclaration();
            REQUIRE(fn != nullptr);
            CHECK_FALSE(parser.hasError());
            CHECK(fn->returnTypeKind() == sig.second);
            REQUIRE(fn->returnTypePtr() != nullptr);
            CHECK(fn->returnTypePtr()->kind() == sig.second);
        }
    }

    SECTION("Qualified return types: const and volatile") {
        Parser p1 = makeParser("const int getConst(void);");
        auto fn1 = p1.parseFunctionDeclaration();
        REQUIRE(fn1 != nullptr);
        REQUIRE(fn1->returnTypePtr() != nullptr);
        CHECK(fn1->returnTypePtr()->isConst());
        CHECK(fn1->returnTypePtr()->kind() == TypeKind::Int);

        Parser p2 = makeParser("volatile int getVolatile(void);");
        auto fn2 = p2.parseFunctionDeclaration();
        REQUIRE(fn2 != nullptr);
        REQUIRE(fn2->returnTypePtr() != nullptr);
        CHECK(fn2->returnTypePtr()->isVolatile());
    }
}

// =============================================================================
// Part G: Storage Class & Qualifier Foundations (Day 42)
// =============================================================================
TEST_CASE("Phase5Integration: Section G - Storage Class and Qualifier Foundations", "[phase5][storage_class][qualifier]") {
    SECTION("Storage classes on variables: static, extern, register") {
        Parser pStatic = makeParser("static int s = 10;");
        auto dStatic = pStatic.parseVarDecl();
        REQUIRE(dStatic != nullptr);
        CHECK(dStatic->isStatic());
        CHECK(dStatic->storageClass() == StorageClass::Static);

        Parser pExtern = makeParser("extern int e;");
        auto dExtern = pExtern.parseVarDecl();
        REQUIRE(dExtern != nullptr);
        CHECK(dExtern->isExtern());
        CHECK(dExtern->storageClass() == StorageClass::Extern);

        Parser pReg = makeParser("register int r = 0;");
        auto dReg = pReg.parseVarDecl();
        REQUIRE(dReg != nullptr);
        CHECK(dReg->isRegister());
        CHECK(dReg->storageClass() == StorageClass::Register);
    }

    SECTION("Storage classes on functions: static, extern, rejected register") {
        Parser pStatic = makeParser("static int helper(void);");
        auto fnStatic = pStatic.parseFunctionDeclaration();
        REQUIRE(fnStatic != nullptr);
        CHECK(fnStatic->isStatic());

        Parser pExtern = makeParser("extern void remote(void);");
        auto fnExtern = pExtern.parseFunctionDeclaration();
        REQUIRE(fnExtern != nullptr);
        CHECK(fnExtern->isExtern());

        Parser pReg = makeParser("register int invalidFunc(void);");
        auto fnReg = pReg.parseFunctionDeclaration();
        CHECK(fnReg == nullptr);
        CHECK(pReg.hasError());
        CHECK(pReg.errorMessage().find("cannot have 'register'") != std::string::npos);
    }

    SECTION("Storage classes on function parameters: register allowed, static/extern rejected") {
        Parser pReg = makeParser("void run(register int count);");
        auto fnReg = pReg.parseFunctionDeclaration();
        REQUIRE(fnReg != nullptr);
        REQUIRE(fnReg->parameterCount() == 1);
        CHECK(fnReg->parameter(0)->isRegister());

        Parser pStatic = makeParser("void run(static int count);");
        auto fnStatic = pStatic.parseFunctionDeclaration();
        CHECK(fnStatic == nullptr);
        CHECK(pStatic.hasError());

        Parser pExtern = makeParser("void run(extern int count);");
        auto fnExtern = pExtern.parseFunctionDeclaration();
        CHECK(fnExtern == nullptr);
        CHECK(pExtern.hasError());
    }

    SECTION("Duplicate and conflicting storage classes are rejected") {
        Parser pDup = makeParser("static static int x;");
        CHECK(pDup.parseVarDecl() == nullptr);
        CHECK(pDup.hasError());

        Parser pConflict = makeParser("static extern int y;");
        CHECK(pConflict.parseVarDecl() == nullptr);
        CHECK(pConflict.hasError());
    }
}

// =============================================================================
// Part H: Declarations & Scopes in Compound Statements (Day 43)
// =============================================================================
TEST_CASE("Phase5Integration: Section H - Declarations & Scopes in Compound Statements", "[phase5][decl][scope]") {
    SECTION("Declarations in outer and nested compound blocks") {
        const std::string src =
            "{\n"
            "    int a = 1;\n"
            "    {\n"
            "        int b = 2;\n"
            "        {\n"
            "            int c = 3;\n"
            "        }\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto root = parser.parseCompoundStatement();
        REQUIRE(root != nullptr);
        CHECK_FALSE(parser.hasError());
        REQUIRE(root->statementCount() == 2);

        auto* l2 = dynamic_cast<const CompoundStmt*>(root->statements()[1].get());
        REQUIRE(l2 != nullptr);
        REQUIRE(l2->statementCount() == 2);

        auto* l3 = dynamic_cast<const CompoundStmt*>(l2->statements()[1].get());
        REQUIRE(l3 != nullptr);
        REQUIRE(l3->statementCount() == 1);
    }

    SECTION("Interleaved declarations and control flow statements") {
        const std::string src =
            "{\n"
            "    int x = 1;\n"
            "    if (x) {\n"
            "        break;\n"
            "    }\n"
            "    const char marker = 'M';\n"
            "    while (x) {\n"
            "        continue;\n"
            "    }\n"
            "    static double finalVal = 9.99;\n"
            "}";
        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());
        REQUIRE(block->statementCount() == 5);

        CHECK(dynamic_cast<const VarDecl*>(block->statements()[0].get()) != nullptr);
        CHECK(dynamic_cast<const IfStmt*>(block->statements()[1].get()) != nullptr);
        CHECK(dynamic_cast<const VarDecl*>(block->statements()[2].get()) != nullptr);
        CHECK(dynamic_cast<const WhileStmt*>(block->statements()[3].get()) != nullptr);
        CHECK(dynamic_cast<const VarDecl*>(block->statements()[4].get()) != nullptr);
    }

    SECTION("Exact source location preservation across nested blocks") {
        const std::string src =
            "{\n"
            "    int first = 1;\n"
            "    {\n"
            "        float second = 2.0;\n"
            "    }\n"
            "}";
        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        auto* d1 = dynamic_cast<const VarDecl*>(block->statements()[0].get());
        REQUIRE(d1 != nullptr);
        CHECK(d1->location().line == 2);
        CHECK(d1->location().column == 5);

        auto* inner = dynamic_cast<const CompoundStmt*>(block->statements()[1].get());
        REQUIRE(inner != nullptr);
        auto* d2 = dynamic_cast<const VarDecl*>(inner->statements()[0].get());
        REQUIRE(d2 != nullptr);
        CHECK(d2->location().line == 4);
        CHECK(d2->location().column == 9);
    }
}

// =============================================================================
// Part I: Full Phase 5 Cross-Cutting Integration Tests (Step 3)
// =============================================================================
TEST_CASE("Phase5Integration: Section I - Cross-Cutting Integration Scenarios", "[phase5][integration]") {
    SECTION("Scenario 1: Realistic C program combining prototype, definition, qualifiers, and nested scopes") {
        // Exact structure from roadmap specification:
        // int calculate(int a, float b);
        // int main(void) {
        //     const int limit = 10;
        //     char marker = 'A';
        //     {
        //         double result = 3.14;
        //         int value = limit;
        //     }
        //     return 0;
        // }
        const std::string programSrc =
            "int calculate(int a, float b);\n\n"
            "int main(void) {\n"
            "    const int limit = 10;\n"
            "    char marker = 'A';\n"
            "    {\n"
            "        double result = 3.14;\n"
            "        int value = limit;\n"
            "    }\n"
            "    return 0;\n"
            "}\n";

        Parser progParser = makeParser(programSrc);
        auto prog = progParser.parseProgram();
        REQUIRE(prog != nullptr);
        CHECK_FALSE(progParser.hasError());
        REQUIRE(prog->statementCount() == 2);

        // 1. Prototype calculate(int a, float b)
        auto* fnProto = dynamic_cast<const Function*>(prog->statements()[0].get());
        REQUIRE(fnProto != nullptr);
        CHECK(fnProto->name() == "calculate");
        CHECK(fnProto->returnTypeKind() == TypeKind::Int);
        CHECK(fnProto->isPrototype());
        REQUIRE(fnProto->parameterCount() == 2);
        CHECK(fnProto->parameter(0)->name() == "a");
        CHECK(fnProto->parameter(0)->typeKind() == TypeKind::Int);
        CHECK(fnProto->parameter(1)->name() == "b");
        CHECK(fnProto->parameter(1)->typeKind() == TypeKind::Float);

        // 2. Definition main(void)
        auto* fnDef = dynamic_cast<const Function*>(prog->statements()[1].get());
        REQUIRE(fnDef != nullptr);
        CHECK(fnDef->name() == "main");
        CHECK(fnDef->returnTypeKind() == TypeKind::Int);
        CHECK(fnDef->isDefinition());
        CHECK(fnDef->isParameterListVoid());

        // 3. Body block verification via direct block parsing
        const std::string bodyBlockSrc =
            "{\n"
            "    const int limit = 10;\n"
            "    char marker = 'A';\n"
            "    {\n"
            "        double result = 3.14;\n"
            "        int value = limit;\n"
            "    }\n"
            "}";
        Parser bodyParser = makeParser(bodyBlockSrc);
        auto bodyBlock = bodyParser.parseCompoundStatement();
        REQUIRE(bodyBlock != nullptr);
        CHECK_FALSE(bodyParser.hasError());
        REQUIRE(bodyBlock->statementCount() == 3);

        // Outer statement 0: const int limit = 10;
        auto* dLimit = dynamic_cast<const VarDecl*>(bodyBlock->statements()[0].get());
        REQUIRE(dLimit != nullptr);
        CHECK(dLimit->name() == "limit");
        CHECK(dLimit->isConst());
        CHECK(dLimit->typeKind() == TypeKind::Int);
        CHECK(dLimit->hasInitializer());

        // Outer statement 1: char marker = 'A';
        auto* dMarker = dynamic_cast<const VarDecl*>(bodyBlock->statements()[1].get());
        REQUIRE(dMarker != nullptr);
        CHECK(dMarker->name() == "marker");
        CHECK_FALSE(dMarker->isConst());
        CHECK(dMarker->typeKind() == TypeKind::Char);
        CHECK(dMarker->hasInitializer());

        // Outer statement 2: nested CompoundStmt
        auto* innerBlock = dynamic_cast<const CompoundStmt*>(bodyBlock->statements()[2].get());
        REQUIRE(innerBlock != nullptr);
        REQUIRE(innerBlock->statementCount() == 2);

        // Inner statement 0: double result = 3.14;
        auto* dResult = dynamic_cast<const VarDecl*>(innerBlock->statements()[0].get());
        REQUIRE(dResult != nullptr);
        CHECK(dResult->name() == "result");
        CHECK(dResult->typeKind() == TypeKind::Double);
        CHECK(dResult->hasInitializer());

        // Inner statement 1: int value = limit;
        auto* dValue = dynamic_cast<const VarDecl*>(innerBlock->statements()[1].get());
        REQUIRE(dValue != nullptr);
        CHECK(dValue->name() == "value");
        CHECK(dValue->typeKind() == TypeKind::Int);
        auto* valInit = dynamic_cast<const IdentifierExpression*>(dValue->initializer());
        REQUIRE(valInit != nullptr);
        CHECK(valInit->name() == "limit");

        // 4. Attach body to Function AST node and verify ASTPrinter
        auto fnWithBody = std::make_unique<Function>("main", Type::getInt(), SourceLocation{1, 1}, true);
        fnWithBody->setParameterListKind(ParameterListKind::Void);
        fnWithBody->addBodyStatement(std::move(bodyBlock));

        std::string tree = dumpAST(fnWithBody.get());
        CHECK(tree.find("Function (definition): int main") != std::string::npos);
        CHECK(tree.find("Body:\n") != std::string::npos);
        CHECK(tree.find("CompoundStmt\n") != std::string::npos);
        CHECK(tree.find("VarDecl: const int limit") != std::string::npos);
        CHECK(tree.find("VarDecl: char marker") != std::string::npos);
        CHECK(tree.find("VarDecl: double result") != std::string::npos);
        CHECK(tree.find("VarDecl: int value") != std::string::npos);
    }

    SECTION("Scenario 2: Program combining storage classes, qualifiers, and control flow in blocks") {
        Parser pVar1 = makeParser("static const int MAX_DEPTH = 16;");
        auto v1 = pVar1.parseVarDecl();
        REQUIRE(v1 != nullptr);
        CHECK_FALSE(pVar1.hasError());
        CHECK(v1->isStatic());
        CHECK(v1->isConst());
        CHECK(v1->typeKind() == TypeKind::Int);
        CHECK(v1->name() == "MAX_DEPTH");

        Parser pVar2 = makeParser("extern volatile int sys_status;");
        auto v2 = pVar2.parseVarDecl();
        REQUIRE(v2 != nullptr);
        CHECK_FALSE(pVar2.hasError());
        CHECK(v2->isExtern());
        CHECK(v2->isVolatile());
        CHECK(v2->typeKind() == TypeKind::Int);
        CHECK(v2->name() == "sys_status");

        const std::string unitSrc =
            "void process(register int code, const float threshold);\n"
            "int handle(void);\n";

        Parser parser = makeParser(unitSrc);
        auto prog = parser.parseProgram();
        REQUIRE(prog != nullptr);
        CHECK_FALSE(parser.hasError());
        REQUIRE(prog->statementCount() == 2);

        // Stmt 0: void process(register int code, const float threshold);
        auto* fn0 = dynamic_cast<const Function*>(prog->statements()[0].get());
        REQUIRE(fn0 != nullptr);
        CHECK(fn0->name() == "process");
        REQUIRE(fn0->parameterCount() == 2);
        CHECK(fn0->parameter(0)->isRegister());
        CHECK(fn0->parameter(0)->name() == "code");
        CHECK(fn0->parameter(1)->isConst());
        CHECK(fn0->parameter(1)->name() == "threshold");

        // Stmt 1: int handle(void);
        auto* fn1 = dynamic_cast<const Function*>(prog->statements()[1].get());
        REQUIRE(fn1 != nullptr);
        CHECK(fn1->name() == "handle");
        CHECK(fn1->parameterListKind() == ParameterListKind::Void);

        // Complex block combining all storage classes, qualifiers, and loops
        const std::string blockSrc =
            "{\n"
            "    static int counter = 0;\n"
            "    const double factor = 1.25;\n"
            "    if (counter) {\n"
            "        volatile char tag = 'X';\n"
            "        {\n"
            "            register int inner = 42;\n"
            "        }\n"
            "    } else {\n"
            "        register int fallback = -1;\n"
            "    }\n"
            "    while (counter) {\n"
            "        int step = 1;\n"
            "        break;\n"
            "    }\n"
            "}";

        Parser blockParser = makeParser(blockSrc);
        auto block = blockParser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(blockParser.hasError());
        REQUIRE(block->statementCount() == 4);

        // 0: static int counter = 0;
        auto* dCounter = dynamic_cast<const VarDecl*>(block->statements()[0].get());
        REQUIRE(dCounter != nullptr);
        CHECK(dCounter->isStatic());

        // 1: const double factor = 1.25;
        auto* dFactor = dynamic_cast<const VarDecl*>(block->statements()[1].get());
        REQUIRE(dFactor != nullptr);
        CHECK(dFactor->isConst());

        // 2: if statement with then/else blocks
        auto* ifStmt = dynamic_cast<const IfStmt*>(block->statements()[2].get());
        REQUIRE(ifStmt != nullptr);
        auto* thenBlock = dynamic_cast<const CompoundStmt*>(ifStmt->thenBranch());
        REQUIRE(thenBlock != nullptr);
        REQUIRE(thenBlock->statementCount() == 2);
        CHECK(dynamic_cast<const VarDecl*>(thenBlock->statements()[0].get())->isVolatile());
        auto* deepest = dynamic_cast<const CompoundStmt*>(thenBlock->statements()[1].get());
        REQUIRE(deepest != nullptr);
        CHECK(dynamic_cast<const VarDecl*>(deepest->statements()[0].get())->isRegister());

        auto* elseBlock = dynamic_cast<const CompoundStmt*>(ifStmt->elseBranch());
        REQUIRE(elseBlock != nullptr);
        CHECK(dynamic_cast<const VarDecl*>(elseBlock->statements()[0].get())->isRegister());

        // 3: while statement
        auto* whileStmt = dynamic_cast<const WhileStmt*>(block->statements()[3].get());
        REQUIRE(whileStmt != nullptr);
        auto* whileBody = dynamic_cast<const CompoundStmt*>(whileStmt->body());
        REQUIRE(whileBody != nullptr);
        REQUIRE(whileBody->statementCount() == 2);
        CHECK(dynamic_cast<const VarDecl*>(whileBody->statements()[0].get())->name() == "step");
        CHECK(dynamic_cast<const BreakStmt*>(whileBody->statements()[1].get()) != nullptr);
    }

    SECTION("Scenario 3: Lexical shadowing across 4 distinct block nesting tiers with different types") {
        const std::string scopeSrc =
            "{\n"
            "    int data = 1;\n"
            "    {\n"
            "        char data = 'A';\n"
            "        {\n"
            "            float data = 2.5;\n"
            "            {\n"
            "                double data = 9.99;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";

        Parser parser = makeParser(scopeSrc);
        auto l1 = parser.parseCompoundStatement();
        REQUIRE(l1 != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* d1 = dynamic_cast<const VarDecl*>(l1->statements()[0].get());
        REQUIRE(d1 != nullptr);
        CHECK(d1->typeKind() == TypeKind::Int);

        auto* l2 = dynamic_cast<const CompoundStmt*>(l1->statements()[1].get());
        REQUIRE(l2 != nullptr);
        auto* d2 = dynamic_cast<const VarDecl*>(l2->statements()[0].get());
        REQUIRE(d2 != nullptr);
        CHECK(d2->typeKind() == TypeKind::Char);

        auto* l3 = dynamic_cast<const CompoundStmt*>(l2->statements()[1].get());
        REQUIRE(l3 != nullptr);
        auto* d3 = dynamic_cast<const VarDecl*>(l3->statements()[0].get());
        REQUIRE(d3 != nullptr);
        CHECK(d3->typeKind() == TypeKind::Float);

        auto* l4 = dynamic_cast<const CompoundStmt*>(l3->statements()[1].get());
        REQUIRE(l4 != nullptr);
        auto* d4 = dynamic_cast<const VarDecl*>(l4->statements()[0].get());
        REQUIRE(d4 != nullptr);
        CHECK(d4->typeKind() == TypeKind::Double);

        // All 4 VarDecl pointers are distinct instances
        CHECK(d1 != d2);
        CHECK(d2 != d3);
        CHECK(d3 != d4);
    }

    SECTION("Scenario 4: Serialization fidelity with ASTPrinter across mixed Phase 5 constructs") {
        ASTPrinter printer;

        // Verify printed form of VarDecl with qualifiers and storage class
        Parser p1 = makeParser("static const volatile int reg = 0;");
        auto d1 = p1.parseVarDecl();
        REQUIRE(d1 != nullptr);
        std::string out1 = printer.print(d1.get());
        CHECK(out1.find("VarDecl: static const volatile int reg") != std::string::npos);

        // Verify printed form of Function prototype with qualifiers and storage class
        Parser p2 = makeParser("extern const int getFixedId(register int offset);");
        auto fn2 = p2.parseFunctionDeclaration();
        REQUIRE(fn2 != nullptr);
        std::string out2 = printer.print(fn2.get());
        CHECK(out2.find("Function (prototype): extern const int getFixedId") != std::string::npos);
        CHECK(out2.find("VariableDeclaration: register int offset") != std::string::npos);
    }
}
