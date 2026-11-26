#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/binary_expr.h"
#include "ast/call_expr.h"
#include "ast/compound_stmt.h"
#include "ast/function.h"
#include "ast/identifier_expression.h"
#include "ast/index_expr.h"
#include "ast/literal_expression.h"
#include "ast/printer.h"
#include "ast/var_decl.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/checks/call_check.h"
#include "semantic/checks/undeclared.h"
#include "types/type.h"

static Parser makeParser(const std::string& src) {
    Lexer lexer(src);
    return Parser(lexer.tokenize());
}

// =============================================================================
// DAY 56 TESTS — Multidimensional Arrays
// =============================================================================

// -----------------------------------------------------------------------------
// Area A: Recursive ArrayType Construction, Formatting, Equality, Qualifiers
// -----------------------------------------------------------------------------
TEST_CASE("MultidimArrayType: 2D and 3D recursive type construction and formatting", "[types][array][multidim]") {
    SECTION("Two-dimensional int[3][4] representation and formatting") {
        auto inner = Type::getArray(Type::getInt(), 4);
        auto outer = Type::getArray(inner, 3);

        REQUIRE(outer != nullptr);
        CHECK(outer->kind() == TypeKind::Array);
        CHECK(outer->isArray());
        CHECK(outer->arraySize() == 3);
        CHECK(outer->sizeInBytes() == 48);
        CHECK(outer->size() == 48);
        CHECK(outer->toString() == "int[3][4]");
        CHECK(std::string(outer->name()) == "int[3][4]");

        REQUIRE(outer->elementType() != nullptr);
        CHECK(outer->elementType()->isArray());
        auto* innerPtr = dynamic_cast<const ArrayType*>(outer->elementType());
        REQUIRE(innerPtr != nullptr);
        CHECK(innerPtr->arraySize() == 4);
        CHECK(innerPtr->sizeInBytes() == 16);
        CHECK(innerPtr->toString() == "int[4]");
        CHECK(outer->elementTypeShared() == inner);
        CHECK(outer->getElementType() == inner);

        REQUIRE(innerPtr->elementType() != nullptr);
        CHECK(innerPtr->elementType()->isInt());
        CHECK(innerPtr->elementType()->toString() == "int");
    }

    SECTION("Three-dimensional int[2][3][4] representation and formatting") {
        auto d2 = Type::getArray(Type::getInt(), 4);
        auto d1 = Type::getArray(d2, 3);
        auto d0 = Type::getArray(d1, 2);

        REQUIRE(d0 != nullptr);
        CHECK(d0->arraySize() == 2);
        CHECK(d0->sizeInBytes() == 96);
        CHECK(d0->toString() == "int[2][3][4]");
        CHECK(std::string(d0->name()) == "int[2][3][4]");

        auto* mid = dynamic_cast<const ArrayType*>(d0->elementType());
        REQUIRE(mid != nullptr);
        CHECK(mid->arraySize() == 3);
        CHECK(mid->sizeInBytes() == 48);
        CHECK(mid->toString() == "int[3][4]");

        auto* innermostArr = dynamic_cast<const ArrayType*>(mid->elementType());
        REQUIRE(innermostArr != nullptr);
        CHECK(innermostArr->arraySize() == 4);
        CHECK(innermostArr->sizeInBytes() == 16);
        CHECK(innermostArr->toString() == "int[4]");

        REQUIRE(innermostArr->elementType() != nullptr);
        CHECK(innermostArr->elementType()->isInt());
    }

    SECTION("Different base types format with outermost-to-innermost dimensions") {
        auto floatMat = Type::getArray(Type::getArray(Type::getFloat(), 5), 4);
        CHECK(floatMat->toString() == "float[4][5]");
        CHECK(floatMat->sizeInBytes() == 80);

        auto charTable = Type::getArray(Type::getArray(Type::getChar(), 20), 10);
        CHECK(charTable->toString() == "char[10][20]");
        CHECK(charTable->sizeInBytes() == 200);

        auto doubleCube = Type::getArray(
            Type::getArray(Type::getArray(Type::getDouble(), 5), 3), 2);
        CHECK(doubleCube->toString() == "double[2][3][5]");
        CHECK(doubleCube->sizeInBytes() == 240);
    }

    SECTION("Single-dimensional ArrayType formatting remains unchanged") {
        auto single = Type::getArray(Type::getInt(), 10);
        CHECK(single->toString() == "int[10]");
        CHECK(single->sizeInBytes() == 40);
    }
}

