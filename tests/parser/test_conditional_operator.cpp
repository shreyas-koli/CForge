#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/printer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

#include <memory>
#include <string>

// Helper to construct a Parser for a source string
static Parser makeParser(const std::string& src) {
    Lexer lexer(src);
    return Parser(lexer.tokenize());
}

// Helper to print AST node to string via ASTPrinter
static std::string dumpAST(const ASTNode* node) {
    ASTPrinter printer;
    return printer.print(node);
}

// =============================================================================
// DAY 26 TESTS - Ternary / Conditional Operator (?:)
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Basic Conditional Expressions
// -----------------------------------------------------------------------------
TEST_CASE("Conditional: basic expressions", "[parser][conditional]") {
    SECTION("identifiers 'a ? b : c'") {
        Parser parser = makeParser("a ? b : c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* cond = dynamic_cast<ConditionalExpr*>(expr.get());
        REQUIRE(cond != nullptr);
        REQUIRE(cond->condition() != nullptr);
        REQUIRE(cond->thenExpr() != nullptr);
        REQUIRE(cond->elseExpr() != nullptr);

        CHECK(cond->condition()->toString() == "IdentifierExpression(a)");
        CHECK(cond->thenExpr()->toString() == "IdentifierExpression(b)");
        CHECK(cond->elseExpr()->toString() == "IdentifierExpression(c)");

        // Test alias accessors
        CHECK(cond->trueExpr()->toString() == "IdentifierExpression(b)");
        CHECK(cond->falseExpr()->toString() == "IdentifierExpression(c)");
        CHECK(cond->getCondition()->toString() == "IdentifierExpression(a)");
        CHECK(cond->getThenExpr()->toString() == "IdentifierExpression(b)");
        CHECK(cond->getElseExpr()->toString() == "IdentifierExpression(c)");
        CHECK(cond->getTrueExpr()->toString() == "IdentifierExpression(b)");
        CHECK(cond->getFalseExpr()->toString() == "IdentifierExpression(c)");

        CHECK(cond->toString() ==
              "ConditionalExpr(IdentifierExpression(a), IdentifierExpression(b), IdentifierExpression(c))");
    }

    SECTION("integer literals '1 ? 2 : 3'") {
        Parser parser = makeParser("1 ? 2 : 3");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* cond = dynamic_cast<ConditionalExpr*>(expr.get());
        REQUIRE(cond != nullptr);
        CHECK(cond->condition()->toString() == "LiteralExpression(Integer, 1)");
        CHECK(cond->thenExpr()->toString() == "LiteralExpression(Integer, 2)");
        CHECK(cond->elseExpr()->toString() == "LiteralExpression(Integer, 3)");
    }

    SECTION("float literals 'c ? 3.14 : 2.71'") {
        Parser parser = makeParser("c ? 3.14 : 2.71");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* cond = dynamic_cast<ConditionalExpr*>(expr.get());
        REQUIRE(cond != nullptr);
        CHECK(cond->condition()->toString() == "IdentifierExpression(c)");
        CHECK(cond->thenExpr()->toString() == "LiteralExpression(Float, 3.14)");
        CHECK(cond->elseExpr()->toString() == "LiteralExpression(Float, 2.71)");
    }

    SECTION("char and string literals 'c ? \"yes\" : \"no\"'") {
        Parser parser = makeParser("c ? \"yes\" : \"no\"");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* cond = dynamic_cast<ConditionalExpr*>(expr.get());
        REQUIRE(cond != nullptr);
        CHECK(cond->condition()->toString() == "IdentifierExpression(c)");
        CHECK(cond->thenExpr()->toString() == "LiteralExpression(String, \"yes\")");
        CHECK(cond->elseExpr()->toString() == "LiteralExpression(String, \"no\")");
    }
}

// -----------------------------------------------------------------------------
// 2. Integration with Assignment Expressions
// -----------------------------------------------------------------------------
TEST_CASE("Conditional: assignment expression integration", "[parser][conditional]") {
    SECTION("simple assignment 'x = condition ? a : b'") {
        Parser parser = makeParser("x = condition ? a : b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "=");
        CHECK(assign->left()->toString() == "IdentifierExpression(x)");

        auto* cond = dynamic_cast<const ConditionalExpr*>(assign->right());
        REQUIRE(cond != nullptr);
        CHECK(cond->condition()->toString() == "IdentifierExpression(condition)");
        CHECK(cond->thenExpr()->toString() == "IdentifierExpression(a)");
        CHECK(cond->elseExpr()->toString() == "IdentifierExpression(b)");
    }

    SECTION("relational condition in assignment 'result = a > b ? a : b'") {
        Parser parser = makeParser("result = a > b ? a : b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->left()->toString() == "IdentifierExpression(result)");

        auto* cond = dynamic_cast<const ConditionalExpr*>(assign->right());
        REQUIRE(cond != nullptr);

        auto* bin = dynamic_cast<const BinaryExpr*>(cond->condition());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == ">");
        CHECK(bin->left()->toString() == "IdentifierExpression(a)");
        CHECK(bin->right()->toString() == "IdentifierExpression(b)");

        CHECK(cond->thenExpr()->toString() == "IdentifierExpression(a)");
        CHECK(cond->elseExpr()->toString() == "IdentifierExpression(b)");
    }

    SECTION("compound assignment 'x += c ? 10 : 20'") {
        Parser parser = makeParser("x += c ? 10 : 20");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "+=");

        auto* cond = dynamic_cast<const ConditionalExpr*>(assign->right());
        REQUIRE(cond != nullptr);
        CHECK(cond->condition()->toString() == "IdentifierExpression(c)");
        CHECK(cond->thenExpr()->toString() == "LiteralExpression(Integer, 10)");
        CHECK(cond->elseExpr()->toString() == "LiteralExpression(Integer, 20)");
    }

    SECTION("assignment inside then branch 'x = a ? y = 1 : 2'") {
        Parser parser = makeParser("x = a ? y = 1 : 2");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->left()->toString() == "IdentifierExpression(x)");

        auto* cond = dynamic_cast<const ConditionalExpr*>(assign->right());
        REQUIRE(cond != nullptr);
        CHECK(cond->condition()->toString() == "IdentifierExpression(a)");

        auto* thenAssign = dynamic_cast<const AssignExpr*>(cond->thenExpr());
        REQUIRE(thenAssign != nullptr);
        CHECK(thenAssign->left()->toString() == "IdentifierExpression(y)");
        CHECK(thenAssign->right()->toString() == "LiteralExpression(Integer, 1)");

        CHECK(cond->elseExpr()->toString() == "LiteralExpression(Integer, 2)");
    }
}

