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
// DAY 23 TESTS - Equality Operators (==, !=)
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Individual Equality Operators
// -----------------------------------------------------------------------------
TEST_CASE("Equality: equal operator '=='", "[parser][equality]") {
    SECTION("identifiers 'a == b'") {
        Parser parser = makeParser("a == b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "==");
        CHECK(bin->left()->toString() == "IdentifierExpression(a)");
        CHECK(bin->right()->toString() == "IdentifierExpression(b)");
        CHECK(bin->toString() == "BinaryExpr(==, IdentifierExpression(a), IdentifierExpression(b))");
    }

    SECTION("integer literals '10 == 20'") {
        Parser parser = makeParser("10 == 20");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "==");
        CHECK(bin->left()->toString() == "LiteralExpression(Integer, 10)");
        CHECK(bin->right()->toString() == "LiteralExpression(Integer, 20)");
    }

    SECTION("float literals '3.14 == 2.71'") {
        Parser parser = makeParser("3.14 == 2.71");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "==");
        CHECK(bin->left()->toString() == "LiteralExpression(Float, 3.14)");
        CHECK(bin->right()->toString() == "LiteralExpression(Float, 2.71)");
    }

    SECTION("character literals ''x' == 'y''") {
        Parser parser = makeParser("'x' == 'y'");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "==");
        CHECK(bin->left()->toString() == "LiteralExpression(Char, 'x')");
        CHECK(bin->right()->toString() == "LiteralExpression(Char, 'y')");
    }

    SECTION("string literals '\"hello\" == \"world\"'") {
        Parser parser = makeParser("\"hello\" == \"world\"");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "==");
        CHECK(bin->left()->toString() == "LiteralExpression(String, \"hello\")");
        CHECK(bin->right()->toString() == "LiteralExpression(String, \"world\")");
    }
}

TEST_CASE("Equality: not-equal operator '!='", "[parser][equality]") {
    SECTION("identifiers 'a != b'") {
        Parser parser = makeParser("a != b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "!=");
        CHECK(bin->left()->toString() == "IdentifierExpression(a)");
        CHECK(bin->right()->toString() == "IdentifierExpression(b)");
        CHECK(bin->toString() == "BinaryExpr(!=, IdentifierExpression(a), IdentifierExpression(b))");
    }

    SECTION("integer literals '42 != 0'") {
        Parser parser = makeParser("42 != 0");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "!=");
        CHECK(bin->left()->toString() == "LiteralExpression(Integer, 42)");
        CHECK(bin->right()->toString() == "LiteralExpression(Integer, 0)");
    }

    SECTION("float literals '0.0 != 1.0'") {
        Parser parser = makeParser("0.0 != 1.0");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "!=");
        CHECK(bin->left()->toString() == "LiteralExpression(Float, 0.0)");
        CHECK(bin->right()->toString() == "LiteralExpression(Float, 1.0)");
    }
}