TEST_CASE("MultidimArrayType: Structural equality and qualifiers", "[types][array][multidim]") {
    auto int3x4_a = Type::getArray(Type::getArray(Type::getInt(), 4), 3);
    auto int3x4_b = Type::getArray(Type::getArray(Type::getInt(), 4), 3);
    auto int4x3   = Type::getArray(Type::getArray(Type::getInt(), 3), 4);
    auto int12    = Type::getArray(Type::getInt(), 12);
    auto float3x4 = Type::getArray(Type::getArray(Type::getFloat(), 4), 3);
    auto cube_a   = Type::getArray(Type::getArray(Type::getArray(Type::getInt(), 4), 3), 2);
    auto cube_b   = Type::getArray(Type::getArray(Type::getArray(Type::getInt(), 4), 3), 2);
    auto cube_diff = Type::getArray(Type::getArray(Type::getArray(Type::getInt(), 3), 4), 2);

    SECTION("Structural equality and inequality across dimensions and types") {
        CHECK(*int3x4_a == *int3x4_b);
        CHECK(int3x4_a->equals(*int3x4_b));
        CHECK(*cube_a == *cube_b);

        CHECK(*int3x4_a != *int4x3);
        CHECK(*int3x4_a != *int12);
        CHECK(*int3x4_a != *float3x4);
        CHECK(*cube_a != *cube_diff);
        CHECK(*int3x4_a != *cube_a);
    }

    SECTION("Const and volatile qualification propagation") {
        auto constInt3x4 = Type::getArray(Type::getArray(Type::getInt(true), 4), 3);
        CHECK(constInt3x4->isConst());
        CHECK(constInt3x4->toString() == "const int[3][4]");
        CHECK(*constInt3x4 != *int3x4_a);

        auto* inner = dynamic_cast<const ArrayType*>(constInt3x4->elementType());
        REQUIRE(inner != nullptr);
        CHECK(inner->isConst());
        REQUIRE(inner->elementType() != nullptr);
        CHECK(inner->elementType()->isConst());

        auto transformedConst = int3x4_a->withConst(true);
        REQUIRE(transformedConst != nullptr);
        CHECK(transformedConst->isConst());
        CHECK(transformedConst->toString() == "const int[3][4]");
        CHECK(*transformedConst == *constInt3x4);

        auto transformedVol = int3x4_a->withVolatile(true);
        REQUIRE(transformedVol != nullptr);
        CHECK(transformedVol->isVolatile());
        CHECK(transformedVol->toString() == "volatile int[3][4]");
    }
}

