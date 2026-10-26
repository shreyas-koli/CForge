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

// =============================================================================
// DAY 21 TESTS - Arithmetic Operators (+, -, *, /, %)
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Individual Arithmetic Operators
// -----------------------------------------------------------------------------
TEST_CASE("Arithmetic: addition operator '+' with various operand kinds", "[parser][arithmetic]") {
    SECTION("identifiers 'a + b'") {
        Parser parser = makeParser("a + b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");
        CHECK(bin->left()->toString() == "IdentifierExpression(a)");
        CHECK(bin->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("integer literals '10 + 20'") {
        Parser parser = makeParser("10 + 20");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");
        CHECK(bin->left()->toString() == "LiteralExpression(Integer, 10)");
        CHECK(bin->right()->toString() == "LiteralExpression(Integer, 20)");
    }

    SECTION("float literals '3.14 + 2.5'") {
        Parser parser = makeParser("3.14 + 2.5");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");
        CHECK(bin->left()->toString() == "LiteralExpression(Float, 3.14)");
        CHECK(bin->right()->toString() == "LiteralExpression(Float, 2.5)");
    }
}

TEST_CASE("Arithmetic: subtraction operator '-' 'x - y'", "[parser][arithmetic]") {
    Parser parser = makeParser("x - y");
    auto expr = parser.parseExpression();
    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());
    CHECK(parser.isAtEnd());

    auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(bin != nullptr);
    CHECK(bin->op() == "-");
    CHECK(bin->left()->toString() == "IdentifierExpression(x)");
    CHECK(bin->right()->toString() == "IdentifierExpression(y)");
}

TEST_CASE("Arithmetic: multiplication operator '*' 'x * y'", "[parser][arithmetic]") {
    Parser parser = makeParser("x * y");
    auto expr = parser.parseExpression();
    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());
    CHECK(parser.isAtEnd());

    auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(bin != nullptr);
    CHECK(bin->op() == "*");
    CHECK(bin->left()->toString() == "IdentifierExpression(x)");
    CHECK(bin->right()->toString() == "IdentifierExpression(y)");
}

TEST_CASE("Arithmetic: division operator '/' 'x / y'", "[parser][arithmetic]") {
    Parser parser = makeParser("x / y");
    auto expr = parser.parseExpression();
    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());
    CHECK(parser.isAtEnd());

    auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(bin != nullptr);
    CHECK(bin->op() == "/");
    CHECK(bin->left()->toString() == "IdentifierExpression(x)");
    CHECK(bin->right()->toString() == "IdentifierExpression(y)");
}

TEST_CASE("Arithmetic: modulo operator '%' 'x % y'", "[parser][arithmetic]") {
    Parser parser = makeParser("x % y");
    auto expr = parser.parseExpression();
    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());
    CHECK(parser.isAtEnd());

    auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(bin != nullptr);
    CHECK(bin->op() == "%");
    CHECK(bin->left()->toString() == "IdentifierExpression(x)");
    CHECK(bin->right()->toString() == "IdentifierExpression(y)");
}

