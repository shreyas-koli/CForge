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
// DAY 25 TESTS - Bitwise & Shift Operators (&, |, ^, ~, <<, >>)
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Individual Bitwise & Shift Operators
// -----------------------------------------------------------------------------
TEST_CASE("Bitwise: individual binary operators '&', '|', '^'", "[parser][bitwise]") {
    SECTION("bitwise AND 'a & b'") {
        Parser parser = makeParser("a & b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "&");
        CHECK(bin->left()->toString() == "IdentifierExpression(a)");
        CHECK(bin->right()->toString() == "IdentifierExpression(b)");
        CHECK(bin->toString() == "BinaryExpr(&, IdentifierExpression(a), IdentifierExpression(b))");
    }

    SECTION("bitwise OR 'a | b'") {
        Parser parser = makeParser("a | b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "|");
        CHECK(bin->left()->toString() == "IdentifierExpression(a)");
        CHECK(bin->right()->toString() == "IdentifierExpression(b)");
        CHECK(bin->toString() == "BinaryExpr(|, IdentifierExpression(a), IdentifierExpression(b))");
    }

    SECTION("bitwise XOR 'a ^ b'") {
        Parser parser = makeParser("a ^ b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "^");
        CHECK(bin->left()->toString() == "IdentifierExpression(a)");
        CHECK(bin->right()->toString() == "IdentifierExpression(b)");
        CHECK(bin->toString() == "BinaryExpr(^, IdentifierExpression(a), IdentifierExpression(b))");
    }

    SECTION("bitwise with literals 'flags & 0xFF'") {
        Parser parser = makeParser("flags & 0xFF");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "&");
        CHECK(bin->left()->toString() == "IdentifierExpression(flags)");
        CHECK(bin->right()->toString() == "LiteralExpression(Integer, 0xFF)");
    }
}

TEST_CASE("Shift: individual operators '<<' and '>>'", "[parser][shift]") {
    SECTION("left shift 'a << 2'") {
        Parser parser = makeParser("a << 2");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "<<");
        CHECK(bin->left()->toString() == "IdentifierExpression(a)");
        CHECK(bin->right()->toString() == "LiteralExpression(Integer, 2)");
        CHECK(bin->toString() == "BinaryExpr(<<, IdentifierExpression(a), LiteralExpression(Integer, 2))");
    }

    SECTION("right shift 'a >> 2'") {
        Parser parser = makeParser("a >> 2");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == ">>");
        CHECK(bin->left()->toString() == "IdentifierExpression(a)");
        CHECK(bin->right()->toString() == "LiteralExpression(Integer, 2)");
        CHECK(bin->toString() == "BinaryExpr(>>, IdentifierExpression(a), LiteralExpression(Integer, 2))");
    }
}

TEST_CASE("Bitwise NOT: prefix unary '~'", "[parser][bitwise][unary]") {
    SECTION("simple '~a'") {
        Parser parser = makeParser("~a");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* un = dynamic_cast<UnaryExpr*>(expr.get());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "~");
        CHECK(un->operand()->toString() == "IdentifierExpression(a)");
        CHECK(un->toString() == "UnaryExpr(~, IdentifierExpression(a))");
    }

    SECTION("nested unary '~!a'") {
        Parser parser = makeParser("~!a");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<UnaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == "~");

        auto* inner = dynamic_cast<const UnaryExpr*>(outer->operand());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == "!");
        CHECK(inner->operand()->toString() == "IdentifierExpression(a)");
    }
}