// -----------------------------------------------------------------------------
// 2. Precedence: Relational Operators Bind Tighter Than Equality
// -----------------------------------------------------------------------------
TEST_CASE("Equality Precedence: Relational operators bind tighter than equality", "[parser][equality][precedence]") {
    SECTION("'a < b == c >= d' -> (a < b) == (c >= d)") {
        Parser parser = makeParser("a < b == c >= d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* eq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");

        auto* leftRel = dynamic_cast<const BinaryExpr*>(eq->left());
        REQUIRE(leftRel != nullptr);
        CHECK(leftRel->op() == "<");
        CHECK(leftRel->left()->toString() == "IdentifierExpression(a)");
        CHECK(leftRel->right()->toString() == "IdentifierExpression(b)");

        auto* rightRel = dynamic_cast<const BinaryExpr*>(eq->right());
        REQUIRE(rightRel != nullptr);
        CHECK(rightRel->op() == ">=");
        CHECK(rightRel->left()->toString() == "IdentifierExpression(c)");
        CHECK(rightRel->right()->toString() == "IdentifierExpression(d)");
    }

    SECTION("'a <= b != c > d' -> (a <= b) != (c > d)") {
        Parser parser = makeParser("a <= b != c > d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* neq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(neq != nullptr);
        CHECK(neq->op() == "!=");

        auto* leftRel = dynamic_cast<const BinaryExpr*>(neq->left());
        REQUIRE(leftRel != nullptr);
        CHECK(leftRel->op() == "<=");
        CHECK(leftRel->left()->toString() == "IdentifierExpression(a)");
        CHECK(leftRel->right()->toString() == "IdentifierExpression(b)");

        auto* rightRel = dynamic_cast<const BinaryExpr*>(neq->right());
        REQUIRE(rightRel != nullptr);
        CHECK(rightRel->op() == ">");
        CHECK(rightRel->left()->toString() == "IdentifierExpression(c)");
        CHECK(rightRel->right()->toString() == "IdentifierExpression(d)");
    }

    SECTION("'a == b < c' -> a == (b < c)") {
        Parser parser = makeParser("a == b < c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* eq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");
        CHECK(eq->left()->toString() == "IdentifierExpression(a)");

        auto* rightRel = dynamic_cast<const BinaryExpr*>(eq->right());
        REQUIRE(rightRel != nullptr);
        CHECK(rightRel->op() == "<");
        CHECK(rightRel->left()->toString() == "IdentifierExpression(b)");
        CHECK(rightRel->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("'a > b == c' -> (a > b) == c") {
        Parser parser = makeParser("a > b == c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* eq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");

        auto* leftRel = dynamic_cast<const BinaryExpr*>(eq->left());
        REQUIRE(leftRel != nullptr);
        CHECK(leftRel->op() == ">");
        CHECK(leftRel->left()->toString() == "IdentifierExpression(a)");
        CHECK(leftRel->right()->toString() == "IdentifierExpression(b)");

        CHECK(eq->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("'a < b != c' -> (a < b) != c") {
        Parser parser = makeParser("a < b != c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* neq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(neq != nullptr);
        CHECK(neq->op() == "!=");

        auto* leftRel = dynamic_cast<const BinaryExpr*>(neq->left());
        REQUIRE(leftRel != nullptr);
        CHECK(leftRel->op() == "<");

        CHECK(neq->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("'a != b >= c' -> a != (b >= c)") {
        Parser parser = makeParser("a != b >= c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* neq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(neq != nullptr);
        CHECK(neq->op() == "!=");
        CHECK(neq->left()->toString() == "IdentifierExpression(a)");

        auto* rightRel = dynamic_cast<const BinaryExpr*>(neq->right());
        REQUIRE(rightRel != nullptr);
        CHECK(rightRel->op() == ">=");
    }
}

// -----------------------------------------------------------------------------
// 3. Precedence: Arithmetic Operators Bind Tighter Than Equality
// -----------------------------------------------------------------------------
TEST_CASE("Equality Precedence: Arithmetic operators bind tighter than equality", "[parser][equality][precedence]") {
    SECTION("'a + b == c * d' -> (a + b) == (c * d)") {
        Parser parser = makeParser("a + b == c * d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* eq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");

        auto* leftAdd = dynamic_cast<const BinaryExpr*>(eq->left());
        REQUIRE(leftAdd != nullptr);
        CHECK(leftAdd->op() == "+");
        CHECK(leftAdd->left()->toString() == "IdentifierExpression(a)");
        CHECK(leftAdd->right()->toString() == "IdentifierExpression(b)");

        auto* rightMul = dynamic_cast<const BinaryExpr*>(eq->right());
        REQUIRE(rightMul != nullptr);
        CHECK(rightMul->op() == "*");
        CHECK(rightMul->left()->toString() == "IdentifierExpression(c)");
        CHECK(rightMul->right()->toString() == "IdentifierExpression(d)");
    }

    SECTION("'a - b != c / d % e' -> (a - b) != ((c / d) % e)") {
        Parser parser = makeParser("a - b != c / d % e");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* neq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(neq != nullptr);
        CHECK(neq->op() == "!=");

        auto* leftSub = dynamic_cast<const BinaryExpr*>(neq->left());
        REQUIRE(leftSub != nullptr);
        CHECK(leftSub->op() == "-");

        auto* rightMod = dynamic_cast<const BinaryExpr*>(neq->right());
        REQUIRE(rightMod != nullptr);
        CHECK(rightMod->op() == "%");
        CHECK(rightMod->right()->toString() == "IdentifierExpression(e)");

        auto* rightDiv = dynamic_cast<const BinaryExpr*>(rightMod->left());
        REQUIRE(rightDiv != nullptr);
        CHECK(rightDiv->op() == "/");
        CHECK(rightDiv->left()->toString() == "IdentifierExpression(c)");
        CHECK(rightDiv->right()->toString() == "IdentifierExpression(d)");
    }

    SECTION("'a * b == c + d' -> (a * b) == (c + d)") {
        Parser parser = makeParser("a * b == c + d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* eq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");

        auto* leftMul = dynamic_cast<const BinaryExpr*>(eq->left());
        REQUIRE(leftMul != nullptr);
        CHECK(leftMul->op() == "*");

        auto* rightAdd = dynamic_cast<const BinaryExpr*>(eq->right());
        REQUIRE(rightAdd != nullptr);
        CHECK(rightAdd->op() == "+");
    }

    SECTION("'a % b != c - d' -> (a % b) != (c - d)") {
        Parser parser = makeParser("a % b != c - d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* neq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(neq != nullptr);
        CHECK(neq->op() == "!=");

        auto* leftMod = dynamic_cast<const BinaryExpr*>(neq->left());
        REQUIRE(leftMod != nullptr);
        CHECK(leftMod->op() == "%");

        auto* rightSub = dynamic_cast<const BinaryExpr*>(neq->right());
        REQUIRE(rightSub != nullptr);
        CHECK(rightSub->op() == "-");
    }

    SECTION("'a + b * c == d / e - f' -> (a + (b * c)) == ((d / e) - f)") {
        Parser parser = makeParser("a + b * c == d / e - f");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* eq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");

        auto* leftAdd = dynamic_cast<const BinaryExpr*>(eq->left());
        REQUIRE(leftAdd != nullptr);
        CHECK(leftAdd->op() == "+");
        CHECK(leftAdd->left()->toString() == "IdentifierExpression(a)");
        auto* mul = dynamic_cast<const BinaryExpr*>(leftAdd->right());
        REQUIRE(mul != nullptr);
        CHECK(mul->op() == "*");

        auto* rightSub = dynamic_cast<const BinaryExpr*>(eq->right());
        REQUIRE(rightSub != nullptr);
        CHECK(rightSub->op() == "-");
        CHECK(rightSub->right()->toString() == "IdentifierExpression(f)");
        auto* div = dynamic_cast<const BinaryExpr*>(rightSub->left());
        REQUIRE(div != nullptr);
        CHECK(div->op() == "/");
    }
}

// -----------------------------------------------------------------------------
// 4. Left-Associativity of Equality Operators
// -----------------------------------------------------------------------------
TEST_CASE("Equality Associativity: Equality operators are left-associative", "[parser][equality][associativity]") {
    SECTION("'a == b == c' -> (a == b) == c") {
        Parser parser = makeParser("a == b == c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == "==");
        CHECK(outer->right()->toString() == "IdentifierExpression(c)");

        auto* inner = dynamic_cast<const BinaryExpr*>(outer->left());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == "==");
        CHECK(inner->left()->toString() == "IdentifierExpression(a)");
        CHECK(inner->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'a != b != c' -> (a != b) != c") {
        Parser parser = makeParser("a != b != c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == "!=");
        CHECK(outer->right()->toString() == "IdentifierExpression(c)");

        auto* inner = dynamic_cast<const BinaryExpr*>(outer->left());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == "!=");
        CHECK(inner->left()->toString() == "IdentifierExpression(a)");
        CHECK(inner->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'a == b != c' -> (a == b) != c") {
        Parser parser = makeParser("a == b != c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == "!=");
        CHECK(outer->right()->toString() == "IdentifierExpression(c)");

        auto* inner = dynamic_cast<const BinaryExpr*>(outer->left());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == "==");
        CHECK(inner->left()->toString() == "IdentifierExpression(a)");
        CHECK(inner->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'a != b == c' -> (a != b) == c") {
        Parser parser = makeParser("a != b == c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == "==");
        CHECK(outer->right()->toString() == "IdentifierExpression(c)");

        auto* inner = dynamic_cast<const BinaryExpr*>(outer->left());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == "!=");
        CHECK(inner->left()->toString() == "IdentifierExpression(a)");
        CHECK(inner->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("chained chain 'a != b == c != d == e' -> ((((a != b) == c) != d) == e)") {
        Parser parser = makeParser("a != b == c != d == e");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        // Level 4: outer is '=='
        auto* op4 = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(op4 != nullptr);
        CHECK(op4->op() == "==");
        CHECK(op4->right()->toString() == "IdentifierExpression(e)");

        // Level 3: inner is '!='
        auto* op3 = dynamic_cast<const BinaryExpr*>(op4->left());
        REQUIRE(op3 != nullptr);
        CHECK(op3->op() == "!=");
        CHECK(op3->right()->toString() == "IdentifierExpression(d)");

        // Level 2: inner is '=='
        auto* op2 = dynamic_cast<const BinaryExpr*>(op3->left());
        REQUIRE(op2 != nullptr);
        CHECK(op2->op() == "==");
        CHECK(op2->right()->toString() == "IdentifierExpression(c)");

        // Level 1: inner is '!='
        auto* op1 = dynamic_cast<const BinaryExpr*>(op2->left());
        REQUIRE(op1 != nullptr);
        CHECK(op1->op() == "!=");
        CHECK(op1->left()->toString() == "IdentifierExpression(a)");
        CHECK(op1->right()->toString() == "IdentifierExpression(b)");
    }
}

// -----------------------------------------------------------------------------
// 5. Interaction with Prefix Unary Operators
// -----------------------------------------------------------------------------
TEST_CASE("Equality Precedence: Prefix unary operators bind tighter", "[parser][equality][unary]") {
    SECTION("'-a == !b' -> (-a) == (!b)") {
        Parser parser = makeParser("-a == !b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* eq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");

        auto* leftUn = dynamic_cast<const UnaryExpr*>(eq->left());
        REQUIRE(leftUn != nullptr);
        CHECK(leftUn->op() == "-");
        CHECK(leftUn->operand()->toString() == "IdentifierExpression(a)");

        auto* rightUn = dynamic_cast<const UnaryExpr*>(eq->right());
        REQUIRE(rightUn != nullptr);
        CHECK(rightUn->op() == "!");
        CHECK(rightUn->operand()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'++a != --b' -> (++a) != (--b)") {
        Parser parser = makeParser("++a != --b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* neq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(neq != nullptr);
        CHECK(neq->op() == "!=");

        auto* leftUn = dynamic_cast<const UnaryExpr*>(neq->left());
        REQUIRE(leftUn != nullptr);
        CHECK(leftUn->op() == "++");

        auto* rightUn = dynamic_cast<const UnaryExpr*>(neq->right());
        REQUIRE(rightUn != nullptr);
        CHECK(rightUn->op() == "--");
    }

    SECTION("'~a == -b' -> (~a) == (-b)") {
        Parser parser = makeParser("~a == -b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* eq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");

        auto* leftUn = dynamic_cast<const UnaryExpr*>(eq->left());
        REQUIRE(leftUn != nullptr);
        CHECK(leftUn->op() == "~");

        auto* rightUn = dynamic_cast<const UnaryExpr*>(eq->right());
        REQUIRE(rightUn != nullptr);
        CHECK(rightUn->op() == "-");
    }

    SECTION("'!a == b' -> (!a) == b") {
        Parser parser = makeParser("!a == b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* eq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");

        auto* leftUn = dynamic_cast<const UnaryExpr*>(eq->left());
        REQUIRE(leftUn != nullptr);
        CHECK(leftUn->op() == "!");

        CHECK(eq->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'a != !b' -> a != (!b)") {
        Parser parser = makeParser("a != !b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* neq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(neq != nullptr);
        CHECK(neq->op() == "!=");
        CHECK(neq->left()->toString() == "IdentifierExpression(a)");

        auto* rightUn = dynamic_cast<const UnaryExpr*>(neq->right());
        REQUIRE(rightUn != nullptr);
        CHECK(rightUn->op() == "!");
    }

    SECTION("'-(a == b)' -> -(a == b)") {
        Parser parser = makeParser("-(a == b)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* un = dynamic_cast<UnaryExpr*>(expr.get());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "-");

        auto* eq = dynamic_cast<const BinaryExpr*>(un->operand());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");
    }

    SECTION("'!(a != b)' -> !(a != b)") {
        Parser parser = makeParser("!(a != b)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* un = dynamic_cast<UnaryExpr*>(expr.get());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "!");

        auto* neq = dynamic_cast<const BinaryExpr*>(un->operand());
        REQUIRE(neq != nullptr);
        CHECK(neq->op() == "!=");
    }
}

// -----------------------------------------------------------------------------
// 6. Interaction with Assignment Operators
// -----------------------------------------------------------------------------
TEST_CASE("Equality Precedence: Assignment has lower precedence than equality", "[parser][equality][assignment]") {
    SECTION("'x = a == b' -> x = (a == b)") {
        Parser parser = makeParser("x = a == b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "=");
        CHECK(assign->left()->toString() == "IdentifierExpression(x)");

        auto* eq = dynamic_cast<const BinaryExpr*>(assign->right());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");
        CHECK(eq->left()->toString() == "IdentifierExpression(a)");
        CHECK(eq->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'x = a != b' -> x = (a != b)") {
        Parser parser = makeParser("x = a != b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "=");
        CHECK(assign->left()->toString() == "IdentifierExpression(x)");

        auto* neq = dynamic_cast<const BinaryExpr*>(assign->right());
        REQUIRE(neq != nullptr);
        CHECK(neq->op() == "!=");
        CHECK(neq->left()->toString() == "IdentifierExpression(a)");
        CHECK(neq->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'x = a < b == c + d' -> x = ((a < b) == (c + d))") {
        Parser parser = makeParser("x = a < b == c + d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "=");
        CHECK(assign->left()->toString() == "IdentifierExpression(x)");

        auto* eq = dynamic_cast<const BinaryExpr*>(assign->right());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");

        auto* rel = dynamic_cast<const BinaryExpr*>(eq->left());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");

        auto* add = dynamic_cast<const BinaryExpr*>(eq->right());
        REQUIRE(add != nullptr);
        CHECK(add->op() == "+");
    }

    SECTION("'x += a == b' -> x += (a == b)") {
        Parser parser = makeParser("x += a == b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "+=");
        CHECK(assign->left()->toString() == "IdentifierExpression(x)");

        auto* eq = dynamic_cast<const BinaryExpr*>(assign->right());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");
    }

    SECTION("'x = y = a == b' -> x = (y = (a == b))") {
        Parser parser = makeParser("x = y = a == b");
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

        auto* eq = dynamic_cast<const BinaryExpr*>(innerAssign->right());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");
        CHECK(eq->left()->toString() == "IdentifierExpression(a)");
        CHECK(eq->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'x = a == b != c' -> x = ((a == b) != c)") {
        Parser parser = makeParser("x = a == b != c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "=");

        auto* outerNeq = dynamic_cast<const BinaryExpr*>(assign->right());
        REQUIRE(outerNeq != nullptr);
        CHECK(outerNeq->op() == "!=");
        CHECK(outerNeq->right()->toString() == "IdentifierExpression(c)");

        auto* innerEq = dynamic_cast<const BinaryExpr*>(outerNeq->left());
        REQUIRE(innerEq != nullptr);
        CHECK(innerEq->op() == "==");
        CHECK(innerEq->left()->toString() == "IdentifierExpression(a)");
        CHECK(innerEq->right()->toString() == "IdentifierExpression(b)");
    }
}

// -----------------------------------------------------------------------------
// 7. Parentheses Overriding Precedence
// -----------------------------------------------------------------------------
TEST_CASE("Equality Precedence: Parentheses override normal precedence", "[parser][equality][parens]") {
    SECTION("'(a == b) == c'") {
        Parser parser = makeParser("(a == b) == c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == "==");
        CHECK(outer->right()->toString() == "IdentifierExpression(c)");

        auto* inner = dynamic_cast<const BinaryExpr*>(outer->left());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == "==");
    }

    SECTION("'a == (b == c)' forces right-associativity") {
        Parser parser = makeParser("a == (b == c)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == "==");
        CHECK(outer->left()->toString() == "IdentifierExpression(a)");

        auto* inner = dynamic_cast<const BinaryExpr*>(outer->right());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == "==");
        CHECK(inner->left()->toString() == "IdentifierExpression(b)");
        CHECK(inner->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("'(a == b) + c' overrides arithmetic precedence") {
        Parser parser = makeParser("(a == b) + c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* add = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(add != nullptr);
        CHECK(add->op() == "+");
        CHECK(add->right()->toString() == "IdentifierExpression(c)");

        auto* eq = dynamic_cast<const BinaryExpr*>(add->left());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");
    }

    SECTION("'a * (b != c)'") {
        Parser parser = makeParser("a * (b != c)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* mul = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(mul != nullptr);
        CHECK(mul->op() == "*");
        CHECK(mul->left()->toString() == "IdentifierExpression(a)");

        auto* neq = dynamic_cast<const BinaryExpr*>(mul->right());
        REQUIRE(neq != nullptr);
        CHECK(neq->op() == "!=");
    }

    SECTION("'(a == b) < c' overrides relational precedence") {
        Parser parser = makeParser("(a == b) < c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* rel = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");
        CHECK(rel->right()->toString() == "IdentifierExpression(c)");

        auto* eq = dynamic_cast<const BinaryExpr*>(rel->left());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");
    }
}

// -----------------------------------------------------------------------------
// 8. AST Printer Representation
// -----------------------------------------------------------------------------
TEST_CASE("Equality ASTPrinter output", "[parser][equality][printer]") {
    SECTION("simple 'a == b'") {
        Parser parser = makeParser("a == b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        std::string expected =
            "BinaryExpr: ==\n"
            "  Left:\n"
            "    IdentifierExpression: a\n"
            "  Right:\n"
            "    IdentifierExpression: b\n";
        CHECK(dumpAST(expr.get()) == expected);
    }

    SECTION("simple 'a != b'") {
        Parser parser = makeParser("a != b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        std::string expected =
            "BinaryExpr: !=\n"
            "  Left:\n"
            "    IdentifierExpression: a\n"
            "  Right:\n"
            "    IdentifierExpression: b\n";
        CHECK(dumpAST(expr.get()) == expected);
    }

    SECTION("relational and equality 'a < b == c >= d'") {
        Parser parser = makeParser("a < b == c >= d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        std::string expected =
            "BinaryExpr: ==\n"
            "  Left:\n"
            "    BinaryExpr: <\n"
            "      Left:\n"
            "        IdentifierExpression: a\n"
            "      Right:\n"
            "        IdentifierExpression: b\n"
            "  Right:\n"
            "    BinaryExpr: >=\n"
            "      Left:\n"
            "        IdentifierExpression: c\n"
            "      Right:\n"
            "        IdentifierExpression: d\n";
        CHECK(dumpAST(expr.get()) == expected);
    }

    SECTION("chained equality 'a == b != c'") {
        Parser parser = makeParser("a == b != c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        std::string expected =
            "BinaryExpr: !=\n"
            "  Left:\n"
            "    BinaryExpr: ==\n"
            "      Left:\n"
            "        IdentifierExpression: a\n"
            "      Right:\n"
            "        IdentifierExpression: b\n"
            "  Right:\n"
            "    IdentifierExpression: c\n";
        CHECK(dumpAST(expr.get()) == expected);
    }

    SECTION("assignment with equality 'x = a == b'") {
        Parser parser = makeParser("x = a == b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        std::string expected =
            "AssignExpr: =\n"
            "  Left:\n"
            "    IdentifierExpression: x\n"
            "  Right:\n"
            "    BinaryExpr: ==\n"
            "      Left:\n"
            "        IdentifierExpression: a\n"
            "      Right:\n"
            "        IdentifierExpression: b\n";
        CHECK(dumpAST(expr.get()) == expected);
    }
}

// -----------------------------------------------------------------------------
// 9. Error Handling for Malformed Equality Expressions
// -----------------------------------------------------------------------------
TEST_CASE("Equality Error: missing operands and malformed input", "[parser][equality][error]") {
    SECTION("missing right operand of '==': 'a =='") {
        Parser parser = makeParser("a ==");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '=='");
    }

    SECTION("missing right operand of '!=': 'a !='") {
        Parser parser = makeParser("a !=");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '!='");
    }

    SECTION("missing right operand before semicolon: 'a == ;'") {
        Parser parser = makeParser("a == ;");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '=='");
    }

    SECTION("missing right operand before closing paren: '(a == )'") {
        Parser parser = makeParser("(a == )");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '=='");
    }

    SECTION("consecutive operators: 'a + == b'") {
        Parser parser = makeParser("a + == b");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '+'");
    }

    SECTION("consecutive equality and relational: 'a == < b'") {
        Parser parser = makeParser("a == < b");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '=='");
    }

    SECTION("missing left operand: '== a'") {
        Parser parser = makeParser("== a");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK_FALSE(parser.isAtEnd());
        CHECK(parser.peek().lexeme == "==");
    }

    SECTION("missing left operand: '!= a'") {
        Parser parser = makeParser("!= a");
        auto expr = parser.parseExpression();
        // != is not prefix unary, so it fails at primary
        CHECK(expr == nullptr);
        CHECK_FALSE(parser.isAtEnd());
        CHECK(parser.peek().lexeme == "!=");
    }
}

// -----------------------------------------------------------------------------
// 10. Regression Across All Precedence Tiers
// -----------------------------------------------------------------------------
TEST_CASE("Equality Precedence Regression: Complete precedence pipeline", "[parser][equality][regression]") {
    SECTION("'x = a + b * c < d == e != f' groups through all tiers") {
        Parser parser = makeParser("x = a + b * c < d == e != f");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        // Level 0: Assignment '='
        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "=");
        CHECK(assign->left()->toString() == "IdentifierExpression(x)");

        // Level 1: Outer equality '!='
        auto* neq = dynamic_cast<const BinaryExpr*>(assign->right());
        REQUIRE(neq != nullptr);
        CHECK(neq->op() == "!=");
        CHECK(neq->right()->toString() == "IdentifierExpression(f)");

        // Level 1: Left of '!=' is '=='
        auto* eq = dynamic_cast<const BinaryExpr*>(neq->left());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");
        CHECK(eq->right()->toString() == "IdentifierExpression(e)");

        // Level 2: Left of '==' is '<'
        auto* rel = dynamic_cast<const BinaryExpr*>(eq->left());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");
        CHECK(rel->right()->toString() == "IdentifierExpression(d)");

        // Level 3: Left of '<' is '+'
        auto* add = dynamic_cast<const BinaryExpr*>(rel->left());
        REQUIRE(add != nullptr);
        CHECK(add->op() == "+");
        CHECK(add->left()->toString() == "IdentifierExpression(a)");

        // Level 4: Right of '+' is '*'
        auto* mul = dynamic_cast<const BinaryExpr*>(add->right());
        REQUIRE(mul != nullptr);
        CHECK(mul->op() == "*");
        CHECK(mul->left()->toString() == "IdentifierExpression(b)");
        CHECK(mul->right()->toString() == "IdentifierExpression(c)");
    }
}