// -----------------------------------------------------------------------------
// 2. Precedence Hierarchy
// -----------------------------------------------------------------------------
TEST_CASE("Arithmetic Precedence: '*' and '/' over '+' and '-'", "[parser][arithmetic][precedence]") {
    SECTION("a + b * c -> a + (b * c)") {
        Parser parser = makeParser("a + b * c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");
        CHECK(bin->left()->toString() == "IdentifierExpression(a)");

        auto* right = dynamic_cast<const BinaryExpr*>(bin->right());
        REQUIRE(right != nullptr);
        CHECK(right->op() == "*");
        CHECK(right->left()->toString() == "IdentifierExpression(b)");
        CHECK(right->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("a * b + c -> (a * b) + c") {
        Parser parser = makeParser("a * b + c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");

        auto* left = dynamic_cast<const BinaryExpr*>(bin->left());
        REQUIRE(left != nullptr);
        CHECK(left->op() == "*");
        CHECK(left->left()->toString() == "IdentifierExpression(a)");
        CHECK(left->right()->toString() == "IdentifierExpression(b)");

        CHECK(bin->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("a - b / c -> a - (b / c)") {
        Parser parser = makeParser("a - b / c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "-");

        auto* right = dynamic_cast<const BinaryExpr*>(bin->right());
        REQUIRE(right != nullptr);
        CHECK(right->op() == "/");
    }
}

TEST_CASE("Arithmetic Precedence: '%' has same precedence as '*' and '/' over '+' and '-'", "[parser][arithmetic][precedence]") {
    SECTION("a + b % c -> a + (b % c)") {
        Parser parser = makeParser("a + b % c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");
        CHECK(bin->left()->toString() == "IdentifierExpression(a)");

        auto* right = dynamic_cast<const BinaryExpr*>(bin->right());
        REQUIRE(right != nullptr);
        CHECK(right->op() == "%");
        CHECK(right->left()->toString() == "IdentifierExpression(b)");
        CHECK(right->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("a % b + c -> (a % b) + c") {
        Parser parser = makeParser("a % b + c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");

        auto* left = dynamic_cast<const BinaryExpr*>(bin->left());
        REQUIRE(left != nullptr);
        CHECK(left->op() == "%");
        CHECK(left->left()->toString() == "IdentifierExpression(a)");
        CHECK(left->right()->toString() == "IdentifierExpression(b)");

        CHECK(bin->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("a - b % c -> a - (b % c)") {
        Parser parser = makeParser("a - b % c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "-");

        auto* right = dynamic_cast<const BinaryExpr*>(bin->right());
        REQUIRE(right != nullptr);
        CHECK(right->op() == "%");
    }

    SECTION("a % b - c -> (a % b) - c") {
        Parser parser = makeParser("a % b - c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "-");

        auto* left = dynamic_cast<const BinaryExpr*>(bin->left());
        REQUIRE(left != nullptr);
        CHECK(left->op() == "%");
    }
}

// -----------------------------------------------------------------------------
// 3. Left-Associativity
// -----------------------------------------------------------------------------
TEST_CASE("Arithmetic Left-Associativity: operators at the same precedence level group left-to-right", "[parser][arithmetic][associativity]") {
    SECTION("subtraction: a - b - c -> (a - b) - c") {
        Parser parser = makeParser("a - b - c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "-");
        CHECK(root->right()->toString() == "IdentifierExpression(c)");

        auto* left = dynamic_cast<const BinaryExpr*>(root->left());
        REQUIRE(left != nullptr);
        CHECK(left->op() == "-");
        CHECK(left->left()->toString() == "IdentifierExpression(a)");
        CHECK(left->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("modulo: a % b % c -> (a % b) % c") {
        Parser parser = makeParser("a % b % c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "%");
        CHECK(root->right()->toString() == "IdentifierExpression(c)");

        auto* left = dynamic_cast<const BinaryExpr*>(root->left());
        REQUIRE(left != nullptr);
        CHECK(left->op() == "%");
        CHECK(left->left()->toString() == "IdentifierExpression(a)");
        CHECK(left->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("division and modulo: a / b % c -> (a / b) % c") {
        Parser parser = makeParser("a / b % c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "%");
        CHECK(root->right()->toString() == "IdentifierExpression(c)");

        auto* left = dynamic_cast<const BinaryExpr*>(root->left());
        REQUIRE(left != nullptr);
        CHECK(left->op() == "/");
        CHECK(left->left()->toString() == "IdentifierExpression(a)");
        CHECK(left->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("modulo and multiplication: a % b * c -> (a % b) * c") {
        Parser parser = makeParser("a % b * c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "*");
        CHECK(root->right()->toString() == "IdentifierExpression(c)");

        auto* left = dynamic_cast<const BinaryExpr*>(root->left());
        REQUIRE(left != nullptr);
        CHECK(left->op() == "%");
        CHECK(left->left()->toString() == "IdentifierExpression(a)");
        CHECK(left->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("multi-operator chain: a + b % c * d -> a + ((b % c) * d)") {
        Parser parser = makeParser("a + b % c * d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "+");
        CHECK(root->left()->toString() == "IdentifierExpression(a)");

        auto* mult = dynamic_cast<const BinaryExpr*>(root->right());
        REQUIRE(mult != nullptr);
        CHECK(mult->op() == "*");
        CHECK(mult->right()->toString() == "IdentifierExpression(d)");

        auto* mod = dynamic_cast<const BinaryExpr*>(mult->left());
        REQUIRE(mod != nullptr);
        CHECK(mod->op() == "%");
        CHECK(mod->left()->toString() == "IdentifierExpression(b)");
        CHECK(mod->right()->toString() == "IdentifierExpression(c)");
    }
}

// -----------------------------------------------------------------------------
// 4. Parentheses Overriding Precedence
// -----------------------------------------------------------------------------
TEST_CASE("Arithmetic Parentheses: parentheses override standard precedence", "[parser][arithmetic][paren]") {
    SECTION("a * (b + c)") {
        Parser parser = makeParser("a * (b + c)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "*");
        CHECK(root->left()->toString() == "IdentifierExpression(a)");

        auto* right = dynamic_cast<const BinaryExpr*>(root->right());
        REQUIRE(right != nullptr);
        CHECK(right->op() == "+");
        CHECK(right->left()->toString() == "IdentifierExpression(b)");
        CHECK(right->right()->toString() == "IdentifierExpression(c)");
    }

    SECTION("(a + b) % c") {
        Parser parser = makeParser("(a + b) % c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "%");
        CHECK(root->right()->toString() == "IdentifierExpression(c)");

        auto* left = dynamic_cast<const BinaryExpr*>(root->left());
        REQUIRE(left != nullptr);
        CHECK(left->op() == "+");
    }

    SECTION("a % (b + c)") {
        Parser parser = makeParser("a % (b + c)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "%");
        CHECK(root->left()->toString() == "IdentifierExpression(a)");

        auto* right = dynamic_cast<const BinaryExpr*>(root->right());
        REQUIRE(right != nullptr);
        CHECK(right->op() == "+");
    }

    SECTION("nested parentheses: ((a + b) * c) % d") {
        Parser parser = makeParser("((a + b) * c) % d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "%");
        CHECK(root->right()->toString() == "IdentifierExpression(d)");

        auto* mult = dynamic_cast<const BinaryExpr*>(root->left());
        REQUIRE(mult != nullptr);
        CHECK(mult->op() == "*");
        CHECK(mult->right()->toString() == "IdentifierExpression(c)");

        auto* sum = dynamic_cast<const BinaryExpr*>(mult->left());
        REQUIRE(sum != nullptr);
        CHECK(sum->op() == "+");
    }
}

// -----------------------------------------------------------------------------
// 5. Arithmetic Combined with Prefix Unary Operators
// -----------------------------------------------------------------------------
TEST_CASE("Arithmetic Unary Interaction: unary operators bind more tightly than binary", "[parser][arithmetic][unary]") {
    SECTION("-a * b -> (-a) * b") {
        Parser parser = makeParser("-a * b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "*");
        CHECK(root->right()->toString() == "IdentifierExpression(b)");

        auto* un = dynamic_cast<const UnaryExpr*>(root->left());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "-");
        CHECK(un->operand()->toString() == "IdentifierExpression(a)");
    }

    SECTION("-a % b -> (-a) % b") {
        Parser parser = makeParser("-a % b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "%");
        CHECK(root->right()->toString() == "IdentifierExpression(b)");

        auto* un = dynamic_cast<const UnaryExpr*>(root->left());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "-");
        CHECK(un->operand()->toString() == "IdentifierExpression(a)");
    }

    SECTION("a % -b -> a % (-b)") {
        Parser parser = makeParser("a % -b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "%");
        CHECK(root->left()->toString() == "IdentifierExpression(a)");

        auto* un = dynamic_cast<const UnaryExpr*>(root->right());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "-");
        CHECK(un->operand()->toString() == "IdentifierExpression(b)");
    }

    SECTION("-(a % b) -> -((a % b))") {
        Parser parser = makeParser("-(a % b)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        auto* un = dynamic_cast<UnaryExpr*>(expr.get());
        REQUIRE(un != nullptr);
        CHECK(un->op() == "-");

        auto* mod = dynamic_cast<const BinaryExpr*>(un->operand());
        REQUIRE(mod != nullptr);
        CHECK(mod->op() == "%");
    }
}

// -----------------------------------------------------------------------------
// 6. Interaction with Assignment Expressions
// -----------------------------------------------------------------------------
TEST_CASE("Arithmetic Assignment Interaction: assignment has lower precedence than arithmetic", "[parser][arithmetic][assign]") {
    SECTION("x = a + b % c -> x = (a + (b % c))") {
        Parser parser = makeParser("x = a + b % c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "=");
        CHECK(assign->left()->toString() == "IdentifierExpression(x)");

        auto* sum = dynamic_cast<const BinaryExpr*>(assign->right());
        REQUIRE(sum != nullptr);
        CHECK(sum->op() == "+");
        CHECK(sum->left()->toString() == "IdentifierExpression(a)");

        auto* mod = dynamic_cast<const BinaryExpr*>(sum->right());
        REQUIRE(mod != nullptr);
        CHECK(mod->op() == "%");
    }

    SECTION("x %= a + b") {
        // %= is not yet in assignment operators, but x = a % b is
        Parser parser = makeParser("x = a % b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "=");

        auto* mod = dynamic_cast<const BinaryExpr*>(assign->right());
        REQUIRE(mod != nullptr);
        CHECK(mod->op() == "%");
    }
}

// -----------------------------------------------------------------------------
// 7. Error Handling for Malformed Arithmetic Expressions
// -----------------------------------------------------------------------------
TEST_CASE("Arithmetic Error: missing right operands", "[parser][arithmetic][error]") {
    SECTION("missing right operand of '%': 'a %'") {
        Parser parser = makeParser("a %");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '%'");
    }

    SECTION("missing right operand of '+': 'a + ;'") {
        Parser parser = makeParser("a + ;");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '+'");
    }

    SECTION("missing right operand of '*': 'a *'") {
        Parser parser = makeParser("a *");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '*'");
    }

    SECTION("missing right operand before delimiter: 'a % )'") {
        Parser parser = makeParser("a % )");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '%'");
    }

    SECTION("consecutive operators without operand: 'a + % b'") {
        Parser parser = makeParser("a + % b");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '+'");
    }

    SECTION("missing left operand: '% a'") {
        Parser parser = makeParser("% a");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        // % is not a prefix operator or primary, returns nullptr cleanly without consuming tokens
        CHECK_FALSE(parser.isAtEnd());
        CHECK(parser.peek().lexeme == "%");
    }
}