// -----------------------------------------------------------------------------
// 3. Precedence Interactions with Binary Operators
// -----------------------------------------------------------------------------
TEST_CASE("Conditional: precedence over binary operators", "[parser][conditional]") {
    SECTION("additive condition 'a + b ? c : d'") {
        // (a + b) ? c : d
        Parser parser = makeParser("a + b ? c : d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* cond = dynamic_cast<ConditionalExpr*>(expr.get());
        REQUIRE(cond != nullptr);

        auto* bin = dynamic_cast<const BinaryExpr*>(cond->condition());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");
        CHECK(bin->left()->toString() == "IdentifierExpression(a)");
        CHECK(bin->right()->toString() == "IdentifierExpression(b)");

        CHECK(cond->thenExpr()->toString() == "IdentifierExpression(c)");
        CHECK(cond->elseExpr()->toString() == "IdentifierExpression(d)");
    }

    SECTION("relational and arithmetic in all branches 'a < b ? x + 1 : y * 2'") {
        // (a < b) ? (x + 1) : (y * 2)
        Parser parser = makeParser("a < b ? x + 1 : y * 2");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* cond = dynamic_cast<ConditionalExpr*>(expr.get());
        REQUIRE(cond != nullptr);

        auto* condBin = dynamic_cast<const BinaryExpr*>(cond->condition());
        REQUIRE(condBin != nullptr);
        CHECK(condBin->op() == "<");

        auto* thenBin = dynamic_cast<const BinaryExpr*>(cond->thenExpr());
        REQUIRE(thenBin != nullptr);
        CHECK(thenBin->op() == "+");

        auto* elseBin = dynamic_cast<const BinaryExpr*>(cond->elseExpr());
        REQUIRE(elseBin != nullptr);
        CHECK(elseBin->op() == "*");
    }

    SECTION("equality operators 'a == b ? 1 : 0'") {
        Parser parser = makeParser("a == b ? 1 : 0");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* cond = dynamic_cast<ConditionalExpr*>(expr.get());
        REQUIRE(cond != nullptr);

        auto* bin = dynamic_cast<const BinaryExpr*>(cond->condition());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "==");
    }

    SECTION("bitwise and shift operators 'a << 1 ? b & c : d | e'") {
        Parser parser = makeParser("a << 1 ? b & c : d | e");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* cond = dynamic_cast<ConditionalExpr*>(expr.get());
        REQUIRE(cond != nullptr);

        auto* condBin = dynamic_cast<const BinaryExpr*>(cond->condition());
        REQUIRE(condBin != nullptr);
        CHECK(condBin->op() == "<<");

        auto* thenBin = dynamic_cast<const BinaryExpr*>(cond->thenExpr());
        REQUIRE(thenBin != nullptr);
        CHECK(thenBin->op() == "&");

        auto* elseBin = dynamic_cast<const BinaryExpr*>(cond->elseExpr());
        REQUIRE(elseBin != nullptr);
        CHECK(elseBin->op() == "|");
    }
}