// -----------------------------------------------------------------------------
// Area B: Multidimensional Declaration Parsing
// -----------------------------------------------------------------------------
TEST_CASE("MultidimArrayDecl: Valid 2D and 3D declarations", "[parser][decl][array][multidim]") {
    SECTION("int matrix[3][3];") {
        Parser parser = makeParser("int matrix[3][3];");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        CHECK(decl->name() == "matrix");
        CHECK(decl->toString() == "VarDecl: int[3][3] matrix");

        auto* outer = dynamic_cast<const ArrayType*>(decl->type());
        REQUIRE(outer != nullptr);
        CHECK(outer->arraySize() == 3);
        CHECK(outer->sizeInBytes() == 36);
        CHECK(outer->toString() == "int[3][3]");

        auto* inner = dynamic_cast<const ArrayType*>(outer->elementType());
        REQUIRE(inner != nullptr);
        CHECK(inner->arraySize() == 3);
        CHECK(inner->elementType()->isInt());
    }

    SECTION("int matrix[3][4];") {
        Parser parser = makeParser("int matrix[3][4];");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        CHECK(decl->name() == "matrix");
        CHECK(decl->toString() == "VarDecl: int[3][4] matrix");

        auto* outer = dynamic_cast<const ArrayType*>(decl->type());
        REQUIRE(outer != nullptr);
        CHECK(outer->arraySize() == 3);
        CHECK(outer->sizeInBytes() == 48);
        CHECK(outer->toString() == "int[3][4]");

        auto* inner = dynamic_cast<const ArrayType*>(outer->elementType());
        REQUIRE(inner != nullptr);
        CHECK(inner->arraySize() == 4);
        CHECK(inner->sizeInBytes() == 16);
        REQUIRE(inner->elementType() != nullptr);
        CHECK(inner->elementType()->isInt());
    }

    SECTION("int cube[2][3][4];") {
        Parser parser = makeParser("int cube[2][3][4];");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        CHECK(decl->name() == "cube");
        CHECK(decl->toString() == "VarDecl: int[2][3][4] cube");

        auto* d0 = dynamic_cast<const ArrayType*>(decl->type());
        REQUIRE(d0 != nullptr);
        CHECK(d0->arraySize() == 2);
        CHECK(d0->sizeInBytes() == 96);
        CHECK(d0->toString() == "int[2][3][4]");

        auto* d1 = dynamic_cast<const ArrayType*>(d0->elementType());
        REQUIRE(d1 != nullptr);
        CHECK(d1->arraySize() == 3);
        CHECK(d1->sizeInBytes() == 48);

        auto* d2 = dynamic_cast<const ArrayType*>(d1->elementType());
        REQUIRE(d2 != nullptr);
        CHECK(d2->arraySize() == 4);
        CHECK(d2->sizeInBytes() == 16);
        REQUIRE(d2->elementType() != nullptr);
        CHECK(d2->elementType()->isInt());
    }

    SECTION("float values[4][5];") {
        Parser parser = makeParser("float values[4][5];");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        CHECK(decl->name() == "values");

        auto* outer = dynamic_cast<const ArrayType*>(decl->type());
        REQUIRE(outer != nullptr);
        CHECK(outer->arraySize() == 4);
        CHECK(outer->sizeInBytes() == 80);
        CHECK(outer->toString() == "float[4][5]");

        auto* inner = dynamic_cast<const ArrayType*>(outer->elementType());
        REQUIRE(inner != nullptr);
        CHECK(inner->arraySize() == 5);
        REQUIRE(inner->elementType() != nullptr);
        CHECK(inner->elementType()->isFloat());
    }

    SECTION("char table[10][20];") {
        Parser parser = makeParser("char table[10][20];");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        CHECK(decl->name() == "table");

        auto* outer = dynamic_cast<const ArrayType*>(decl->type());
        REQUIRE(outer != nullptr);
        CHECK(outer->arraySize() == 10);
        CHECK(outer->sizeInBytes() == 200);
        CHECK(outer->toString() == "char[10][20]");

        auto* inner = dynamic_cast<const ArrayType*>(outer->elementType());
        REQUIRE(inner != nullptr);
        CHECK(inner->arraySize() == 20);
        REQUIRE(inner->elementType() != nullptr);
        CHECK(inner->elementType()->isChar());
    }

    SECTION("const int data[3][4];") {
        Parser parser = makeParser("const int data[3][4];");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        CHECK(decl->name() == "data");
        CHECK(decl->isConst());
        CHECK(decl->toString() == "VarDecl: const int[3][4] data");

        auto* outer = dynamic_cast<const ArrayType*>(decl->type());
        REQUIRE(outer != nullptr);
        CHECK(outer->isConst());
        CHECK(outer->arraySize() == 3);
        CHECK(outer->toString() == "const int[3][4]");

        auto* inner = dynamic_cast<const ArrayType*>(outer->elementType());
        REQUIRE(inner != nullptr);
        CHECK(inner->isConst());
        CHECK(inner->arraySize() == 4);
        REQUIRE(inner->elementType() != nullptr);
        CHECK(inner->elementType()->isInt());
        CHECK(inner->elementType()->isConst());
    }

    SECTION("1x1 minimal multidimensional array and storage class") {
        Parser p1 = makeParser("int b[1][1];");
        auto d1 = p1.parseVarDecl();
        REQUIRE(d1 != nullptr);
        CHECK_FALSE(p1.hasError());
        CHECK(d1->type()->toString() == "int[1][1]");
        CHECK(d1->type()->size() == 4);

        Parser p2 = makeParser("static int grid[2][5];");
        auto d2 = p2.parseVarDecl();
        REQUIRE(d2 != nullptr);
        CHECK_FALSE(p2.hasError());
        CHECK(d2->isStatic());
        CHECK(d2->toString() == "VarDecl: static int[2][5] grid");
    }

    SECTION("Multidimensional declarations inside compound block") {
        const std::string src =
            "{\n"
            "    int matrix[3][4];\n"
            "    int cube[2][3][4];\n"
            "}";
        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        REQUIRE(block->statementCount() == 2);

        // Verify 2D declaration: int matrix[3][4];
        auto* matDecl = dynamic_cast<const VarDecl*>(block->statements()[0].get());
        REQUIRE(matDecl != nullptr);
        CHECK(matDecl->name() == "matrix");
        auto* matOuter = dynamic_cast<const ArrayType*>(matDecl->type());
        REQUIRE(matOuter != nullptr);
        CHECK(matOuter->arraySize() == 3);
        CHECK(matOuter->sizeInBytes() == 48);
        CHECK(matOuter->toString() == "int[3][4]");
        auto* matInner = dynamic_cast<const ArrayType*>(matOuter->elementType());
        REQUIRE(matInner != nullptr);
        CHECK(matInner->arraySize() == 4);
        REQUIRE(matInner->elementType() != nullptr);
        CHECK(matInner->elementType()->isInt());

        // Verify 3D declaration: int cube[2][3][4];
        auto* cubeDecl = dynamic_cast<const VarDecl*>(block->statements()[1].get());
        REQUIRE(cubeDecl != nullptr);
        CHECK(cubeDecl->name() == "cube");
        auto* cubeD0 = dynamic_cast<const ArrayType*>(cubeDecl->type());
        REQUIRE(cubeD0 != nullptr);
        CHECK(cubeD0->arraySize() == 2);
        CHECK(cubeD0->sizeInBytes() == 96);
        CHECK(cubeD0->toString() == "int[2][3][4]");
        auto* cubeD1 = dynamic_cast<const ArrayType*>(cubeD0->elementType());
        REQUIRE(cubeD1 != nullptr);
        CHECK(cubeD1->arraySize() == 3);
        auto* cubeD2 = dynamic_cast<const ArrayType*>(cubeD1->elementType());
        REQUIRE(cubeD2 != nullptr);
        CHECK(cubeD2->arraySize() == 4);
        REQUIRE(cubeD2->elementType() != nullptr);
        CHECK(cubeD2->elementType()->isInt());
    }
}

