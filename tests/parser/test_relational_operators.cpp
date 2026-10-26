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
// DAY 22 TESTS - Relational Operators (<, >, <=, >=)
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Individual Relational Operators
// -----------------------------------------------------------------------------
TEST_CASE("Relational: less-than operator '<'", "[parser][relational]") {
    SECTION("identifiers 'a < b'") {
        Parser parser = makeParser("a < b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "<");
        CHECK(bin->left()->toString() == "IdentifierExpression(a)");
        CHECK(bin->right()->toString() == "IdentifierExpression(b)");
        CHECK(bin->toString() == "BinaryExpr(<, IdentifierExpression(a), IdentifierExpression(b))");
    }

    SECTION("integer literals '10 < 20'") {
        Parser parser = makeParser("10 < 20");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "<");
        CHECK(bin->left()->toString() == "LiteralExpression(Integer, 10)");
        CHECK(bin->right()->toString() == "LiteralExpression(Integer, 20)");
    }

    SECTION("float literals '1.5 < 2.5'") {
        Parser parser = makeParser("1.5 < 2.5");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "<");
        CHECK(bin->left()->toString() == "LiteralExpression(Float, 1.5)");
        CHECK(bin->right()->toString() == "LiteralExpression(Float, 2.5)");
    }
}

TEST_CASE("Relational: greater-than operator '>'", "[parser][relational]") {
    SECTION("identifiers 'x > y'") {
        Parser parser = makeParser("x > y");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == ">");
        CHECK(bin->left()->toString() == "IdentifierExpression(x)");
        CHECK(bin->right()->toString() == "IdentifierExpression(y)");
        CHECK(bin->toString() == "BinaryExpr(>, IdentifierExpression(x), IdentifierExpression(y))");
    }

    SECTION("integer literals '42 > 7'") {
        Parser parser = makeParser("42 > 7");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == ">");
        CHECK(bin->left()->toString() == "LiteralExpression(Integer, 42)");
        CHECK(bin->right()->toString() == "LiteralExpression(Integer, 7)");
    }
}

TEST_CASE("Relational: less-than-or-equal operator '<='", "[parser][relational]") {
    SECTION("identifiers 'count <= limit'") {
        Parser parser = makeParser("count <= limit");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "<=");
        CHECK(bin->left()->toString() == "IdentifierExpression(count)");
        CHECK(bin->right()->toString() == "IdentifierExpression(limit)");
        CHECK(bin->toString() == "BinaryExpr(<=, IdentifierExpression(count), IdentifierExpression(limit))");
    }

    SECTION("literal and identifier '0 <= i'") {
        Parser parser = makeParser("0 <= i");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "<=");
        CHECK(bin->left()->toString() == "LiteralExpression(Integer, 0)");
        CHECK(bin->right()->toString() == "IdentifierExpression(i)");
    }
}

TEST_CASE("Relational: greater-than-or-equal operator '>='", "[parser][relational]") {
    SECTION("identifiers 'score >= threshold'") {
        Parser parser = makeParser("score >= threshold");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == ">=");
        CHECK(bin->left()->toString() == "IdentifierExpression(score)");
        CHECK(bin->right()->toString() == "IdentifierExpression(threshold)");
        CHECK(bin->toString() == "BinaryExpr(>=, IdentifierExpression(score), IdentifierExpression(threshold))");
    }

    SECTION("identifier and literal 'val >= 100'") {
        Parser parser = makeParser("val >= 100");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == ">=");
        CHECK(bin->left()->toString() == "IdentifierExpression(val)");
        CHECK(bin->right()->toString() == "LiteralExpression(Integer, 100)");
    }
}