// -----------------------------------------------------------------------------
// 4. Associativity (Right-Associative)
// -----------------------------------------------------------------------------
TEST_CASE("Conditional: right associativity", "[parser][conditional]") {
    SECTION("chained else-if 'a ? b : c ? d : e'") {
        // Right-associative: a ? b : (c ? d : e)
        Parser parser = makeParser("a ? b : c ? d : e");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* cond = dynamic_cast<ConditionalExpr*>(expr.get());
        REQUIRE(cond != nullptr);
        CHECK(cond->condition()->toString() == "IdentifierExpression(a)");
        CHECK(cond->thenExpr()->toString() == "IdentifierExpression(b)");

        auto* nestedElse = dynamic_cast<const ConditionalExpr*>(cond->elseExpr());
        REQUIRE(nestedElse != nullptr);
        CHECK(nestedElse->condition()->toString() == "IdentifierExpression(c)");
        CHECK(nestedElse->thenExpr()->toString() == "IdentifierExpression(d)");
        CHECK(nestedElse->elseExpr()->toString() == "IdentifierExpression(e)");
    }

    SECTION("three-level chain 'a ? 1 : b ? 2 : c ? 3 : 4'") {
        // a ? 1 : (b ? 2 : (c ? 3 : 4))
        Parser parser = makeParser("a ? 1 : b ? 2 : c ? 3 : 4");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* l1 = dynamic_cast<ConditionalExpr*>(expr.get());
        REQUIRE(l1 != nullptr);
        CHECK(l1->condition()->toString() == "IdentifierExpression(a)");

        auto* l2 = dynamic_cast<const ConditionalExpr*>(l1->elseExpr());
        REQUIRE(l2 != nullptr);
        CHECK(l2->condition()->toString() == "IdentifierExpression(b)");

        auto* l3 = dynamic_cast<const ConditionalExpr*>(l2->elseExpr());
        REQUIRE(l3 != nullptr);
        CHECK(l3->condition()->toString() == "IdentifierExpression(c)");
        CHECK(l3->thenExpr()->toString() == "LiteralExpression(Integer, 3)");
        CHECK(l3->elseExpr()->toString() == "LiteralExpression(Integer, 4)");
    }

    SECTION("nested in then branch 'a ? b ? c : d : e'") {
        // a ? (b ? c : d) : e
        Parser parser = makeParser("a ? b ? c : d : e");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<ConditionalExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->condition()->toString() == "IdentifierExpression(a)");
        CHECK(outer->elseExpr()->toString() == "IdentifierExpression(e)");

        auto* innerThen = dynamic_cast<const ConditionalExpr*>(outer->thenExpr());
        REQUIRE(innerThen != nullptr);
        CHECK(innerThen->condition()->toString() == "IdentifierExpression(b)");
        CHECK(innerThen->thenExpr()->toString() == "IdentifierExpression(c)");
        CHECK(innerThen->elseExpr()->toString() == "IdentifierExpression(d)");
    }
}