TEST_CASE("MultidimArrayDecl: Independent dimension validation errors", "[parser][decl][array][multidim][errors]") {
    SECTION("Zero first dimension: 'int a[0][4];'") {
        Parser parser = makeParser("int a[0][4];");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("greater than zero") != std::string::npos);
    }

    SECTION("Zero second dimension: 'int a[3][0];'") {
        Parser parser = makeParser("int a[3][0];");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("greater than zero") != std::string::npos);
    }

    SECTION("Negative second dimension: 'int b[3][-1];'") {
        Parser parser = makeParser("int b[3][-1];");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("positive") != std::string::npos);
    }

    SECTION("Negative first dimension: 'int b[-1][3];'") {
        Parser parser = makeParser("int b[-1][3];");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("positive") != std::string::npos);
    }

    SECTION("Empty first dimension: 'int c[][4];'") {
        Parser parser = makeParser("int c[][4];");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected array size") != std::string::npos);
    }

    SECTION("Empty second dimension: 'int d[3][];'") {
        Parser parser = makeParser("int d[3][];");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected array size") != std::string::npos);
    }

    SECTION("VLA in second dimension: 'int e[3][size];'") {
        Parser parser = makeParser("int e[3][size];");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Variable-length arrays are not supported") != std::string::npos);
    }

    SECTION("VLA in first dimension: 'int f[size][3];'") {
        Parser parser = makeParser("int f[size][3];");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Variable-length arrays are not supported") != std::string::npos);
    }

    SECTION("Missing closing bracket on second dimension: 'int g[3][4;'") {
        Parser parser = makeParser("int g[3][4;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ']'") != std::string::npos);
    }

    SECTION("Multidimensional array initialization still rejected: 'int h[3][4] = 0;'") {
        Parser parser = makeParser("int h[3][4] = 0;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Array initialization is not supported") != std::string::npos);
    }
}

