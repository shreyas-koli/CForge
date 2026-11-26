#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/compound_stmt.h"
#include "ast/var_decl.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "types/type.h"

static Parser makeParser(const std::string& src) {
    Lexer lexer(src);
    return Parser(lexer.tokenize());
}

// =============================================================================
// DAY 57 TESTS — Pointer Declarations
// =============================================================================

TEST_CASE("PointerDecl: Single-level pointer declarations", "[parser][decl][pointer]") {
    SECTION("int *p;") {
        Parser parser = makeParser("int *p;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "p");
        CHECK_FALSE(decl->hasInitializer());
        REQUIRE(decl->type() != nullptr);
        CHECK(decl->type()->kind() == TypeKind::Pointer);
        CHECK(decl->type()->isPointer());
        CHECK(decl->type()->toString() == "int*");
        CHECK(decl->toString() == "VarDecl: int* p");

        auto* ptrType = dynamic_cast<const PointerType*>(decl->type());
        REQUIRE(ptrType != nullptr);
        REQUIRE(ptrType->pointeeType() != nullptr);
        CHECK(ptrType->pointeeType()->isInt());
    }

    SECTION("char *name;") {
        Parser parser = makeParser("char *name;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "name");
        auto* ptrType = dynamic_cast<const PointerType*>(decl->type());
        REQUIRE(ptrType != nullptr);
        REQUIRE(ptrType->pointeeType() != nullptr);
        CHECK(ptrType->pointeeType()->isChar());
        CHECK(ptrType->toString() == "char*");
        CHECK(decl->toString() == "VarDecl: char* name");
    }

    SECTION("float *value;") {
        Parser parser = makeParser("float *value;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "value");
        auto* ptrType = dynamic_cast<const PointerType*>(decl->type());
        REQUIRE(ptrType != nullptr);
        REQUIRE(ptrType->pointeeType() != nullptr);
        CHECK(ptrType->pointeeType()->isFloat());
        CHECK(ptrType->toString() == "float*");
    }

    SECTION("double *dp; and void *vp;") {
        Parser pDouble = makeParser("double *dp;");
        auto dDouble = pDouble.parseVarDecl();
        REQUIRE(dDouble != nullptr);
        CHECK_FALSE(pDouble.hasError());
        auto* dPtr = dynamic_cast<const PointerType*>(dDouble->type());
        REQUIRE(dPtr != nullptr);
        CHECK(dPtr->pointeeType()->isDouble());
        CHECK(dPtr->toString() == "double*");

        Parser pVoid = makeParser("void *vp;");
        auto dVoid = pVoid.parseVarDecl();
        REQUIRE(dVoid != nullptr);
        CHECK_FALSE(pVoid.hasError());
        auto* vPtr = dynamic_cast<const PointerType*>(dVoid->type());
        REQUIRE(vPtr != nullptr);
        CHECK(vPtr->pointeeType()->isVoid());
        CHECK(vPtr->toString() == "void*");
    }

    SECTION("Distinct pointee types produce distinct types") {
        Parser p1 = makeParser("int *a;");
        auto d1 = p1.parseVarDecl();
        REQUIRE(d1 != nullptr);

        Parser p2 = makeParser("char *b;");
        auto d2 = p2.parseVarDecl();
        REQUIRE(d2 != nullptr);

        CHECK(*d1->type() != *d2->type());
    }
}

TEST_CASE("PointerDecl: Multi-level nested pointer declarations", "[parser][decl][pointer]") {
    SECTION("int **ptr;") {
        Parser parser = makeParser("int **ptr;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "ptr");
        CHECK(decl->toString() == "VarDecl: int** ptr");

        auto* outer = dynamic_cast<const PointerType*>(decl->type());
        REQUIRE(outer != nullptr);
        CHECK(outer->toString() == "int**");

        auto* inner = dynamic_cast<const PointerType*>(outer->pointeeType());
        REQUIRE(inner != nullptr);
        CHECK(inner->toString() == "int*");
        REQUIRE(inner->pointeeType() != nullptr);
        CHECK(inner->pointeeType()->isInt());
    }

    SECTION("int ***triple;") {
        Parser parser = makeParser("int ***triple;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(decl->name() == "triple");
        CHECK(decl->toString() == "VarDecl: int*** triple");

        auto* l3 = dynamic_cast<const PointerType*>(decl->type());
        REQUIRE(l3 != nullptr);
        CHECK(l3->toString() == "int***");

        auto* l2 = dynamic_cast<const PointerType*>(l3->pointeeType());
        REQUIRE(l2 != nullptr);
        CHECK(l2->toString() == "int**");

        auto* l1 = dynamic_cast<const PointerType*>(l2->pointeeType());
        REQUIRE(l1 != nullptr);
        CHECK(l1->toString() == "int*");
        REQUIRE(l1->pointeeType() != nullptr);
        CHECK(l1->pointeeType()->isInt());
    }

    SECTION("Different indirection levels produce unequal types") {
        Parser p1 = makeParser("int *p1;");
        auto d1 = p1.parseVarDecl();
        Parser p2 = makeParser("int **p2;");
        auto d2 = p2.parseVarDecl();
        Parser p3 = makeParser("int ***p3;");
        auto d3 = p3.parseVarDecl();

        REQUIRE(d1 != nullptr);
        REQUIRE(d2 != nullptr);
        REQUIRE(d3 != nullptr);

        CHECK(*d1->type() != *d2->type());
        CHECK(*d2->type() != *d3->type());
        CHECK(*d1->type() != *d3->type());
    }
}

TEST_CASE("PointerDecl: Qualifiers, storage classes, and compound blocks", "[parser][decl][pointer]") {
    SECTION("const int *p; qualifies pointee, not pointer") {
        Parser parser = makeParser("const int *p;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* ptr = dynamic_cast<const PointerType*>(decl->type());
        REQUIRE(ptr != nullptr);
        CHECK_FALSE(ptr->isConst());
        REQUIRE(ptr->pointeeType() != nullptr);
        CHECK(ptr->pointeeType()->isConst());
        CHECK(ptr->pointeeType()->isInt());
        CHECK(ptr->toString() == "const int*");
    }

    SECTION("int * const p; qualifies pointer, not pointee") {
        Parser parser = makeParser("int * const p;");
        auto decl = parser.parseVarDecl();
        REQUIRE(decl != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* ptr = dynamic_cast<const PointerType*>(decl->type());
        REQUIRE(ptr != nullptr);
        CHECK(ptr->isConst());
        REQUIRE(ptr->pointeeType() != nullptr);
        CHECK_FALSE(ptr->pointeeType()->isConst());
        CHECK(ptr->pointeeType()->isInt());
        CHECK(ptr->toString() == "int* const");
    }

    SECTION("static int *sp; and extern char *ep;") {
        Parser p1 = makeParser("static int *sp;");
        auto d1 = p1.parseVarDecl();
        REQUIRE(d1 != nullptr);
        CHECK_FALSE(p1.hasError());
        CHECK(d1->isStatic());
        CHECK(d1->toString() == "VarDecl: static int* sp");

        Parser p2 = makeParser("extern char *ep;");
        auto d2 = p2.parseVarDecl();
        REQUIRE(d2 != nullptr);
        CHECK_FALSE(p2.hasError());
        CHECK(d2->isExtern());
        CHECK(d2->toString() == "VarDecl: extern char* ep");
    }

    SECTION("Pointer declarations inside compound statement") {
        const std::string src =
            "{\n"
            "    int *p;\n"
            "    char *name;\n"
            "    int **ptr;\n"
            "    int ***triple;\n"
            "}";
        Parser parser = makeParser(src);
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        REQUIRE(block->statementCount() == 4);

        auto* v0 = dynamic_cast<const VarDecl*>(block->statements()[0].get());
        auto* v1 = dynamic_cast<const VarDecl*>(block->statements()[1].get());
        auto* v2 = dynamic_cast<const VarDecl*>(block->statements()[2].get());
        auto* v3 = dynamic_cast<const VarDecl*>(block->statements()[3].get());
        REQUIRE(v0 != nullptr);
        REQUIRE(v1 != nullptr);
        REQUIRE(v2 != nullptr);
        REQUIRE(v3 != nullptr);

        CHECK(v0->type()->toString() == "int*");
        CHECK(v1->type()->toString() == "char*");
        CHECK(v2->type()->toString() == "int**");
        CHECK(v3->type()->toString() == "int***");
    }
}

TEST_CASE("PointerDecl: Invalid pointer declarations and regression checks", "[parser][decl][pointer][errors]") {
    SECTION("Missing identifier after '*': 'int *;'") {
        Parser parser = makeParser("int *;");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected identifier") != std::string::npos);
    }

    SECTION("Missing semicolon after pointer declaration: 'int *p'") {
        Parser parser = makeParser("int *p");
        auto decl = parser.parseVarDecl();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ';'") != std::string::npos);
    }

    SECTION("parseDeclaration does not consume pointer variable declaration") {
        Parser parser = makeParser("int *p;");
        auto fn = parser.parseDeclaration();
        CHECK(fn == nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.peek().lexeme == "int");

        auto var = parser.parseVarDecl();
        REQUIRE(var != nullptr);
        CHECK(var->name() == "p");
        CHECK(var->type()->isPointer());
        CHECK(parser.isAtEnd());
    }
}