// -----------------------------------------------------------------------------
// 2. Precedence: Shift vs Arithmetic and Relational
// -----------------------------------------------------------------------------
TEST_CASE("Shift Precedence: Arithmetic binds tighter than Shift", "[parser][shift][precedence]") {
    SECTION("'a + b << 2' -> (a + b) << 2") {
        Parser parser = makeParser("a + b << 2");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* sh = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(sh != nullptr);
        CHECK(sh->op() == "<<");
        CHECK(sh->right()->toString() == "LiteralExpression(Integer, 2)");

        auto* add = dynamic_cast<const BinaryExpr*>(sh->left());
        REQUIRE(add != nullptr);
        CHECK(add->op() == "+");
        CHECK(add->left()->toString() == "IdentifierExpression(a)");
        CHECK(add->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'a << b + c' -> a << (b + c)") {
        Parser parser = makeParser("a << b + c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* sh = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(sh != nullptr);
        CHECK(sh->op() == "<<");
        CHECK(sh->left()->toString() == "IdentifierExpression(a)");

        auto* add = dynamic_cast<const BinaryExpr*>(sh->right());
        REQUIRE(add != nullptr);
        CHECK(add->op() == "+");
        CHECK(add->left()->toString() == "IdentifierExpression(b)");
        CHECK(add->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("'a * b >> c - d' -> (a * b) >> (c - d)") {
        Parser parser = makeParser("a * b >> c - d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* sh = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(sh != nullptr);
        CHECK(sh->op() == ">>");

        auto* mul = dynamic_cast<const BinaryExpr*>(sh->left());
        REQUIRE(mul != nullptr);
        CHECK(mul->op() == "*");

        auto* sub = dynamic_cast<const BinaryExpr*>(sh->right());
        REQUIRE(sub != nullptr);
        CHECK(sub->op() == "-");
    }
}

TEST_CASE("Shift Precedence: Shift binds tighter than Relational", "[parser][shift][precedence]") {
    SECTION("'a < b << c' -> a < (b << c)") {
        Parser parser = makeParser("a < b << c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* rel = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");
        CHECK(rel->left()->toString() == "IdentifierExpression(a)");

        auto* sh = dynamic_cast<const BinaryExpr*>(rel->right());
        REQUIRE(sh != nullptr);
        CHECK(sh->op() == "<<");
        CHECK(sh->left()->toString() == "IdentifierExpression(b)");
        CHECK(sh->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("'a >> b >= c' -> (a >> b) >= c") {
        Parser parser = makeParser("a >> b >= c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* rel = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == ">=");
        CHECK(rel->right()->toString() == "IdentifierExpression(c)");

        auto* sh = dynamic_cast<const BinaryExpr*>(rel->left());
        REQUIRE(sh != nullptr);
        CHECK(sh->op() == ">>");
        CHECK(sh->left()->toString() == "IdentifierExpression(a)");
        CHECK(sh->right()->toString() == "IdentifierExpression(b)");
    }
}

// -----------------------------------------------------------------------------
// 3. Precedence: Bitwise AND, XOR, OR and Equality
// -----------------------------------------------------------------------------
TEST_CASE("Bitwise Precedence: Distinct levels for &, ^, and |", "[parser][bitwise][precedence]") {
    SECTION("'a & b ^ c | d' -> ((a & b) ^ c) | d") {
        Parser parser = makeParser("a & b ^ c | d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        // Outer operator is '|' (lowest bitwise precedence)
        auto* orOp = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(orOp != nullptr);
        CHECK(orOp->op() == "|");
        CHECK(orOp->right()->toString() == "IdentifierExpression(d)");

        // Left of '|' is '^'
        auto* xorOp = dynamic_cast<const BinaryExpr*>(orOp->left());
        REQUIRE(xorOp != nullptr);
        CHECK(xorOp->op() == "^");
        CHECK(xorOp->right()->toString() == "IdentifierExpression(c)");

        // Left of '^' is '&'
        auto* andOp = dynamic_cast<const BinaryExpr*>(xorOp->left());
        REQUIRE(andOp != nullptr);
        CHECK(andOp->op() == "&");
        CHECK(andOp->left()->toString() == "IdentifierExpression(a)");
        CHECK(andOp->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'a | b & c' -> a | (b & c)") {
        Parser parser = makeParser("a | b & c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* orOp = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(orOp != nullptr);
        CHECK(orOp->op() == "|");
        CHECK(orOp->left()->toString() == "IdentifierExpression(a)");

        auto* andOp = dynamic_cast<const BinaryExpr*>(orOp->right());
        REQUIRE(andOp != nullptr);
        CHECK(andOp->op() == "&");
        CHECK(andOp->left()->toString() == "IdentifierExpression(b)");
        CHECK(andOp->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("'a ^ b | c & d' -> (a ^ b) | (c & d)") {
        Parser parser = makeParser("a ^ b | c & d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* orOp = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(orOp != nullptr);
        CHECK(orOp->op() == "|");

        auto* xorOp = dynamic_cast<const BinaryExpr*>(orOp->left());
        REQUIRE(xorOp != nullptr);
        CHECK(xorOp->op() == "^");

        auto* andOp = dynamic_cast<const BinaryExpr*>(orOp->right());
        REQUIRE(andOp != nullptr);
        CHECK(andOp->op() == "&");
    }
}

TEST_CASE("Bitwise vs Equality Precedence: 'a & b == c' -> (a & b) == c", "[parser][bitwise][equality]") {
    SECTION("'a & b == c' parses as (a & b) == c") {
        Parser parser = makeParser("a & b == c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* eq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");
        CHECK(eq->right()->toString() == "IdentifierExpression(c)");

        auto* andOp = dynamic_cast<const BinaryExpr*>(eq->left());
        REQUIRE(andOp != nullptr);
        CHECK(andOp->op() == "&");
        CHECK(andOp->left()->toString() == "IdentifierExpression(a)");
        CHECK(andOp->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'a == b & c' parses as (a == b) or a == (b & c)? '&' has higher precedence: a == (b & c)") {
        Parser parser = makeParser("a == b & c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* eq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");
        CHECK(eq->left()->toString() == "IdentifierExpression(a)");

        auto* andOp = dynamic_cast<const BinaryExpr*>(eq->right());
        REQUIRE(andOp != nullptr);
        CHECK(andOp->op() == "&");
        CHECK(andOp->left()->toString() == "IdentifierExpression(b)");
        CHECK(andOp->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("'a & b != c' -> (a & b) != c") {
        Parser parser = makeParser("a & b != c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* neq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(neq != nullptr);
        CHECK(neq->op() == "!=");

        auto* andOp = dynamic_cast<const BinaryExpr*>(neq->left());
        REQUIRE(andOp != nullptr);
        CHECK(andOp->op() == "&");
    }
}

// -----------------------------------------------------------------------------
// 4. Left-Associativity of Bitwise and Shift Operators
// -----------------------------------------------------------------------------
TEST_CASE("Bitwise and Shift Associativity: Left-to-right grouping", "[parser][bitwise][shift][associativity]") {
    SECTION("'a << 2 << 3' -> (a << 2) << 3") {
        Parser parser = makeParser("a << 2 << 3");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == "<<");
        CHECK(outer->right()->toString() == "LiteralExpression(Integer, 3)");

        auto* inner = dynamic_cast<const BinaryExpr*>(outer->left());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == "<<");
        CHECK(inner->left()->toString() == "IdentifierExpression(a)");
        CHECK(inner->right()->toString() == "LiteralExpression(Integer, 2)");
    }

    SECTION("'a >> 1 >> 2' -> (a >> 1) >> 2") {
        Parser parser = makeParser("a >> 1 >> 2");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == ">>");

        auto* inner = dynamic_cast<const BinaryExpr*>(outer->left());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == ">>");
    }

    SECTION("'a & b & c' -> (a & b) & c") {
        Parser parser = makeParser("a & b & c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == "&");
        CHECK(outer->right()->toString() == "IdentifierExpression(c)");

        auto* inner = dynamic_cast<const BinaryExpr*>(outer->left());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == "&");
    }

    SECTION("'a ^ b ^ c' -> (a ^ b) ^ c") {
        Parser parser = makeParser("a ^ b ^ c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == "^");

        auto* inner = dynamic_cast<const BinaryExpr*>(outer->left());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == "^");
    }

    SECTION("'a | b | c' -> (a | b) | c") {
        Parser parser = makeParser("a | b | c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* outer = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(outer != nullptr);
        CHECK(outer->op() == "|");

        auto* inner = dynamic_cast<const BinaryExpr*>(outer->left());
        REQUIRE(inner != nullptr);
        CHECK(inner->op() == "|");
    }
}

// -----------------------------------------------------------------------------
// 5. Parentheses Overriding Precedence
// -----------------------------------------------------------------------------
TEST_CASE("Bitwise and Shift Parentheses: Parentheses override precedence", "[parser][bitwise][parens]") {
    SECTION("'(a < b) << c' overrides shift > relational") {
        Parser parser = makeParser("(a < b) << c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* sh = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(sh != nullptr);
        CHECK(sh->op() == "<<");
        CHECK(sh->right()->toString() == "IdentifierExpression(c)");

        auto* rel = dynamic_cast<const BinaryExpr*>(sh->left());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");
    }

    SECTION("'a & (b == c)' forces equality first") {
        Parser parser = makeParser("a & (b == c)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* andOp = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(andOp != nullptr);
        CHECK(andOp->op() == "&");
        CHECK(andOp->left()->toString() == "IdentifierExpression(a)");

        auto* eq = dynamic_cast<const BinaryExpr*>(andOp->right());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");
    }

    SECTION("'(a & b) == c' explicit grouping") {
        Parser parser = makeParser("(a & b) == c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* eq = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(eq != nullptr);
        CHECK(eq->op() == "==");
        CHECK(eq->right()->toString() == "IdentifierExpression(c)");

        auto* andOp = dynamic_cast<const BinaryExpr*>(eq->left());
        REQUIRE(andOp != nullptr);
        CHECK(andOp->op() == "&");
    }

    SECTION("'a & (b ^ c)' overrides & > ^") {
        Parser parser = makeParser("a & (b ^ c)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* andOp = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(andOp != nullptr);
        CHECK(andOp->op() == "&");
        CHECK(andOp->left()->toString() == "IdentifierExpression(a)");

        auto* xorOp = dynamic_cast<const BinaryExpr*>(andOp->right());
        REQUIRE(xorOp != nullptr);
        CHECK(xorOp->op() == "^");
    }

    SECTION("'(a | b) & c' overrides & > |") {
        Parser parser = makeParser("(a | b) & c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* andOp = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(andOp != nullptr);
        CHECK(andOp->op() == "&");
        CHECK(andOp->right()->toString() == "IdentifierExpression(c)");

        auto* orOp = dynamic_cast<const BinaryExpr*>(andOp->left());
        REQUIRE(orOp != nullptr);
        CHECK(orOp->op() == "|");
    }
}

// -----------------------------------------------------------------------------
// 6. Interaction with Unary Operators and Assignment
// -----------------------------------------------------------------------------
TEST_CASE("Bitwise with Unary Operators and Assignment", "[parser][bitwise][unary][assignment]") {
    SECTION("'~a & b' -> (~a) & b") {
        Parser parser = makeParser("~a & b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* andOp = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(andOp != nullptr);
        CHECK(andOp->op() == "&");
        CHECK(andOp->right()->toString() == "IdentifierExpression(b)");

        auto* un = dynamic_cast<const UnaryExpr*>(andOp->left());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "~");
        CHECK(un->operand()->toString() == "IdentifierExpression(a)");
    }

    SECTION("'a | ~b' -> a | (~b)") {
        Parser parser = makeParser("a | ~b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* orOp = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(orOp != nullptr);
        CHECK(orOp->op() == "|");
        CHECK(orOp->left()->toString() == "IdentifierExpression(a)");

        auto* un = dynamic_cast<const UnaryExpr*>(orOp->right());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "~");
        CHECK(un->operand()->toString() == "IdentifierExpression(b)");
    }

    SECTION("'~a << 2' -> (~a) << 2") {
        Parser parser = makeParser("~a << 2");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* sh = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(sh != nullptr);
        CHECK(sh->op() == "<<");

        auto* un = dynamic_cast<const UnaryExpr*>(sh->left());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "~");
    }

    SECTION("'~(a & b)' -> ~(a & b)") {
        Parser parser = makeParser("~(a & b)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* un = dynamic_cast<UnaryExpr*>(expr.get());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "~");

        auto* andOp = dynamic_cast<const BinaryExpr*>(un->operand());
        REQUIRE(andOp != nullptr);
        CHECK(andOp->op() == "&");
    }

    SECTION("'x = a & b' assignment has lower precedence") {
        Parser parser = makeParser("x = a & b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "=");
        CHECK(assign->left()->toString() == "IdentifierExpression(x)");

        auto* andOp = dynamic_cast<const BinaryExpr*>(assign->right());
        REQUIRE(andOp != nullptr);
        CHECK(andOp->op() == "&");
    }

    SECTION("'x = a + b << 2'") {
        Parser parser = makeParser("x = a + b << 2");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "=");

        auto* sh = dynamic_cast<const BinaryExpr*>(assign->right());
        REQUIRE(sh != nullptr);
        CHECK(sh->op() == "<<");

        auto* add = dynamic_cast<const BinaryExpr*>(sh->left());
        REQUIRE(add != nullptr);
        CHECK(add->op() == "+");
    }
}

// -----------------------------------------------------------------------------
// 7. AST Printer Representation
// -----------------------------------------------------------------------------
TEST_CASE("Bitwise and Shift ASTPrinter output", "[parser][bitwise][printer]") {
    SECTION("simple 'a & b'") {
        Parser parser = makeParser("a & b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        std::string expected =
            "BinaryExpr: &\n"
            "  Left:\n"
            "    IdentifierExpression: a\n"
            "  Right:\n"
            "    IdentifierExpression: b\n";
        CHECK(dumpAST(expr.get()) == expected);
    }

    SECTION("'a + b << 2'") {
        Parser parser = makeParser("a + b << 2");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        std::string expected =
            "BinaryExpr: <<\n"
            "  Left:\n"
            "    BinaryExpr: +\n"
            "      Left:\n"
            "        IdentifierExpression: a\n"
            "      Right:\n"
            "        IdentifierExpression: b\n"
            "  Right:\n"
            "    LiteralExpression (Integer): 2\n";
        CHECK(dumpAST(expr.get()) == expected);
    }

    SECTION("'a < b << c'") {
        Parser parser = makeParser("a < b << c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        std::string expected =
            "BinaryExpr: <\n"
            "  Left:\n"
            "    IdentifierExpression: a\n"
            "  Right:\n"
            "    BinaryExpr: <<\n"
            "      Left:\n"
            "        IdentifierExpression: b\n"
            "      Right:\n"
            "        IdentifierExpression: c\n";
        CHECK(dumpAST(expr.get()) == expected);
    }

    SECTION("'a & b == c'") {
        Parser parser = makeParser("a & b == c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        std::string expected =
            "BinaryExpr: ==\n"
            "  Left:\n"
            "    BinaryExpr: &\n"
            "      Left:\n"
            "        IdentifierExpression: a\n"
            "      Right:\n"
            "        IdentifierExpression: b\n"
            "  Right:\n"
            "    IdentifierExpression: c\n";
        CHECK(dumpAST(expr.get()) == expected);
    }

    SECTION("'a & b ^ c | d'") {
        Parser parser = makeParser("a & b ^ c | d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        std::string expected =
            "BinaryExpr: |\n"
            "  Left:\n"
            "    BinaryExpr: ^\n"
            "      Left:\n"
            "        BinaryExpr: &\n"
            "          Left:\n"
            "            IdentifierExpression: a\n"
            "          Right:\n"
            "            IdentifierExpression: b\n"
            "      Right:\n"
            "        IdentifierExpression: c\n"
            "  Right:\n"
            "    IdentifierExpression: d\n";
        CHECK(dumpAST(expr.get()) == expected);
    }
}

// -----------------------------------------------------------------------------
// 8. Error Handling for Malformed Bitwise and Shift Expressions
// -----------------------------------------------------------------------------
TEST_CASE("Bitwise and Shift Errors: missing operands", "[parser][bitwise][error]") {
    SECTION("missing right operand of '&': 'a &'") {
        Parser parser = makeParser("a &");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '&'");
    }

    SECTION("missing right operand of '|': 'a |'") {
        Parser parser = makeParser("a |");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '|'");
    }

    SECTION("missing right operand of '^': 'a ^'") {
        Parser parser = makeParser("a ^");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '^'");
    }

    SECTION("missing right operand of '<<': 'a <<'") {
        Parser parser = makeParser("a <<");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '<<'");
    }

    SECTION("missing right operand of '>>': 'a >>'") {
        Parser parser = makeParser("a >>");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '>>'");
    }

    SECTION("missing right operand before semicolon: 'a ^ ;'") {
        Parser parser = makeParser("a ^ ;");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '^'");
    }

    SECTION("missing right operand before paren: '(a & )'") {
        Parser parser = makeParser("(a & )");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '&'");
    }

    SECTION("consecutive operators without operand: 'a + & b'") {
        Parser parser = makeParser("a + & b");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '+'");
    }

    SECTION("missing left operand: '& a'") {
        Parser parser = makeParser("& a");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK_FALSE(parser.isAtEnd());
        CHECK(parser.peek().lexeme == "&");
    }

    SECTION("missing left operand: '<< a'") {
        Parser parser = makeParser("<< a");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK_FALSE(parser.isAtEnd());
        CHECK(parser.peek().lexeme == "<<");
    }
}

