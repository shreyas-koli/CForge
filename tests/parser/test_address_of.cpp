#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/binary_expr.h"
#include "ast/identifier_expression.h"
#include "ast/literal_expression.h"
#include "ast/printer.h"
#include "ast/unary_expr.h"
#include "ast/var_decl.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/checks/undeclared.h"
#include "semantic/symbol_table.h"
#include "semantic/type_checker.h"
#include "types/type.h"

static Parser makeParser(const std::string& src) {
    Lexer lexer(src);
    return Parser(lexer.tokenize());
}

// =============================================================================
// DAY 58 TESTS — Address-Of Operator (&)
// =============================================================================

TEST_CASE("AddressOf: Parsing unary '&' expressions", "[parser][expr][unary][address_of]") {
    SECTION("Address-of variable: &x") {
        Parser parser = makeParser("&x");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* un = dynamic_cast<const UnaryExpr*>(expr.get());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "&");
        CHECK(un->toString() == "UnaryExpr(&, IdentifierExpression(x))");

        auto* id = dynamic_cast<const IdentifierExpression*>(un->operand());
        REQUIRE(id != nullptr);
        CHECK(id->name() == "x");
    }

    SECTION("Address-of pointer variable: &p") {
        Parser parser = makeParser("&p");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* un = dynamic_cast<const UnaryExpr*>(expr.get());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "&");

        auto* id = dynamic_cast<const IdentifierExpression*>(un->operand());
        REQUIRE(id != nullptr);
        CHECK(id->name() == "p");
    }

    SECTION("Preserves source location and ASTPrinter output") {
        Parser parser = makeParser("  &x");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* un = dynamic_cast<const UnaryExpr*>(expr.get());
        REQUIRE(un != nullptr);
        CHECK(un->location().line == 1);
        CHECK(un->location().column == 3);

        ASTPrinter printer;
        std::string expected =
            "UnaryExpr: &\n"
            "  Operand:\n"
            "    IdentifierExpression: x\n";
        CHECK(printer.print(un) == expected);
    }

    SECTION("Interaction with existing unary and parenthesized expressions") {
        Parser p1 = makeParser("(&x)");
        auto e1 = p1.parseExpression();
        REQUIRE(e1 != nullptr);
        CHECK_FALSE(p1.hasError());
        auto* un1 = dynamic_cast<const UnaryExpr*>(e1.get());
        REQUIRE(un1 != nullptr);
        CHECK(un1->op() == "&");

        Parser p2 = makeParser("!&x");
        auto e2 = p2.parseExpression();
        REQUIRE(e2 != nullptr);
        CHECK_FALSE(p2.hasError());
        auto* outer = dynamic_cast<const UnaryExpr*>(e2.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == "!");
        auto* inner = dynamic_cast<const UnaryExpr*>(outer->operand());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == "&");
        auto* id = dynamic_cast<const IdentifierExpression*>(inner->operand());
        REQUIRE(id != nullptr);
        CHECK(id->name() == "x");
    }

    SECTION("Regression: binary bitwise AND vs unary address-of") {
        Parser pBin = makeParser("a & b");
        auto eBin = pBin.parseExpression();
        REQUIRE(eBin != nullptr);
        CHECK_FALSE(pBin.hasError());
        auto* bin = dynamic_cast<const BinaryExpr*>(eBin.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "&");
        CHECK(bin->left()->toString() == "IdentifierExpression(a)");
        CHECK(bin->right()->toString() == "IdentifierExpression(b)");

        // Precedence: unary & binds tighter than binary & (a & &b -> BinaryExpr(&, a, UnaryExpr(&, b)))
        Parser pCombo = makeParser("a & &b");
        auto eCombo = pCombo.parseExpression();
        REQUIRE(eCombo != nullptr);
        CHECK_FALSE(pCombo.hasError());
        auto* comboBin = dynamic_cast<const BinaryExpr*>(eCombo.get());
        REQUIRE(comboBin != nullptr);
        CHECK(comboBin->op() == "&");
        auto* rhsUn = dynamic_cast<const UnaryExpr*>(comboBin->right());
        REQUIRE(rhsUn != nullptr);
        CHECK(rhsUn->op() == "&");
        auto* rhsId = dynamic_cast<const IdentifierExpression*>(rhsUn->operand());
        REQUIRE(rhsId != nullptr);
        CHECK(rhsId->name() == "b");
    }

    SECTION("Missing operand after '&' produces parser error") {
        Parser parser = makeParser("&");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression after '&'") != std::string::npos);
    }
}