// -----------------------------------------------------------------------------
// Area C: Multidimensional Indexing AST Verification
// -----------------------------------------------------------------------------
TEST_CASE("MultidimArrayIndex: 2D and 3D subscript expressions", "[parser][expr][array][multidim]") {
    SECTION("matrix[0][0]") {
        Parser parser = makeParser("matrix[0][0]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* outer = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        auto* idx1 = dynamic_cast<const LiteralExpression*>(outer->index());
        REQUIRE(idx1 != nullptr);
        CHECK(idx1->value() == "0");

        auto* inner = dynamic_cast<const IndexExpr*>(outer->base());
        REQUIRE(inner != nullptr);
        auto* idx0 = dynamic_cast<const LiteralExpression*>(inner->index());
        REQUIRE(idx0 != nullptr);
        CHECK(idx0->value() == "0");

        auto* base = dynamic_cast<const IdentifierExpression*>(inner->base());
        REQUIRE(base != nullptr);
        CHECK(base->name() == "matrix");
    }

    SECTION("matrix[1][2]") {
        Parser parser = makeParser("matrix[1][2]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        auto* idx1 = dynamic_cast<const LiteralExpression*>(outer->index());
        REQUIRE(idx1 != nullptr);
        CHECK(idx1->value() == "2");

        auto* inner = dynamic_cast<const IndexExpr*>(outer->base());
        REQUIRE(inner != nullptr);
        auto* idx0 = dynamic_cast<const LiteralExpression*>(inner->index());
        REQUIRE(idx0 != nullptr);
        CHECK(idx0->value() == "1");

        auto* base = dynamic_cast<const IdentifierExpression*>(inner->base());
        REQUIRE(base != nullptr);
        CHECK(base->name() == "matrix");
    }

    SECTION("cube[1][2][3]") {
        Parser parser = makeParser("cube[1][2][3]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* idx3 = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(idx3 != nullptr);
        auto* lit3 = dynamic_cast<const LiteralExpression*>(idx3->index());
        REQUIRE(lit3 != nullptr);
        CHECK(lit3->value() == "3");

        auto* idx2 = dynamic_cast<const IndexExpr*>(idx3->base());
        REQUIRE(idx2 != nullptr);
        auto* lit2 = dynamic_cast<const LiteralExpression*>(idx2->index());
        REQUIRE(lit2 != nullptr);
        CHECK(lit2->value() == "2");

        auto* idx1 = dynamic_cast<const IndexExpr*>(idx2->base());
        REQUIRE(idx1 != nullptr);
        auto* lit1 = dynamic_cast<const LiteralExpression*>(idx1->index());
        REQUIRE(lit1 != nullptr);
        CHECK(lit1->value() == "1");

        auto* base = dynamic_cast<const IdentifierExpression*>(idx1->base());
        REQUIRE(base != nullptr);
        CHECK(base->name() == "cube");
    }

    SECTION("matrix[i][j + 1]") {
        Parser parser = makeParser("matrix[i][j + 1]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        auto* addBin = dynamic_cast<const BinaryExpr*>(outer->index());
        REQUIRE(addBin != nullptr);
        CHECK(addBin->op() == "+");

        auto* jId = dynamic_cast<const IdentifierExpression*>(addBin->left());
        REQUIRE(jId != nullptr);
        CHECK(jId->name() == "j");

        auto* oneLit = dynamic_cast<const LiteralExpression*>(addBin->right());
        REQUIRE(oneLit != nullptr);
        CHECK(oneLit->value() == "1");

        auto* inner = dynamic_cast<const IndexExpr*>(outer->base());
        REQUIRE(inner != nullptr);
        auto* iId = dynamic_cast<const IdentifierExpression*>(inner->index());
        REQUIRE(iId != nullptr);
        CHECK(iId->name() == "i");

        auto* matId = dynamic_cast<const IdentifierExpression*>(inner->base());
        REQUIRE(matId != nullptr);
        CHECK(matId->name() == "matrix");
    }

    SECTION("matrix[i + 1][j * 2]") {
        Parser parser = makeParser("matrix[i + 1][j * 2]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        auto* mulBin = dynamic_cast<const BinaryExpr*>(outer->index());
        REQUIRE(mulBin != nullptr);
        CHECK(mulBin->op() == "*");

        auto* inner = dynamic_cast<const IndexExpr*>(outer->base());
        REQUIRE(inner != nullptr);
        auto* addBin = dynamic_cast<const BinaryExpr*>(inner->index());
        REQUIRE(addBin != nullptr);
        CHECK(addBin->op() == "+");
    }
}

// -----------------------------------------------------------------------------
// Area D: Day 55 Review Coverage Gaps (Call Index, ASTPrinter, SourceLocation, Semantics)
// -----------------------------------------------------------------------------
TEST_CASE("ArrayIndexing: Day 55 gap coverage", "[parser][expr][array][semantic][multidim]") {
    SECTION("Function call inside index brackets: numbers[function()]") {
        Parser parser = makeParser("numbers[function()]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* idx = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(idx != nullptr);

        auto* baseId = dynamic_cast<const IdentifierExpression*>(idx->base());
        REQUIRE(baseId != nullptr);
        CHECK(baseId->name() == "numbers");

        auto* call = dynamic_cast<const CallExpr*>(idx->index());
        REQUIRE(call != nullptr);
        CHECK(call->calleeName() == "function");
        CHECK(call->argumentCount() == 0);
    }

    SECTION("Nested ASTPrinter output for numbers[i][j]") {
        Parser parser = makeParser("numbers[i][j]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        ASTPrinter printer;
        std::string tree = printer.print(expr.get());
        std::string expected =
            "IndexExpr\n"
            "  Base:\n"
            "    IndexExpr\n"
            "      Base:\n"
            "        IdentifierExpression: numbers\n"
            "      Index:\n"
            "        IdentifierExpression: i\n"
            "  Index:\n"
            "    IdentifierExpression: j\n";

        CHECK(tree == expected);
    }

    SECTION("IndexExpr preserves source location on single and chained indexing") {
        Parser parser = makeParser("  numbers[i][j]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->location().line == 1);
        CHECK(outer->location().column == 3);

        auto* inner = dynamic_cast<const IndexExpr*>(outer->base());
        REQUIRE(inner != nullptr);
        CHECK(inner->location().line == 1);
        CHECK(inner->location().column == 3);
    }

    SECTION("UndeclaredChecker validates identifiers in base and all subscript dimensions") {
        {
            Parser parser = makeParser(
                "{\n"
                "    int matrix[3][4];\n"
                "    int i = 0;\n"
                "    int j = 1;\n"
                "    int val = matrix[i][j];\n"
                "}");
            auto block = parser.parseCompoundStatement();
            REQUIRE(block != nullptr);
            CHECK_FALSE(parser.hasError());
            auto diags = checkUndeclared(*block);
            CHECK(diags.empty());
        }

        {
            Parser parser = makeParser(
                "{\n"
                "    int i = 0;\n"
                "    int j = 1;\n"
                "    int val = unknown_mat[i][j];\n"
                "}");
            auto block = parser.parseCompoundStatement();
            REQUIRE(block != nullptr);
            CHECK_FALSE(parser.hasError());
            auto diags = checkUndeclared(*block);
            REQUIRE(diags.size() == 1);
            CHECK(diags[0].message.find("unknown_mat") != std::string::npos);
        }

        {
            Parser parser = makeParser(
                "{\n"
                "    int matrix[3][4];\n"
                "    int i = 0;\n"
                "    int val = matrix[i][missing_j];\n"
                "}");
            auto block = parser.parseCompoundStatement();
            REQUIRE(block != nullptr);
            CHECK_FALSE(parser.hasError());
            auto diags = checkUndeclared(*block);
            REQUIRE(diags.size() == 1);
            CHECK(diags[0].message.find("missing_j") != std::string::npos);
        }
    }

    SECTION("CallChecker validates calls inside multidimensional index expressions") {
        SymbolTable table;
        table.insert(Symbol::createFunction(
            "getIdx",
            Type::getInt(),
            {},
            false,
            ParameterListKind::Void));
        table.insert(Symbol::createFunction(
            "needArg",
            Type::getInt(),
            {Type::getInt()},
            false,
            ParameterListKind::Typed));
        table.insert(Symbol::createVariable(
            "matrix",
            Type::getArray(Type::getArray(Type::getInt(), 4), 3)));

        {
            Parser parser = makeParser("matrix[getIdx()][0]");
            auto expr = parser.parseExpression();
            REQUIRE(expr != nullptr);
            CHECK_FALSE(parser.hasError());
            auto diags = checkCalls(*expr, table);
            CHECK(diags.empty());
        }

        {
            Parser parser = makeParser("matrix[0][needArg()]");
            auto expr = parser.parseExpression();
            REQUIRE(expr != nullptr);
            CHECK_FALSE(parser.hasError());
            auto diags = checkCalls(*expr, table);
            REQUIRE_FALSE(diags.empty());
        }

        {
            Parser parser = makeParser("matrix[0][missingFn()]");
            auto expr = parser.parseExpression();
            REQUIRE(expr != nullptr);
            CHECK_FALSE(parser.hasError());
            auto diags = checkCalls(*expr, table);
            REQUIRE_FALSE(diags.empty());
        }
    }
}