// -----------------------------------------------------------------------------
// 2. Precedence: Arithmetic Binds Tighter Than Relational
// -----------------------------------------------------------------------------
TEST_CASE("Relational Precedence: Arithmetic operators bind tighter than relational", "[parser][relational][precedence]") {
    SECTION("'a + b < c * d' -> (a + b) < (c * d)") {
        Parser parser = makeParser("a + b < c * d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* rel = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");

        auto* leftAdd = dynamic_cast<const BinaryExpr*>(rel->left());
        REQUIRE(leftAdd != nullptr);
        CHECK(leftAdd->op() == "+");
        CHECK(leftAdd->left()->toString() == "IdentifierExpression(a)");
        CHECK(leftAdd->right()->toString() == "IdentifierExpression(b)");

        auto* rightMul = dynamic_cast<const BinaryExpr*>(rel->right());
        REQUIRE(rightMul != nullptr);
        CHECK(rightMul->op() == "*");
        CHECK(rightMul->left()->toString() == "IdentifierExpression(c)");
        CHECK(rightMul->right()->toString() == "IdentifierExpression(d)");
    }

    SECTION("'a < b + c' -> a < (b + c)") {
        Parser parser = makeParser("a < b + c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* rel = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");
        CHECK(rel->left()->toString() == "IdentifierExpression(a)");

        auto* rightAdd = dynamic_cast<const BinaryExpr*>(rel->right());
        REQUIRE(rightAdd != nullptr);
        CHECK(rightAdd->op() == "+");
        CHECK(rightAdd->left()->toString() == "IdentifierExpression(b)");
        CHECK(rightAdd->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("'a + b >= c' -> (a + b) >= c") {
        Parser parser = makeParser("a + b >= c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* rel = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == ">=");

        auto* leftAdd = dynamic_cast<const BinaryExpr*>(rel->left());
        REQUIRE(leftAdd != nullptr);
        CHECK(leftAdd->op() == "+");
        CHECK(leftAdd->left()->toString() == "IdentifierExpression(a)");
        CHECK(leftAdd->right()->toString() == "IdentifierExpression(b)");

        CHECK(rel->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("'a * b > c + d' -> (a * b) > (c + d)") {
        Parser parser = makeParser("a * b > c + d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* rel = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == ">");

        auto* leftMul = dynamic_cast<const BinaryExpr*>(rel->left());
        REQUIRE(leftMul != nullptr);
        CHECK(leftMul->op() == "*");

        auto* rightAdd = dynamic_cast<const BinaryExpr*>(rel->right());
        REQUIRE(rightAdd != nullptr);
        CHECK(rightAdd->op() == "+");
    }

    SECTION("'a % b <= c / d' -> (a % b) <= (c / d)") {
        Parser parser = makeParser("a % b <= c / d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* rel = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<=");

        auto* leftMod = dynamic_cast<const BinaryExpr*>(rel->left());
        REQUIRE(leftMod != nullptr);
        CHECK(leftMod->op() == "%");

        auto* rightDiv = dynamic_cast<const BinaryExpr*>(rel->right());
        REQUIRE(rightDiv != nullptr);
        CHECK(rightDiv->op() == "/");
    }

    SECTION("'a - b > c % d' -> (a - b) > (c % d)") {
        Parser parser = makeParser("a - b > c % d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* rel = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == ">");

        auto* leftSub = dynamic_cast<const BinaryExpr*>(rel->left());
        REQUIRE(leftSub != nullptr);
        CHECK(leftSub->op() == "-");

        auto* rightMod = dynamic_cast<const BinaryExpr*>(rel->right());
        REQUIRE(rightMod != nullptr);
        CHECK(rightMod->op() == "%");
    }

    SECTION("'a * b * c < d + e + f' -> ((a * b) * c) < ((d + e) + f)") {
        Parser parser = makeParser("a * b * c < d + e + f");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* rel = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");

        // Left operand should be ((a * b) * c)
        auto* mul2 = dynamic_cast<const BinaryExpr*>(rel->left());
        REQUIRE(mul2 != nullptr);
        CHECK(mul2->op() == "*");
        CHECK(mul2->right()->toString() == "IdentifierExpression(c)");

        auto* mul1 = dynamic_cast<const BinaryExpr*>(mul2->left());
        REQUIRE(mul1 != nullptr);
        CHECK(mul1->op() == "*");
        CHECK(mul1->left()->toString() == "IdentifierExpression(a)");
        CHECK(mul1->right()->toString() == "IdentifierExpression(b)");

        // Right operand should be ((d + e) + f)
        auto* add2 = dynamic_cast<const BinaryExpr*>(rel->right());
        REQUIRE(add2 != nullptr);
        CHECK(add2->op() == "+");
        CHECK(add2->right()->toString() == "IdentifierExpression(f)");

        auto* add1 = dynamic_cast<const BinaryExpr*>(add2->left());
        REQUIRE(add1 != nullptr);
        CHECK(add1->op() == "+");
        CHECK(add1->left()->toString() == "IdentifierExpression(d)");
        CHECK(add1->right()->toString() == "IdentifierExpression(e)");
    }
}

// -----------------------------------------------------------------------------
// 3. Left-Associativity of Relational Operators
// -----------------------------------------------------------------------------
TEST_CASE("Relational Associativity: Relational operators are left-associative", "[parser][relational][associativity]") {
    SECTION("'a < b < c' -> (a < b) < c") {
        Parser parser = makeParser("a < b < c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == "<");
        CHECK(outer->right()->toString() == "IdentifierExpression(c)");

        auto* inner = dynamic_cast<const BinaryExpr*>(outer->left());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == "<");
        CHECK(inner->left()->toString() == "IdentifierExpression(a)");
        CHECK(inner->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'a <= b <= c' -> (a <= b) <= c") {
        Parser parser = makeParser("a <= b <= c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == "<=");
        CHECK(outer->right()->toString() == "IdentifierExpression(c)");

        auto* inner = dynamic_cast<const BinaryExpr*>(outer->left());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == "<=");
        CHECK(inner->left()->toString() == "IdentifierExpression(a)");
        CHECK(inner->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'a > b > c' -> (a > b) > c") {
        Parser parser = makeParser("a > b > c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == ">");
        CHECK(outer->right()->toString() == "IdentifierExpression(c)");

        auto* inner = dynamic_cast<const BinaryExpr*>(outer->left());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == ">");
        CHECK(inner->left()->toString() == "IdentifierExpression(a)");
        CHECK(inner->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'a >= b >= c' -> (a >= b) >= c") {
        Parser parser = makeParser("a >= b >= c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == ">=");
        CHECK(outer->right()->toString() == "IdentifierExpression(c)");

        auto* inner = dynamic_cast<const BinaryExpr*>(outer->left());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == ">=");
        CHECK(inner->left()->toString() == "IdentifierExpression(a)");
        CHECK(inner->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("mixed chain 'a < b <= c > d >= e' -> ((((a < b) <= c) > d) >= e)") {
        Parser parser = makeParser("a < b <= c > d >= e");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        // Level 4: outer is '>='
        auto* op4 = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(op4 != nullptr);
        CHECK(op4->op() == ">=");
        CHECK(op4->right()->toString() == "IdentifierExpression(e)");

        // Level 3: inner is '>'
        auto* op3 = dynamic_cast<const BinaryExpr*>(op4->left());
        REQUIRE(op3 != nullptr);
        CHECK(op3->op() == ">");
        CHECK(op3->right()->toString() == "IdentifierExpression(d)");

        // Level 2: inner is '<='
        auto* op2 = dynamic_cast<const BinaryExpr*>(op3->left());
        REQUIRE(op2 != nullptr);
        CHECK(op2->op() == "<=");
        CHECK(op2->right()->toString() == "IdentifierExpression(c)");

        // Level 1: inner is '<'
        auto* op1 = dynamic_cast<const BinaryExpr*>(op2->left());
        REQUIRE(op1 != nullptr);
        CHECK(op1->op() == "<");
        CHECK(op1->left()->toString() == "IdentifierExpression(a)");
        CHECK(op1->right()->toString() == "IdentifierExpression(b)");
    }
}

// -----------------------------------------------------------------------------
// 4. Parentheses Overriding Precedence
// -----------------------------------------------------------------------------
TEST_CASE("Relational Precedence: Parentheses override normal precedence", "[parser][relational][parens]") {
    SECTION("'(a < b) + c' -> (a < b) + c") {
        Parser parser = makeParser("(a < b) + c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* add = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(add != nullptr);
        CHECK(add->op() == "+");
        CHECK(add->right()->toString() == "IdentifierExpression(c)");

        auto* rel = dynamic_cast<const BinaryExpr*>(add->left());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");
        CHECK(rel->left()->toString() == "IdentifierExpression(a)");
        CHECK(rel->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'a * (b < c)' -> a * (b < c)") {
        Parser parser = makeParser("a * (b < c)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* mul = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(mul != nullptr);
        CHECK(mul->op() == "*");
        CHECK(mul->left()->toString() == "IdentifierExpression(a)");

        auto* rel = dynamic_cast<const BinaryExpr*>(mul->right());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");
        CHECK(rel->left()->toString() == "IdentifierExpression(b)");
        CHECK(rel->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("'a < (b < c)' forces right-associativity") {
        Parser parser = makeParser("a < (b < c)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == "<");
        CHECK(outer->left()->toString() == "IdentifierExpression(a)");

        auto* inner = dynamic_cast<const BinaryExpr*>(outer->right());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == "<");
        CHECK(inner->left()->toString() == "IdentifierExpression(b)");
        CHECK(inner->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("'(a + b) * (c < d)'") {
        Parser parser = makeParser("(a + b) * (c < d)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* mul = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(mul != nullptr);
        CHECK(mul->op() == "*");

        auto* leftAdd = dynamic_cast<const BinaryExpr*>(mul->left());
        REQUIRE(leftAdd != nullptr);
        CHECK(leftAdd->op() == "+");

        auto* rightRel = dynamic_cast<const BinaryExpr*>(mul->right());
        REQUIRE(rightRel != nullptr);
        CHECK(rightRel->op() == "<");
    }
}

// -----------------------------------------------------------------------------
// 5. Interaction with Prefix Unary Operators
// -----------------------------------------------------------------------------
TEST_CASE("Relational Precedence: Prefix unary operators bind tighter", "[parser][relational][unary]") {
    SECTION("'-a < b' -> (-a) < b") {
        Parser parser = makeParser("-a < b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* rel = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");
        CHECK(rel->right()->toString() == "IdentifierExpression(b)");

        auto* un = dynamic_cast<const UnaryExpr*>(rel->left());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "-");
        CHECK(un->operand()->toString() == "IdentifierExpression(a)");
    }

    SECTION("'a < -b' -> a < (-b)") {
        Parser parser = makeParser("a < -b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* rel = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");
        CHECK(rel->left()->toString() == "IdentifierExpression(a)");

        auto* un = dynamic_cast<const UnaryExpr*>(rel->right());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "-");
        CHECK(un->operand()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'-a < -b' -> (-a) < (-b)") {
        Parser parser = makeParser("-a < -b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* rel = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");

        auto* leftUn = dynamic_cast<const UnaryExpr*>(rel->left());
        REQUIRE(leftUn != nullptr);
        CHECK(leftUn->op() == "-");

        auto* rightUn = dynamic_cast<const UnaryExpr*>(rel->right());
        REQUIRE(rightUn != nullptr);
        CHECK(rightUn->op() == "-");
    }

    SECTION("'!a <= ~b' -> (!a) <= (~b)") {
        Parser parser = makeParser("!a <= ~b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* rel = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<=");

        auto* leftUn = dynamic_cast<const UnaryExpr*>(rel->left());
        REQUIRE(leftUn != nullptr);
        CHECK(leftUn->op() == "!");

        auto* rightUn = dynamic_cast<const UnaryExpr*>(rel->right());
        REQUIRE(rightUn != nullptr);
        CHECK(rightUn->op() == "~");
    }

    SECTION("'++a > --b' -> (++a) > (--b)") {
        Parser parser = makeParser("++a > --b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* rel = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == ">");

        auto* leftUn = dynamic_cast<const UnaryExpr*>(rel->left());
        REQUIRE(leftUn != nullptr);
        CHECK(leftUn->op() == "++");

        auto* rightUn = dynamic_cast<const UnaryExpr*>(rel->right());
        REQUIRE(rightUn != nullptr);
        CHECK(rightUn->op() == "--");
    }

    SECTION("'-(a < b)' -> -(a < b)") {
        Parser parser = makeParser("-(a < b)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* un = dynamic_cast<UnaryExpr*>(expr.get());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "-");

        auto* rel = dynamic_cast<const BinaryExpr*>(un->operand());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");
    }
}

// -----------------------------------------------------------------------------
// 6. Interaction with Assignment Operators
// -----------------------------------------------------------------------------
TEST_CASE("Relational Precedence: Assignment has lower precedence than relational", "[parser][relational][assignment]") {
    SECTION("'x = a < b' -> x = (a < b)") {
        Parser parser = makeParser("x = a < b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "=");
        CHECK(assign->left()->toString() == "IdentifierExpression(x)");

        auto* rel = dynamic_cast<const BinaryExpr*>(assign->right());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");
        CHECK(rel->left()->toString() == "IdentifierExpression(a)");
        CHECK(rel->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'x = a + b < c * d' -> x = ((a + b) < (c * d))") {
        Parser parser = makeParser("x = a + b < c * d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "=");
        CHECK(assign->left()->toString() == "IdentifierExpression(x)");

        auto* rel = dynamic_cast<const BinaryExpr*>(assign->right());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");

        auto* add = dynamic_cast<const BinaryExpr*>(rel->left());
        REQUIRE(add != nullptr);
        CHECK(add->op() == "+");

        auto* mul = dynamic_cast<const BinaryExpr*>(rel->right());
        REQUIRE(mul != nullptr);
        CHECK(mul->op() == "*");
    }

    SECTION("'x += a >= b' -> x += (a >= b)") {
        Parser parser = makeParser("x += a >= b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "+=");
        CHECK(assign->left()->toString() == "IdentifierExpression(x)");

        auto* rel = dynamic_cast<const BinaryExpr*>(assign->right());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == ">=");
    }

    SECTION("'x = y = a <= b' -> x = (y = (a <= b))") {
        Parser parser = makeParser("x = y = a <= b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outerAssign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(outerAssign != nullptr);
        CHECK(outerAssign->op() == "=");
        CHECK(outerAssign->left()->toString() == "IdentifierExpression(x)");

        auto* innerAssign = dynamic_cast<const AssignExpr*>(outerAssign->right());
        REQUIRE(innerAssign != nullptr);
        CHECK(innerAssign->op() == "=");
        CHECK(innerAssign->left()->toString() == "IdentifierExpression(y)");

        auto* rel = dynamic_cast<const BinaryExpr*>(innerAssign->right());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<=");
        CHECK(rel->left()->toString() == "IdentifierExpression(a)");
        CHECK(rel->right()->toString() == "IdentifierExpression(b)");
    }
}

// -----------------------------------------------------------------------------
// 7. AST Printer Representation
// -----------------------------------------------------------------------------
TEST_CASE("Relational ASTPrinter output", "[parser][relational][printer]") {
    SECTION("simple 'a < b'") {
        Parser parser = makeParser("a < b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        std::string expected =
            "BinaryExpr: <\n"
            "  Left:\n"
            "    IdentifierExpression: a\n"
            "  Right:\n"
            "    IdentifierExpression: b\n";
        CHECK(dumpAST(expr.get()) == expected);
    }

    SECTION("mixed 'a + b < c * d'") {
        Parser parser = makeParser("a + b < c * d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        std::string expected =
            "BinaryExpr: <\n"
            "  Left:\n"
            "    BinaryExpr: +\n"
            "      Left:\n"
            "        IdentifierExpression: a\n"
            "      Right:\n"
            "        IdentifierExpression: b\n"
            "  Right:\n"
            "    BinaryExpr: *\n"
            "      Left:\n"
            "        IdentifierExpression: c\n"
            "      Right:\n"
            "        IdentifierExpression: d\n";
        CHECK(dumpAST(expr.get()) == expected);
    }

    SECTION("chained 'a < b < c'") {
        Parser parser = makeParser("a < b < c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        std::string expected =
            "BinaryExpr: <\n"
            "  Left:\n"
            "    BinaryExpr: <\n"
            "      Left:\n"
            "        IdentifierExpression: a\n"
            "      Right:\n"
            "        IdentifierExpression: b\n"
            "  Right:\n"
            "    IdentifierExpression: c\n";
        CHECK(dumpAST(expr.get()) == expected);
    }
}

// -----------------------------------------------------------------------------
// 8. Error Handling for Malformed Relational Expressions
// -----------------------------------------------------------------------------
TEST_CASE("Relational Error: missing right operands", "[parser][relational][error]") {
    SECTION("missing right operand of '<': 'a <'") {
        Parser parser = makeParser("a <");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '<'");
    }

    SECTION("missing right operand of '>': 'a >'") {
        Parser parser = makeParser("a >");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '>'");
    }

    SECTION("missing right operand of '<=': 'a <='") {
        Parser parser = makeParser("a <=");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '<='");
    }

    SECTION("missing right operand of '>=': 'a >='") {
        Parser parser = makeParser("a >=");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '>='");
    }

    SECTION("missing right operand before semicolon: 'a < ;'") {
        Parser parser = makeParser("a < ;");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '<'");
    }

    SECTION("missing right operand before closing paren: '(a < )'") {
        Parser parser = makeParser("(a < )");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '<'");
    }

    SECTION("consecutive operators without operand: 'a + < b'") {
        Parser parser = makeParser("a + < b");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '+'");
    }

    SECTION("missing left operand: '< a'") {
        Parser parser = makeParser("< a");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK_FALSE(parser.isAtEnd());
        CHECK(parser.peek().lexeme == "<");
    }
}