TEST_CASE("AddressOf: Semantic type checking and operand restrictions", "[semantic][type_check][address_of]") {
    SymbolTable table;
    table.insert(Symbol::createVariable("x", Type::getInt()));
    table.insert(Symbol::createVariable("c", Type::getChar()));
    table.insert(Symbol::createVariable("f", Type::getFloat()));
    table.insert(Symbol::createVariable("p", Type::getPointer(Type::getInt())));
    table.insert(Symbol::createVariable("cx", Type::getInt(true)));

    SECTION("Address-of integer variable yields int*") {
        Parser parser = makeParser("&x");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        TypeChecker checker(table);
        auto res = checker.inferType(*expr);
        REQUIRE(res != nullptr);
        CHECK_FALSE(checker.hasErrors());
        CHECK(res->isPointer());
        CHECK(res->toString() == "int*");

        auto* ptr = dynamic_cast<const PointerType*>(res.get());
        REQUIRE(ptr != nullptr);
        REQUIRE(ptr->pointeeType() != nullptr);
        CHECK(ptr->pointeeType()->isInt());
    }

    SECTION("Address-of pointer variable yields int**") {
        Parser parser = makeParser("&p");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        TypeChecker checker(table);
        auto res = checker.inferType(*expr);
        REQUIRE(res != nullptr);
        CHECK_FALSE(checker.hasErrors());
        CHECK(res->isPointer());
        CHECK(res->toString() == "int**");

        auto* outer = dynamic_cast<const PointerType*>(res.get());
        REQUIRE(outer != nullptr);
        auto* inner = dynamic_cast<const PointerType*>(outer->pointeeType());
        REQUIRE(inner != nullptr);
        REQUIRE(inner->pointeeType() != nullptr);
        CHECK(inner->pointeeType()->isInt());
    }

    SECTION("Address-of char and float variables yield char* and float*") {
        Parser pChar = makeParser("&c");
        auto eChar = pChar.parseExpression();
        REQUIRE(eChar != nullptr);

        TypeChecker checker(table);
        auto tCharPtr = checker.inferType(*eChar);
        REQUIRE(tCharPtr != nullptr);
        CHECK_FALSE(checker.hasErrors());
        CHECK(tCharPtr->isPointer());
        CHECK(tCharPtr->toString() == "char*");

        Parser pFloat = makeParser("&f");
        auto eFloat = pFloat.parseExpression();
        REQUIRE(eFloat != nullptr);
        auto tFloatPtr = checker.inferType(*eFloat);
        REQUIRE(tFloatPtr != nullptr);
        CHECK_FALSE(checker.hasErrors());
        CHECK(tFloatPtr->isPointer());
        CHECK(tFloatPtr->toString() == "float*");
    }

    SECTION("Address-of const variable preserves const qualification on pointee") {
        Parser parser = makeParser("&cx");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        TypeChecker checker(table);
        auto res = checker.inferType(*expr);
        REQUIRE(res != nullptr);
        CHECK_FALSE(checker.hasErrors());
        CHECK(res->isPointer());
        CHECK_FALSE(res->isConst());
        CHECK(res->toString() == "const int*");

        auto* ptr = dynamic_cast<const PointerType*>(res.get());
        REQUIRE(ptr != nullptr);
        REQUIRE(ptr->pointeeType() != nullptr);
        CHECK(ptr->pointeeType()->isConst());
        CHECK(ptr->pointeeType()->isInt());
    }

    SECTION("Invalid non-lvalue operands are rejected") {
        for (const char* invalidExpr : {"&5", "&(x + 1)", "&-x", "&(&x)"}) {
            DYNAMIC_SECTION("Rejects '" << invalidExpr << "'") {
                Parser parser = makeParser(invalidExpr);
                auto expr = parser.parseExpression();
                REQUIRE(expr != nullptr);
                CHECK_FALSE(parser.hasError());

                TypeChecker checker(table);
                auto res = checker.inferType(*expr);
                CHECK(res == nullptr);
                CHECK(checker.hasErrors());
                REQUIRE(checker.errorCount() >= 1);
                CHECK(checker.diagnostics()[0].message.find("lvalue") != std::string::npos);
            }
        }
    }

    SECTION("UndeclaredChecker detects undeclared operand inside '&'") {
        Parser parser = makeParser("&missing_var");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        auto diags = checkUndeclared(*expr, table);
        REQUIRE(diags.size() == 1);
        CHECK(diags[0].identifier == "missing_var");
    }
}