// -----------------------------------------------------------------------------
// 5. Parentheses Overriding Precedence
// -----------------------------------------------------------------------------
TEST_CASE("Conditional: parenthesized expressions", "[parser][conditional]") {
    SECTION("parenthesized conditional as operand '(a ? b : c) + d'") {
        Parser parser = makeParser("(a ? b : c) + d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");

        auto* cond = dynamic_cast<const ConditionalExpr*>(bin->left());
        REQUIRE(cond != nullptr);
        CHECK(cond->condition()->toString() == "IdentifierExpression(a)");
        CHECK(cond->thenExpr()->toString() == "IdentifierExpression(b)");
        CHECK(cond->elseExpr()->toString() == "IdentifierExpression(c)");

        CHECK(bin->right()->toString() == "IdentifierExpression(d)");
    }

    SECTION("parenthesized condition '(a ? b : c) ? d : e'") {
        Parser parser = makeParser("(a ? b : c) ? d : e");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<ConditionalExpr*>(expr.get());
        REQUIRE(outer != nullptr);

        auto* innerCond = dynamic_cast<const ConditionalExpr*>(outer->condition());
        REQUIRE(innerCond != nullptr);
        CHECK(innerCond->condition()->toString() == "IdentifierExpression(a)");
        CHECK(innerCond->thenExpr()->toString() == "IdentifierExpression(b)");
        CHECK(innerCond->elseExpr()->toString() == "IdentifierExpression(c)");

        CHECK(outer->thenExpr()->toString() == "IdentifierExpression(d)");
        CHECK(outer->elseExpr()->toString() == "IdentifierExpression(e)");
    }
}

// -----------------------------------------------------------------------------
// 6. AST Printer Verification
// -----------------------------------------------------------------------------
TEST_CASE("Conditional: AST printer output", "[parser][conditional][printer]") {
    SECTION("printer format for 'a ? b : c'") {
        Parser parser = makeParser("a ? b : c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        std::string expected =
            "ConditionalExpr\n"
            "  Condition:\n"
            "    IdentifierExpression: a\n"
            "  Then:\n"
            "    IdentifierExpression: b\n"
            "  Else:\n"
            "    IdentifierExpression: c\n";
        CHECK(dumpAST(expr.get()) == expected);
    }
}

// -----------------------------------------------------------------------------
// 7. Error Handling and Negative Tests
// -----------------------------------------------------------------------------
TEST_CASE("Conditional: error handling and syntax errors", "[parser][conditional][error]") {
    SECTION("missing ':' after then expression 'a ? b'") {
        Parser parser = makeParser("a ? b");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ':'") != std::string::npos);
    }

    SECTION("missing true expression 'a ? : c'") {
        Parser parser = makeParser("a ? : c");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression after '?'") != std::string::npos);
    }

    SECTION("missing ':' with extra token 'a ? b c'") {
        Parser parser = makeParser("a ? b c");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ':'") != std::string::npos);
    }

    SECTION("trailing '?' at EOF 'a ?'") {
        Parser parser = makeParser("a ?");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression after '?'") != std::string::npos);
    }

    SECTION("trailing ':' at EOF 'a ? b :'") {
        Parser parser = makeParser("a ? b :");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression after ':'") != std::string::npos);
    }
}
