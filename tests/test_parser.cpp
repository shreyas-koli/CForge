#include <catch2/catch.hpp>

#include "lexer/lexer.h"
#include "parser/parser.h"

// Helper: create a parser for a source code string
static Parser makeParser(const std::string& src) {
    Lexer lexer(src);
    return Parser(lexer.tokenize());
}

// -----------------------------------------------------------------------------
// 1. Integer Literal Parsing
// -----------------------------------------------------------------------------
TEST_CASE("parsePrimary: integer literal '10'", "[parser][primary][integer]") {
    Parser parser = makeParser("10");
    auto expr = parser.parsePrimary();

    REQUIRE(expr != nullptr);
    CHECK(expr->location().line == 1);
    CHECK(expr->location().column == 1);

    auto* lit = dynamic_cast<LiteralExpression*>(expr.get());
    REQUIRE(lit != nullptr);
    CHECK(lit->kind() == LiteralKind::Integer);
    CHECK(lit->value() == "10");
    CHECK(lit->toString() == "LiteralExpression(Integer, 10)");
}

TEST_CASE("parsePrimary: integer hex literal '0xFF'", "[parser][primary][integer]") {
    Parser parser = makeParser("0xFF");
    auto expr = parser.parsePrimary();

    REQUIRE(expr != nullptr);
    auto* lit = dynamic_cast<LiteralExpression*>(expr.get());
    REQUIRE(lit != nullptr);
    CHECK(lit->kind() == LiteralKind::Integer);
    CHECK(lit->value() == "0xFF");
}

// -----------------------------------------------------------------------------
// 2. Float Literal Parsing
// -----------------------------------------------------------------------------
TEST_CASE("parsePrimary: float literal '3.14'", "[parser][primary][float]") {
    Parser parser = makeParser("3.14");
    auto expr = parser.parsePrimary();

    REQUIRE(expr != nullptr);
    CHECK(expr->location().line == 1);
    CHECK(expr->location().column == 1);

    auto* lit = dynamic_cast<LiteralExpression*>(expr.get());
    REQUIRE(lit != nullptr);
    CHECK(lit->kind() == LiteralKind::Float);
    CHECK(lit->value() == "3.14");
    CHECK(lit->toString() == "LiteralExpression(Float, 3.14)");
}

TEST_CASE("parsePrimary: float exponent literal '2.5e-3'", "[parser][primary][float]") {
    Parser parser = makeParser("2.5e-3");
    auto expr = parser.parsePrimary();

    REQUIRE(expr != nullptr);
    auto* lit = dynamic_cast<LiteralExpression*>(expr.get());
    REQUIRE(lit != nullptr);
    CHECK(lit->kind() == LiteralKind::Float);
    CHECK(lit->value() == "2.5e-3");
}

// -----------------------------------------------------------------------------
// 3. Char Literal Parsing
// -----------------------------------------------------------------------------
TEST_CASE("parsePrimary: char literal ''x''", "[parser][primary][char]") {
    Parser parser = makeParser("'x'");
    auto expr = parser.parsePrimary();

    REQUIRE(expr != nullptr);
    CHECK(expr->location().line == 1);
    CHECK(expr->location().column == 1);

    auto* lit = dynamic_cast<LiteralExpression*>(expr.get());
    REQUIRE(lit != nullptr);
    CHECK(lit->kind() == LiteralKind::Char);
    CHECK(lit->value() == "'x'");
    CHECK(lit->toString() == "LiteralExpression(Char, 'x')");
}

TEST_CASE("parsePrimary: char escape literal ''\\n''", "[parser][primary][char]") {
    Parser parser = makeParser("'\\n'");
    auto expr = parser.parsePrimary();

    REQUIRE(expr != nullptr);
    auto* lit = dynamic_cast<LiteralExpression*>(expr.get());
    REQUIRE(lit != nullptr);
    CHECK(lit->kind() == LiteralKind::Char);
    CHECK(lit->value() == "'\\n'");
}

// -----------------------------------------------------------------------------
// 4. String Literal Parsing
// -----------------------------------------------------------------------------
TEST_CASE("parsePrimary: string literal '\"hello\"'", "[parser][primary][string]") {
    Parser parser = makeParser("\"hello\"");
    auto expr = parser.parsePrimary();

    REQUIRE(expr != nullptr);
    CHECK(expr->location().line == 1);
    CHECK(expr->location().column == 1);

    auto* lit = dynamic_cast<LiteralExpression*>(expr.get());
    REQUIRE(lit != nullptr);
    CHECK(lit->kind() == LiteralKind::String);
    CHECK(lit->value() == "\"hello\"");
    CHECK(lit->toString() == "LiteralExpression(String, \"hello\")");
}

TEST_CASE("parsePrimary: string escape literal '\"hello\\nworld\"'", "[parser][primary][string]") {
    Parser parser = makeParser("\"hello\\nworld\"");
    auto expr = parser.parsePrimary();

    REQUIRE(expr != nullptr);
    auto* lit = dynamic_cast<LiteralExpression*>(expr.get());
    REQUIRE(lit != nullptr);
    CHECK(lit->kind() == LiteralKind::String);
    CHECK(lit->value() == "\"hello\\nworld\"");
}

// -----------------------------------------------------------------------------
// 5. Bare Identifier Parsing
// -----------------------------------------------------------------------------
TEST_CASE("parsePrimary: identifier 'x'", "[parser][primary][identifier]") {
    Parser parser = makeParser("x");
    auto expr = parser.parsePrimary();

    REQUIRE(expr != nullptr);
    CHECK(expr->location().line == 1);
    CHECK(expr->location().column == 1);

    auto* id = dynamic_cast<IdentifierExpression*>(expr.get());
    REQUIRE(id != nullptr);
    CHECK(id->name() == "x");
    CHECK(id->toString() == "IdentifierExpression(x)");
}

TEST_CASE("parsePrimary: identifier 'myFunction_123'", "[parser][primary][identifier]") {
    Parser parser = makeParser("myFunction_123");
    auto expr = parser.parsePrimary();

    REQUIRE(expr != nullptr);
    auto* id = dynamic_cast<IdentifierExpression*>(expr.get());
    REQUIRE(id != nullptr);
    CHECK(id->name() == "myFunction_123");
}

// -----------------------------------------------------------------------------
// 6. Non-Primary / Invalid Tokens Fail Cleanly Without Consuming
// -----------------------------------------------------------------------------
TEST_CASE("parsePrimary: operator '+' is not a primary expression", "[parser][primary][error]") {
    Parser parser = makeParser("+ 42");
    auto expr = parser.parsePrimary();

    CHECK(expr == nullptr);
    // Verify '+' was not consumed: current token is still '+'
    CHECK(parser.peek().lexeme == "+");
    CHECK(parser.peek().type == TokenType::Operator);
}

TEST_CASE("parsePrimary: punctuation ';' is not a primary expression", "[parser][primary][error]") {
    Parser parser = makeParser(";");
    auto expr = parser.parsePrimary();

    CHECK(expr == nullptr);
    CHECK(parser.peek().lexeme == ";");
    CHECK(parser.peek().type == TokenType::Punctuation);
}

TEST_CASE("parsePrimary: keyword 'int' is not a primary expression", "[parser][primary][error]") {
    Parser parser = makeParser("int x");
    auto expr = parser.parsePrimary();

    CHECK(expr == nullptr);
    CHECK(parser.peek().lexeme == "int");
    CHECK(parser.peek().type == TokenType::Keyword);
}

TEST_CASE("parsePrimary: EOF returns nullptr cleanly", "[parser][primary][error]") {
    Parser parser = makeParser("");
    auto expr = parser.parsePrimary();

    CHECK(expr == nullptr);
    CHECK(parser.isAtEnd());
}

// -----------------------------------------------------------------------------
// 7. Source Location Tracking
// -----------------------------------------------------------------------------
TEST_CASE("parsePrimary: source location with leading whitespace", "[parser][primary][location]") {
    // "   42" -> 3 spaces then '42' at col 4
    Parser parser = makeParser("   42");
    auto expr = parser.parsePrimary();

    REQUIRE(expr != nullptr);
    CHECK(expr->location().line == 1);
    CHECK(expr->location().column == 4);
}

TEST_CASE("parsePrimary: source location on line 2", "[parser][primary][location]") {
    Parser parser = makeParser("\n  varName");
    auto expr = parser.parsePrimary();

    REQUIRE(expr != nullptr);
    CHECK(expr->location().line == 2);
    CHECK(expr->location().column == 3);
}

// -----------------------------------------------------------------------------
// 8. Sequential parsePrimary Calls
// -----------------------------------------------------------------------------
TEST_CASE("parsePrimary: sequential calls consume tokens correctly", "[parser][primary][sequence]") {
    Parser parser = makeParser("10 3.14 'a' \"str\" count");

    auto e1 = parser.parsePrimary();
    REQUIRE(e1 != nullptr);
    auto* l1 = dynamic_cast<LiteralExpression*>(e1.get());
    REQUIRE(l1 != nullptr);
    CHECK(l1->kind() == LiteralKind::Integer);
    CHECK(l1->value() == "10");

    auto e2 = parser.parsePrimary();
    REQUIRE(e2 != nullptr);
    auto* l2 = dynamic_cast<LiteralExpression*>(e2.get());
    REQUIRE(l2 != nullptr);
    CHECK(l2->kind() == LiteralKind::Float);
    CHECK(l2->value() == "3.14");

    auto e3 = parser.parsePrimary();
    REQUIRE(e3 != nullptr);
    auto* l3 = dynamic_cast<LiteralExpression*>(e3.get());
    REQUIRE(l3 != nullptr);
    CHECK(l3->kind() == LiteralKind::Char);
    CHECK(l3->value() == "'a'");

    auto e4 = parser.parsePrimary();
    REQUIRE(e4 != nullptr);
    auto* l4 = dynamic_cast<LiteralExpression*>(e4.get());
    REQUIRE(l4 != nullptr);
    CHECK(l4->kind() == LiteralKind::String);
    CHECK(l4->value() == "\"str\"");

    auto e5 = parser.parsePrimary();
    REQUIRE(e5 != nullptr);
    auto* id = dynamic_cast<IdentifierExpression*>(e5.get());
    REQUIRE(id != nullptr);
    CHECK(id->name() == "count");

    // Next call at EOF returns nullptr
    auto e6 = parser.parsePrimary();
    CHECK(e6 == nullptr);
    CHECK(parser.isAtEnd());
}

// =============================================================================
// DAY 13 TESTS — Parenthesized Expressions
// =============================================================================

// -----------------------------------------------------------------------------
// 9. Parenthesized Primary Expressions
// -----------------------------------------------------------------------------
TEST_CASE("parsePrimary: parenthesized identifier '(x)'", "[parser][paren][identifier]") {
    Parser parser = makeParser("(x)");
    auto expr = parser.parsePrimary();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* id = dynamic_cast<IdentifierExpression*>(expr.get());
    REQUIRE(id != nullptr);
    CHECK(id->name() == "x");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parsePrimary: parenthesized integer literal '(10)'", "[parser][paren][integer]") {
    Parser parser = makeParser("(10)");
    auto expr = parser.parsePrimary();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* lit = dynamic_cast<LiteralExpression*>(expr.get());
    REQUIRE(lit != nullptr);
    CHECK(lit->kind() == LiteralKind::Integer);
    CHECK(lit->value() == "10");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parsePrimary: parenthesized float literal '(3.14)'", "[parser][paren][float]") {
    Parser parser = makeParser("(3.14)");
    auto expr = parser.parsePrimary();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* lit = dynamic_cast<LiteralExpression*>(expr.get());
    REQUIRE(lit != nullptr);
    CHECK(lit->kind() == LiteralKind::Float);
    CHECK(lit->value() == "3.14");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parsePrimary: parenthesized char and string literals", "[parser][paren][literals]") {
    {
        Parser parser = makeParser("('a')");
        auto expr = parser.parsePrimary();
        REQUIRE(expr != nullptr);
        auto* lit = dynamic_cast<LiteralExpression*>(expr.get());
        REQUIRE(lit != nullptr);
        CHECK(lit->kind() == LiteralKind::Char);
        CHECK(lit->value() == "'a'");
    }
    {
        Parser parser = makeParser("(\"hello\")");
        auto expr = parser.parsePrimary();
        REQUIRE(expr != nullptr);
        auto* lit = dynamic_cast<LiteralExpression*>(expr.get());
        REQUIRE(lit != nullptr);
        CHECK(lit->kind() == LiteralKind::String);
        CHECK(lit->value() == "\"hello\"");
    }
}

TEST_CASE("parsePrimary: nested parentheses '((x))' and '(((42)))'", "[parser][paren][nested]") {
    {
        Parser parser = makeParser("((x))");
        auto expr = parser.parsePrimary();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* id = dynamic_cast<IdentifierExpression*>(expr.get());
        REQUIRE(id != nullptr);
        CHECK(id->name() == "x");
        CHECK(parser.isAtEnd());
    }
    {
        Parser parser = makeParser("(((42)))");
        auto expr = parser.parsePrimary();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* lit = dynamic_cast<LiteralExpression*>(expr.get());
        REQUIRE(lit != nullptr);
        CHECK(lit->kind() == LiteralKind::Integer);
        CHECK(lit->value() == "42");
        CHECK(parser.isAtEnd());
    }
}

TEST_CASE("parseExpression: top-level parenthesized expression delegator", "[parser][paren][expression]") {
    Parser parser = makeParser("(foo)");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* id = dynamic_cast<IdentifierExpression*>(expr.get());
    REQUIRE(id != nullptr);
    CHECK(id->name() == "foo");
    CHECK(parser.isAtEnd());
}

// -----------------------------------------------------------------------------
// 10. Parenthesized Expression Limitations & Error Handling
// -----------------------------------------------------------------------------
TEST_CASE("parsePrimary: '(a + b)' parses binary expression in parentheses", "[parser][paren][binary]") {
    Parser parser = makeParser("(a + b)");
    auto expr = parser.parsePrimary();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(bin != nullptr);
    CHECK(bin->op() == "+");

    auto* left = dynamic_cast<const IdentifierExpression*>(bin->left());
    REQUIRE(left != nullptr);
    CHECK(left->name() == "a");

    auto* right = dynamic_cast<const IdentifierExpression*>(bin->right());
    REQUIRE(right != nullptr);
    CHECK(right->name() == "b");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parsePrimary: unclosed parenthesis '(x' reports missing ')'", "[parser][paren][error]") {
    Parser parser = makeParser("(x");
    auto expr = parser.parsePrimary();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected ')' after expression");
}

TEST_CASE("parsePrimary: unclosed nested parenthesis '((x)' reports missing ')'", "[parser][paren][error]") {
    Parser parser = makeParser("((x)");
    auto expr = parser.parsePrimary();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected ')' after expression");
}

TEST_CASE("parsePrimary: unexpected closing parenthesis 'x)' leaves ')' for caller", "[parser][paren][error]") {
    Parser parser = makeParser("x)");
    auto expr = parser.parsePrimary();

    // 'x' is parsed successfully as an identifier
    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());
    auto* id = dynamic_cast<IdentifierExpression*>(expr.get());
    REQUIRE(id != nullptr);
    CHECK(id->name() == "x");

    // The trailing ')' is left unconsumed in the token stream
    CHECK_FALSE(parser.isAtEnd());
    CHECK(parser.peek().lexeme == ")");
    CHECK(parser.peek().type == TokenType::Punctuation);

    // Subsequent parsePrimary() call on ')' fails cleanly without consuming ')'
    auto expr2 = parser.parsePrimary();
    CHECK(expr2 == nullptr);
    CHECK(parser.peek().lexeme == ")");
}

TEST_CASE("parsePrimary: empty parentheses '()' fails cleanly", "[parser][paren][error]") {
    Parser parser = makeParser("()");
    auto expr = parser.parsePrimary();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Empty parenthesized expression");
    // Verifies '()' was consumed cleanly without consuming further tokens
    CHECK(parser.isAtEnd());
}

TEST_CASE("parsePrimary: invalid enclosed syntax '(*' fails cleanly", "[parser][paren][error]") {
    Parser parser = makeParser("(*");
    auto expr = parser.parsePrimary();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '('");
}

// =============================================================================
// DAY 14 TESTS — Binary Expressions
// =============================================================================

// -----------------------------------------------------------------------------
// 11. Basic Binary Expressions (+, -, *, /)
// -----------------------------------------------------------------------------
TEST_CASE("parseExpression: binary addition 'a + b'", "[parser][binary]") {
    Parser parser = makeParser("a + b");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(bin != nullptr);
    CHECK(bin->op() == "+");
    CHECK(bin->getOp() == "+");

    auto* left = dynamic_cast<const IdentifierExpression*>(bin->left());
    REQUIRE(left != nullptr);
    CHECK(left->name() == "a");

    auto* right = dynamic_cast<const IdentifierExpression*>(bin->right());
    REQUIRE(right != nullptr);
    CHECK(right->name() == "b");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: binary subtraction 'a - b'", "[parser][binary]") {
    Parser parser = makeParser("a - b");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(bin != nullptr);
    CHECK(bin->op() == "-");

    auto* left = dynamic_cast<const IdentifierExpression*>(bin->left());
    REQUIRE(left != nullptr);
    CHECK(left->name() == "a");

    auto* right = dynamic_cast<const IdentifierExpression*>(bin->right());
    REQUIRE(right != nullptr);
    CHECK(right->name() == "b");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: binary multiplication 'a * b'", "[parser][binary]") {
    Parser parser = makeParser("a * b");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(bin != nullptr);
    CHECK(bin->op() == "*");

    auto* left = dynamic_cast<const IdentifierExpression*>(bin->left());
    REQUIRE(left != nullptr);
    CHECK(left->name() == "a");

    auto* right = dynamic_cast<const IdentifierExpression*>(bin->right());
    REQUIRE(right != nullptr);
    CHECK(right->name() == "b");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: binary division 'a / b'", "[parser][binary]") {
    Parser parser = makeParser("a / b");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(bin != nullptr);
    CHECK(bin->op() == "/");

    auto* left = dynamic_cast<const IdentifierExpression*>(bin->left());
    REQUIRE(left != nullptr);
    CHECK(left->name() == "a");

    auto* right = dynamic_cast<const IdentifierExpression*>(bin->right());
    REQUIRE(right != nullptr);
    CHECK(right->name() == "b");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: binary expression with numeric literals '10 + 20'", "[parser][binary]") {
    Parser parser = makeParser("10 + 20");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(bin != nullptr);
    CHECK(bin->op() == "+");

    auto* left = dynamic_cast<const LiteralExpression*>(bin->left());
    REQUIRE(left != nullptr);
    CHECK(left->kind() == LiteralKind::Integer);
    CHECK(left->value() == "10");

    auto* right = dynamic_cast<const LiteralExpression*>(bin->right());
    REQUIRE(right != nullptr);
    CHECK(right->kind() == LiteralKind::Integer);
    CHECK(right->value() == "20");

    CHECK(parser.isAtEnd());
}

// -----------------------------------------------------------------------------
// 12. Operator Precedence and Left-Associativity (Day 15)
// -----------------------------------------------------------------------------
TEST_CASE("parseExpression: left-associativity 'a + b + c'", "[parser][binary][associativity]") {
    Parser parser = makeParser("a + b + c");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(+)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "+");

    // Left child is BinaryExpr(+) for "a + b"
    auto* leftBin = dynamic_cast<const BinaryExpr*>(root->left());
    REQUIRE(leftBin != nullptr);
    CHECK(leftBin->op() == "+");

    auto* leftA = dynamic_cast<const IdentifierExpression*>(leftBin->left());
    REQUIRE(leftA != nullptr);
    CHECK(leftA->name() == "a");

    auto* rightB = dynamic_cast<const IdentifierExpression*>(leftBin->right());
    REQUIRE(rightB != nullptr);
    CHECK(rightB->name() == "b");

    // Right child is Identifier(c)
    auto* rightC = dynamic_cast<const IdentifierExpression*>(root->right());
    REQUIRE(rightC != nullptr);
    CHECK(rightC->name() == "c");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: operator precedence 'a + b * c' -> a + (b * c)", "[parser][binary][precedence]") {
    // Under Day 15 precedence climbing, '*' binds tighter than '+': "a + (b * c)"
    Parser parser = makeParser("a + b * c");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(+)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "+");

    // Left child is Identifier(a)
    auto* leftA = dynamic_cast<const IdentifierExpression*>(root->left());
    REQUIRE(leftA != nullptr);
    CHECK(leftA->name() == "a");

    // Right child is BinaryExpr(*) for "b * c"
    auto* rightBin = dynamic_cast<const BinaryExpr*>(root->right());
    REQUIRE(rightBin != nullptr);
    CHECK(rightBin->op() == "*");

    auto* leftB = dynamic_cast<const IdentifierExpression*>(rightBin->left());
    REQUIRE(leftB != nullptr);
    CHECK(leftB->name() == "b");

    auto* rightC = dynamic_cast<const IdentifierExpression*>(rightBin->right());
    REQUIRE(rightC != nullptr);
    CHECK(rightC->name() == "c");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: operator precedence 'a * b + c' -> (a * b) + c", "[parser][binary][precedence]") {
    Parser parser = makeParser("a * b + c");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(+)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "+");

    // Left child is BinaryExpr(*) for "a * b"
    auto* leftBin = dynamic_cast<const BinaryExpr*>(root->left());
    REQUIRE(leftBin != nullptr);
    CHECK(leftBin->op() == "*");

    auto* leftA = dynamic_cast<const IdentifierExpression*>(leftBin->left());
    REQUIRE(leftA != nullptr);
    CHECK(leftA->name() == "a");

    auto* rightB = dynamic_cast<const IdentifierExpression*>(leftBin->right());
    REQUIRE(rightB != nullptr);
    CHECK(rightB->name() == "b");

    // Right child is Identifier(c)
    auto* rightC = dynamic_cast<const IdentifierExpression*>(root->right());
    REQUIRE(rightC != nullptr);
    CHECK(rightC->name() == "c");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: left-associativity 'a - b - c' -> (a - b) - c", "[parser][binary][associativity]") {
    Parser parser = makeParser("a - b - c");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(-)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "-");

    // Left child is BinaryExpr(-) for "a - b"
    auto* leftBin = dynamic_cast<const BinaryExpr*>(root->left());
    REQUIRE(leftBin != nullptr);
    CHECK(leftBin->op() == "-");

    auto* leftA = dynamic_cast<const IdentifierExpression*>(leftBin->left());
    REQUIRE(leftA != nullptr);
    CHECK(leftA->name() == "a");

    auto* rightB = dynamic_cast<const IdentifierExpression*>(leftBin->right());
    REQUIRE(rightB != nullptr);
    CHECK(rightB->name() == "b");

    // Right child is Identifier(c)
    auto* rightC = dynamic_cast<const IdentifierExpression*>(root->right());
    REQUIRE(rightC != nullptr);
    CHECK(rightC->name() == "c");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: mixed precedence and associativity 'a + b / c * d' -> a + ((b / c) * d)", "[parser][binary][precedence]") {
    // '/' and '*' have same precedence (2) and are left-associative: "((b / c) * d)"
    // '+' has lower precedence (1): "a + ((b / c) * d)"
    Parser parser = makeParser("a + b / c * d");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(+)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "+");

    // Left child is Identifier(a)
    auto* leftA = dynamic_cast<const IdentifierExpression*>(root->left());
    REQUIRE(leftA != nullptr);
    CHECK(leftA->name() == "a");

    // Right child is BinaryExpr(*) for "(b / c) * d"
    auto* rightMul = dynamic_cast<const BinaryExpr*>(root->right());
    REQUIRE(rightMul != nullptr);
    CHECK(rightMul->op() == "*");

    // Left child of '*' is BinaryExpr(/) for "b / c"
    auto* leftDiv = dynamic_cast<const BinaryExpr*>(rightMul->left());
    REQUIRE(leftDiv != nullptr);
    CHECK(leftDiv->op() == "/");

    auto* leftB = dynamic_cast<const IdentifierExpression*>(leftDiv->left());
    REQUIRE(leftB != nullptr);
    CHECK(leftB->name() == "b");

    auto* rightC = dynamic_cast<const IdentifierExpression*>(leftDiv->right());
    REQUIRE(rightC != nullptr);
    CHECK(rightC->name() == "c");

    // Right child of '*' is Identifier(d)
    auto* rightD = dynamic_cast<const IdentifierExpression*>(rightMul->right());
    REQUIRE(rightD != nullptr);
    CHECK(rightD->name() == "d");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: mixed operators 'a * b - c / d' -> (a * b) - (c / d)", "[parser][binary][precedence]") {
    Parser parser = makeParser("a * b - c / d");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(-)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "-");

    // Left child is BinaryExpr(*) for "a * b"
    auto* leftMul = dynamic_cast<const BinaryExpr*>(root->left());
    REQUIRE(leftMul != nullptr);
    CHECK(leftMul->op() == "*");

    auto* leftA = dynamic_cast<const IdentifierExpression*>(leftMul->left());
    REQUIRE(leftA != nullptr);
    CHECK(leftA->name() == "a");

    auto* rightB = dynamic_cast<const IdentifierExpression*>(leftMul->right());
    REQUIRE(rightB != nullptr);
    CHECK(rightB->name() == "b");

    // Right child is BinaryExpr(/) for "c / d"
    auto* rightDiv = dynamic_cast<const BinaryExpr*>(root->right());
    REQUIRE(rightDiv != nullptr);
    CHECK(rightDiv->op() == "/");

    auto* leftC = dynamic_cast<const IdentifierExpression*>(rightDiv->left());
    REQUIRE(leftC != nullptr);
    CHECK(leftC->name() == "c");

    auto* rightD = dynamic_cast<const IdentifierExpression*>(rightDiv->right());
    REQUIRE(rightD != nullptr);
    CHECK(rightD->name() == "d");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: left-associativity of division 'a / b / c' -> (a / b) / c", "[parser][binary][associativity]") {
    Parser parser = makeParser("a / b / c");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(/)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "/");

    // Left child is BinaryExpr(/) for "a / b"
    auto* leftDiv = dynamic_cast<const BinaryExpr*>(root->left());
    REQUIRE(leftDiv != nullptr);
    CHECK(leftDiv->op() == "/");

    auto* leftA = dynamic_cast<const IdentifierExpression*>(leftDiv->left());
    REQUIRE(leftA != nullptr);
    CHECK(leftA->name() == "a");

    auto* rightB = dynamic_cast<const IdentifierExpression*>(leftDiv->right());
    REQUIRE(rightB != nullptr);
    CHECK(rightB->name() == "b");

    // Right child is Identifier(c)
    auto* rightC = dynamic_cast<const IdentifierExpression*>(root->right());
    REQUIRE(rightC != nullptr);
    CHECK(rightC->name() == "c");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: numeric literals with precedence '1 + 2 * 3' -> 1 + (2 * 3)", "[parser][binary][precedence]") {
    Parser parser = makeParser("1 + 2 * 3");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(+)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "+");

    // Left child is LiteralExpression(1)
    auto* leftLit = dynamic_cast<const LiteralExpression*>(root->left());
    REQUIRE(leftLit != nullptr);
    CHECK(leftLit->value() == "1");

    // Right child is BinaryExpr(*) for "2 * 3"
    auto* rightMul = dynamic_cast<const BinaryExpr*>(root->right());
    REQUIRE(rightMul != nullptr);
    CHECK(rightMul->op() == "*");

    auto* left2 = dynamic_cast<const LiteralExpression*>(rightMul->left());
    REQUIRE(left2 != nullptr);
    CHECK(left2->value() == "2");

    auto* right3 = dynamic_cast<const LiteralExpression*>(rightMul->right());
    REQUIRE(right3 != nullptr);
    CHECK(right3->value() == "3");

    CHECK(parser.isAtEnd());
}

// -----------------------------------------------------------------------------
// 13. Parentheses Interaction with Binary Expressions
// -----------------------------------------------------------------------------
TEST_CASE("parseExpression: '(a + b) * c' respects parentheses", "[parser][binary][paren]") {
    Parser parser = makeParser("(a + b) * c");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(*)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "*");

    // Left child is BinaryExpr(+) from inside parens
    auto* leftBin = dynamic_cast<const BinaryExpr*>(root->left());
    REQUIRE(leftBin != nullptr);
    CHECK(leftBin->op() == "+");

    auto* leftA = dynamic_cast<const IdentifierExpression*>(leftBin->left());
    REQUIRE(leftA != nullptr);
    CHECK(leftA->name() == "a");

    auto* rightB = dynamic_cast<const IdentifierExpression*>(leftBin->right());
    REQUIRE(rightB != nullptr);
    CHECK(rightB->name() == "b");

    // Right child is Identifier(c)
    auto* rightC = dynamic_cast<const IdentifierExpression*>(root->right());
    REQUIRE(rightC != nullptr);
    CHECK(rightC->name() == "c");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: 'a * (b + c)' respects parentheses on right operand", "[parser][binary][paren]") {
    Parser parser = makeParser("a * (b + c)");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(*)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "*");

    // Left child is Identifier(a)
    auto* leftA = dynamic_cast<const IdentifierExpression*>(root->left());
    REQUIRE(leftA != nullptr);
    CHECK(leftA->name() == "a");

    // Right child is BinaryExpr(+) from inside parens
    auto* rightBin = dynamic_cast<const BinaryExpr*>(root->right());
    REQUIRE(rightBin != nullptr);
    CHECK(rightBin->op() == "+");

    auto* leftB = dynamic_cast<const IdentifierExpression*>(rightBin->left());
    REQUIRE(leftB != nullptr);
    CHECK(leftB->name() == "b");

    auto* rightC = dynamic_cast<const IdentifierExpression*>(rightBin->right());
    REQUIRE(rightC != nullptr);
    CHECK(rightC->name() == "c");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: nested binary in parens '((a + b) * c) / d'", "[parser][binary][paren]") {
    Parser parser = makeParser("((a + b) * c) / d");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(/)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "/");

    auto* leftMul = dynamic_cast<const BinaryExpr*>(root->left());
    REQUIRE(leftMul != nullptr);
    CHECK(leftMul->op() == "*");

    auto* leftAdd = dynamic_cast<const BinaryExpr*>(leftMul->left());
    REQUIRE(leftAdd != nullptr);
    CHECK(leftAdd->op() == "+");

    auto* rightD = dynamic_cast<const IdentifierExpression*>(root->right());
    REQUIRE(rightD != nullptr);
    CHECK(rightD->name() == "d");

    CHECK(parser.isAtEnd());
}

// -----------------------------------------------------------------------------
// 14. Binary Expression Error Handling
// -----------------------------------------------------------------------------
TEST_CASE("parseExpression: missing right operand 'a +' at EOF fails cleanly", "[parser][binary][error]") {
    Parser parser = makeParser("a +");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '+'");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: missing right operand 'a -' at EOF fails cleanly", "[parser][binary][error]") {
    Parser parser = makeParser("a -");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '-'");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: missing right operand 'a *' at EOF fails cleanly", "[parser][binary][error]") {
    Parser parser = makeParser("a *");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '*'");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: missing right operand 'a /' at EOF fails cleanly", "[parser][binary][error]") {
    Parser parser = makeParser("a /");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '/'");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: missing right operand in chained precedence 'a + b *' at EOF fails cleanly", "[parser][binary][error]") {
    Parser parser = makeParser("a + b *");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '*'");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: missing right operand before punctuation 'a * ;' fails cleanly", "[parser][binary][error]") {
    Parser parser = makeParser("a * ;");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '*'");
    // Semicolon was not consumed
    CHECK_FALSE(parser.isAtEnd());
    CHECK(parser.peek().lexeme == ";");
    CHECK(parser.peek().type == TokenType::Punctuation);
}

TEST_CASE("parseExpression: leading binary operator '* a' fails cleanly without consuming '*'", "[parser][binary][error]") {
    Parser parser = makeParser("* a");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK_FALSE(parser.isAtEnd());
    CHECK(parser.peek().lexeme == "*");
    CHECK(parser.peek().type == TokenType::Operator);
}

// -----------------------------------------------------------------------------
// 15. BinaryExpr AST Node Unit Verification
// -----------------------------------------------------------------------------
TEST_CASE("BinaryExpr: AST node accessors and toString", "[ast][binary]") {
    auto left = std::make_unique<IdentifierExpression>("x", SourceLocation{1, 1});
    auto right = std::make_unique<LiteralExpression>(LiteralKind::Integer, "42", SourceLocation{1, 5});

    BinaryExpr bin("+", std::move(left), std::move(right), SourceLocation{1, 1});
    CHECK(bin.op() == "+");
    CHECK(bin.getOp() == "+");
    CHECK(bin.location().line == 1);
    CHECK(bin.location().column == 1);

    REQUIRE(bin.left() != nullptr);
    REQUIRE(bin.getLeft() != nullptr);
    CHECK(bin.left()->toString() == "IdentifierExpression(x)");

    REQUIRE(bin.right() != nullptr);
    REQUIRE(bin.getRight() != nullptr);
    CHECK(bin.right()->toString() == "LiteralExpression(Integer, 42)");

    CHECK(bin.toString() == "BinaryExpr(+, IdentifierExpression(x), LiteralExpression(Integer, 42))");

    auto takenLeft = bin.takeLeft();
    CHECK(bin.left() == nullptr);
    CHECK(takenLeft->toString() == "IdentifierExpression(x)");

    auto takenRight = bin.takeRight();
    CHECK(bin.right() == nullptr);
    CHECK(takenRight->toString() == "LiteralExpression(Integer, 42)");
}

// =============================================================================
// DAY 16 TESTS — Unary Expressions
// =============================================================================

// -----------------------------------------------------------------------------
// 16. Basic Prefix Unary Expressions (-, +, !, ~, ++, --)
// -----------------------------------------------------------------------------
TEST_CASE("parseExpression: unary minus '-x'", "[parser][unary]") {
    Parser parser = makeParser("-x");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* un = dynamic_cast<UnaryExpr*>(expr.get());
    REQUIRE(un != nullptr);
    CHECK(un->op() == "-");
    CHECK(un->getOp() == "-");
    CHECK(un->location().line == 1);
    CHECK(un->location().column == 1);

    auto* operand = dynamic_cast<const IdentifierExpression*>(un->operand());
    REQUIRE(operand != nullptr);
    CHECK(operand->name() == "x");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: unary plus '+x'", "[parser][unary]") {
    Parser parser = makeParser("+x");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* un = dynamic_cast<UnaryExpr*>(expr.get());
    REQUIRE(un != nullptr);
    CHECK(un->op() == "+");
    CHECK(un->getOp() == "+");
    CHECK(un->location().line == 1);
    CHECK(un->location().column == 1);

    auto* operand = dynamic_cast<const IdentifierExpression*>(un->operand());
    REQUIRE(operand != nullptr);
    CHECK(operand->name() == "x");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: logical NOT '!x'", "[parser][unary]") {
    Parser parser = makeParser("!x");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* un = dynamic_cast<UnaryExpr*>(expr.get());
    REQUIRE(un != nullptr);
    CHECK(un->op() == "!");
    CHECK(un->getOp() == "!");

    auto* operand = dynamic_cast<const IdentifierExpression*>(un->operand());
    REQUIRE(operand != nullptr);
    CHECK(operand->name() == "x");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: bitwise NOT '~x'", "[parser][unary]") {
    Parser parser = makeParser("~x");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* un = dynamic_cast<UnaryExpr*>(expr.get());
    REQUIRE(un != nullptr);
    CHECK(un->op() == "~");
    CHECK(un->getOp() == "~");

    auto* operand = dynamic_cast<const IdentifierExpression*>(un->operand());
    REQUIRE(operand != nullptr);
    CHECK(operand->name() == "x");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: prefix increment '++x'", "[parser][unary]") {
    Parser parser = makeParser("++x");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* un = dynamic_cast<UnaryExpr*>(expr.get());
    REQUIRE(un != nullptr);
    CHECK(un->op() == "++");
    CHECK(un->getOp() == "++");
    CHECK(un->location().line == 1);
    CHECK(un->location().column == 1);

    auto* operand = dynamic_cast<const IdentifierExpression*>(un->operand());
    REQUIRE(operand != nullptr);
    CHECK(operand->name() == "x");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: prefix decrement '--x'", "[parser][unary]") {
    Parser parser = makeParser("--x");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* un = dynamic_cast<UnaryExpr*>(expr.get());
    REQUIRE(un != nullptr);
    CHECK(un->op() == "--");
    CHECK(un->getOp() == "--");
    CHECK(un->location().line == 1);
    CHECK(un->location().column == 1);

    auto* operand = dynamic_cast<const IdentifierExpression*>(un->operand());
    REQUIRE(operand != nullptr);
    CHECK(operand->name() == "x");

    CHECK(parser.isAtEnd());
}

// -----------------------------------------------------------------------------
// 17. Unary Expressions with Literals
// -----------------------------------------------------------------------------
TEST_CASE("parseExpression: unary minus with integer literal '-10'", "[parser][unary][literal]") {
    Parser parser = makeParser("-10");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* un = dynamic_cast<UnaryExpr*>(expr.get());
    REQUIRE(un != nullptr);
    CHECK(un->op() == "-");

    auto* lit = dynamic_cast<const LiteralExpression*>(un->operand());
    REQUIRE(lit != nullptr);
    CHECK(lit->kind() == LiteralKind::Integer);
    CHECK(lit->value() == "10");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: unary plus with float literal '+3.14'", "[parser][unary][literal]") {
    Parser parser = makeParser("+3.14");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* un = dynamic_cast<UnaryExpr*>(expr.get());
    REQUIRE(un != nullptr);
    CHECK(un->op() == "+");

    auto* lit = dynamic_cast<const LiteralExpression*>(un->operand());
    REQUIRE(lit != nullptr);
    CHECK(lit->kind() == LiteralKind::Float);
    CHECK(lit->value() == "3.14");

    CHECK(parser.isAtEnd());
}

// -----------------------------------------------------------------------------
// 18. Chained Prefix Unary Expressions
// -----------------------------------------------------------------------------
TEST_CASE("parseExpression: chained prefix '!-x' -> !(-x)", "[parser][unary][chain]") {
    Parser parser = makeParser("!-x");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Outer is UnaryExpr(!)
    auto* outer = dynamic_cast<UnaryExpr*>(expr.get());
    REQUIRE(outer != nullptr);
    CHECK(outer->op() == "!");

    // Inner is UnaryExpr(-)
    auto* inner = dynamic_cast<const UnaryExpr*>(outer->operand());
    REQUIRE(inner != nullptr);
    CHECK(inner->op() == "-");

    auto* operand = dynamic_cast<const IdentifierExpression*>(inner->operand());
    REQUIRE(operand != nullptr);
    CHECK(operand->name() == "x");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: chained prefix '- - x' -> -(-x)", "[parser][unary][chain]") {
    Parser parser = makeParser("- - x");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* outer = dynamic_cast<UnaryExpr*>(expr.get());
    REQUIRE(outer != nullptr);
    CHECK(outer->op() == "-");

    auto* inner = dynamic_cast<const UnaryExpr*>(outer->operand());
    REQUIRE(inner != nullptr);
    CHECK(inner->op() == "-");

    auto* operand = dynamic_cast<const IdentifierExpression*>(inner->operand());
    REQUIRE(operand != nullptr);
    CHECK(operand->name() == "x");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: chained prefix '++--x' -> ++(--x)", "[parser][unary][chain]") {
    Parser parser = makeParser("++--x");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* outer = dynamic_cast<UnaryExpr*>(expr.get());
    REQUIRE(outer != nullptr);
    CHECK(outer->op() == "++");

    auto* inner = dynamic_cast<const UnaryExpr*>(outer->operand());
    REQUIRE(inner != nullptr);
    CHECK(inner->op() == "--");

    auto* operand = dynamic_cast<const IdentifierExpression*>(inner->operand());
    REQUIRE(operand != nullptr);
    CHECK(operand->name() == "x");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: chained prefix '~-x' -> ~(-x)", "[parser][unary][chain]") {
    Parser parser = makeParser("~-x");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* outer = dynamic_cast<UnaryExpr*>(expr.get());
    REQUIRE(outer != nullptr);
    CHECK(outer->op() == "~");

    auto* inner = dynamic_cast<const UnaryExpr*>(outer->operand());
    REQUIRE(inner != nullptr);
    CHECK(inner->op() == "-");

    auto* operand = dynamic_cast<const IdentifierExpression*>(inner->operand());
    REQUIRE(operand != nullptr);
    CHECK(operand->name() == "x");

    CHECK(parser.isAtEnd());
}

// -----------------------------------------------------------------------------
// 19. Interaction between Unary and Binary Expressions
// -----------------------------------------------------------------------------
TEST_CASE("parseExpression: unary on left operand '-x * y' -> (-x) * y", "[parser][unary][binary]") {
    Parser parser = makeParser("-x * y");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(*)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "*");

    // Left is UnaryExpr(-)
    auto* leftUn = dynamic_cast<const UnaryExpr*>(root->left());
    REQUIRE(leftUn != nullptr);
    CHECK(leftUn->op() == "-");

    auto* leftX = dynamic_cast<const IdentifierExpression*>(leftUn->operand());
    REQUIRE(leftX != nullptr);
    CHECK(leftX->name() == "x");

    // Right is Identifier(y)
    auto* rightY = dynamic_cast<const IdentifierExpression*>(root->right());
    REQUIRE(rightY != nullptr);
    CHECK(rightY->name() == "y");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: unary on right operand 'a * -b' -> a * (-b)", "[parser][unary][binary]") {
    Parser parser = makeParser("a * -b");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(*)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "*");

    // Left is Identifier(a)
    auto* leftA = dynamic_cast<const IdentifierExpression*>(root->left());
    REQUIRE(leftA != nullptr);
    CHECK(leftA->name() == "a");

    // Right is UnaryExpr(-)
    auto* rightUn = dynamic_cast<const UnaryExpr*>(root->right());
    REQUIRE(rightUn != nullptr);
    CHECK(rightUn->op() == "-");

    auto* rightB = dynamic_cast<const IdentifierExpression*>(rightUn->operand());
    REQUIRE(rightB != nullptr);
    CHECK(rightB->name() == "b");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: binary subtraction with unary minus 'a - -b' -> a - (-b)", "[parser][unary][binary]") {
    Parser parser = makeParser("a - -b");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(-)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "-");

    auto* leftA = dynamic_cast<const IdentifierExpression*>(root->left());
    REQUIRE(leftA != nullptr);
    CHECK(leftA->name() == "a");

    // Right is UnaryExpr(-)
    auto* rightUn = dynamic_cast<const UnaryExpr*>(root->right());
    REQUIRE(rightUn != nullptr);
    CHECK(rightUn->op() == "-");

    auto* rightB = dynamic_cast<const IdentifierExpression*>(rightUn->operand());
    REQUIRE(rightB != nullptr);
    CHECK(rightB->name() == "b");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: binary addition with unary plus 'a + +b' -> a + (+b)", "[parser][unary][binary]") {
    Parser parser = makeParser("a + +b");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(+)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "+");

    auto* leftA = dynamic_cast<const IdentifierExpression*>(root->left());
    REQUIRE(leftA != nullptr);
    CHECK(leftA->name() == "a");

    // Right is UnaryExpr(+)
    auto* rightUn = dynamic_cast<const UnaryExpr*>(root->right());
    REQUIRE(rightUn != nullptr);
    CHECK(rightUn->op() == "+");

    auto* rightB = dynamic_cast<const IdentifierExpression*>(rightUn->operand());
    REQUIRE(rightB != nullptr);
    CHECK(rightB->name() == "b");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: complex precedence with unary '-a * b + -c / d'", "[parser][unary][binary]") {
    // (-a * b) + (-c / d)
    Parser parser = makeParser("-a * b + -c / d");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(+)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "+");

    // Left child is BinaryExpr(*)
    auto* leftMul = dynamic_cast<const BinaryExpr*>(root->left());
    REQUIRE(leftMul != nullptr);
    CHECK(leftMul->op() == "*");

    auto* unA = dynamic_cast<const UnaryExpr*>(leftMul->left());
    REQUIRE(unA != nullptr);
    CHECK(unA->op() == "-");
    auto* idA = dynamic_cast<const IdentifierExpression*>(unA->operand());
    REQUIRE(idA != nullptr);
    CHECK(idA->name() == "a");

    auto* idB = dynamic_cast<const IdentifierExpression*>(leftMul->right());
    REQUIRE(idB != nullptr);
    CHECK(idB->name() == "b");

    // Right child is BinaryExpr(/)
    auto* rightDiv = dynamic_cast<const BinaryExpr*>(root->right());
    REQUIRE(rightDiv != nullptr);
    CHECK(rightDiv->op() == "/");

    auto* unC = dynamic_cast<const UnaryExpr*>(rightDiv->left());
    REQUIRE(unC != nullptr);
    CHECK(unC->op() == "-");
    auto* idC = dynamic_cast<const IdentifierExpression*>(unC->operand());
    REQUIRE(idC != nullptr);
    CHECK(idC->name() == "c");

    auto* idD = dynamic_cast<const IdentifierExpression*>(rightDiv->right());
    REQUIRE(idD != nullptr);
    CHECK(idD->name() == "d");

    CHECK(parser.isAtEnd());
}

// -----------------------------------------------------------------------------
// 20. Parentheses and Unary Expressions
// -----------------------------------------------------------------------------
TEST_CASE("parseExpression: unary applied to parenthesized expression '-(a + b)'", "[parser][unary][paren]") {
    Parser parser = makeParser("-(a + b)");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is UnaryExpr(-)
    auto* un = dynamic_cast<UnaryExpr*>(expr.get());
    REQUIRE(un != nullptr);
    CHECK(un->op() == "-");

    // Operand is BinaryExpr(+)
    auto* bin = dynamic_cast<const BinaryExpr*>(un->operand());
    REQUIRE(bin != nullptr);
    CHECK(bin->op() == "+");

    auto* leftA = dynamic_cast<const IdentifierExpression*>(bin->left());
    REQUIRE(leftA != nullptr);
    CHECK(leftA->name() == "a");

    auto* rightB = dynamic_cast<const IdentifierExpression*>(bin->right());
    REQUIRE(rightB != nullptr);
    CHECK(rightB->name() == "b");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: parenthesized unary expression '(-x) * y'", "[parser][unary][paren]") {
    Parser parser = makeParser("(-x) * y");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(*)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "*");

    // Left child is UnaryExpr(-) from inside parens
    auto* leftUn = dynamic_cast<const UnaryExpr*>(root->left());
    REQUIRE(leftUn != nullptr);
    CHECK(leftUn->op() == "-");

    auto* leftX = dynamic_cast<const IdentifierExpression*>(leftUn->operand());
    REQUIRE(leftX != nullptr);
    CHECK(leftX->name() == "x");

    auto* rightY = dynamic_cast<const IdentifierExpression*>(root->right());
    REQUIRE(rightY != nullptr);
    CHECK(rightY->name() == "y");

    CHECK(parser.isAtEnd());
}

// -----------------------------------------------------------------------------
// 21. Unary Expression Error Handling
// -----------------------------------------------------------------------------
TEST_CASE("parseExpression: missing operand after '-' at EOF fails cleanly", "[parser][unary][error]") {
    Parser parser = makeParser("-");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '-'");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: missing operand after '++' at EOF fails cleanly", "[parser][unary][error]") {
    Parser parser = makeParser("++");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '++'");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: missing operand after '!' at EOF fails cleanly", "[parser][unary][error]") {
    Parser parser = makeParser("!");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '!'");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: missing operand after '~' at EOF fails cleanly", "[parser][unary][error]") {
    Parser parser = makeParser("~");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '~'");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: missing operand after '--' at EOF fails cleanly", "[parser][unary][error]") {
    Parser parser = makeParser("--");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '--'");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: missing operand after '+' at EOF fails cleanly", "[parser][unary][error]") {
    Parser parser = makeParser("+");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '+'");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: missing operand before punctuation '- ;' fails cleanly", "[parser][unary][error]") {
    Parser parser = makeParser("- ;");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '-'");
    // Semicolon was not consumed
    CHECK_FALSE(parser.isAtEnd());
    CHECK(parser.peek().lexeme == ";");
    CHECK(parser.peek().type == TokenType::Punctuation);
}

TEST_CASE("parseExpression: missing operand before punctuation '++ ;' fails cleanly", "[parser][unary][error]") {
    Parser parser = makeParser("++ ;");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '++'");
    CHECK_FALSE(parser.isAtEnd());
    CHECK(parser.peek().lexeme == ";");
    CHECK(parser.peek().type == TokenType::Punctuation);
}

TEST_CASE("parseExpression: missing operand in chained prefix '!- ;' fails cleanly", "[parser][unary][error]") {
    Parser parser = makeParser("!- ;");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '-'");
    CHECK_FALSE(parser.isAtEnd());
    CHECK(parser.peek().lexeme == ";");
}

// -----------------------------------------------------------------------------
// 22. UnaryExpr AST Node Unit Verification
// -----------------------------------------------------------------------------
TEST_CASE("UnaryExpr: AST node accessors and toString", "[ast][unary]") {
    auto operand = std::make_unique<IdentifierExpression>("x", SourceLocation{1, 2});

    UnaryExpr un("-", std::move(operand), SourceLocation{1, 1});
    CHECK(un.op() == "-");
    CHECK(un.getOp() == "-");
    CHECK(un.location().line == 1);
    CHECK(un.location().column == 1);

    REQUIRE(un.operand() != nullptr);
    REQUIRE(un.getOperand() != nullptr);
    CHECK(un.operand()->toString() == "IdentifierExpression(x)");

    CHECK(un.toString() == "UnaryExpr(-, IdentifierExpression(x))");

    auto taken = un.takeOperand();
    CHECK(un.operand() == nullptr);
    CHECK(taken->toString() == "IdentifierExpression(x)");
}

// =============================================================================
// DAY 17 TESTS — Assignment Expressions
// =============================================================================

// -----------------------------------------------------------------------------
// 23. Basic Simple and Compound Assignments (=, +=, -=, *=, /=)
// -----------------------------------------------------------------------------
TEST_CASE("parseExpression: simple assignment 'x = 10'", "[parser][assign]") {
    Parser parser = makeParser("x = 10");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* assign = dynamic_cast<AssignExpr*>(expr.get());
    REQUIRE(assign != nullptr);
    CHECK(assign->op() == "=");
    CHECK(assign->getOp() == "=");
    CHECK(assign->location().line == 1);
    CHECK(assign->location().column == 1);

    auto* left = dynamic_cast<const IdentifierExpression*>(assign->left());
    REQUIRE(left != nullptr);
    CHECK(left->name() == "x");

    auto* right = dynamic_cast<const LiteralExpression*>(assign->right());
    REQUIRE(right != nullptr);
    CHECK(right->kind() == LiteralKind::Integer);
    CHECK(right->value() == "10");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: compound addition assignment 'x += 5'", "[parser][assign]") {
    Parser parser = makeParser("x += 5");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* assign = dynamic_cast<AssignExpr*>(expr.get());
    REQUIRE(assign != nullptr);
    CHECK(assign->op() == "+=");

    auto* left = dynamic_cast<const IdentifierExpression*>(assign->left());
    REQUIRE(left != nullptr);
    CHECK(left->name() == "x");

    auto* right = dynamic_cast<const LiteralExpression*>(assign->right());
    REQUIRE(right != nullptr);
    CHECK(right->value() == "5");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: compound subtraction assignment 'x -= 2'", "[parser][assign]") {
    Parser parser = makeParser("x -= 2");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* assign = dynamic_cast<AssignExpr*>(expr.get());
    REQUIRE(assign != nullptr);
    CHECK(assign->op() == "-=");

    auto* left = dynamic_cast<const IdentifierExpression*>(assign->left());
    REQUIRE(left != nullptr);
    CHECK(left->name() == "x");

    auto* right = dynamic_cast<const LiteralExpression*>(assign->right());
    REQUIRE(right != nullptr);
    CHECK(right->value() == "2");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: compound multiplication assignment 'x *= 3'", "[parser][assign]") {
    Parser parser = makeParser("x *= 3");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* assign = dynamic_cast<AssignExpr*>(expr.get());
    REQUIRE(assign != nullptr);
    CHECK(assign->op() == "*=");

    auto* left = dynamic_cast<const IdentifierExpression*>(assign->left());
    REQUIRE(left != nullptr);
    CHECK(left->name() == "x");

    auto* right = dynamic_cast<const LiteralExpression*>(assign->right());
    REQUIRE(right != nullptr);
    CHECK(right->value() == "3");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: compound division assignment 'x /= 4'", "[parser][assign]") {
    Parser parser = makeParser("x /= 4");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* assign = dynamic_cast<AssignExpr*>(expr.get());
    REQUIRE(assign != nullptr);
    CHECK(assign->op() == "/=");

    auto* left = dynamic_cast<const IdentifierExpression*>(assign->left());
    REQUIRE(left != nullptr);
    CHECK(left->name() == "x");

    auto* right = dynamic_cast<const LiteralExpression*>(assign->right());
    REQUIRE(right != nullptr);
    CHECK(right->value() == "4");

    CHECK(parser.isAtEnd());
}

// -----------------------------------------------------------------------------
// 24. Right-Associativity of Assignment
// -----------------------------------------------------------------------------
TEST_CASE("parseExpression: right-associativity 'a = b = c'", "[parser][assign][associativity]") {
    // Expected grouping: a = (b = c)
    Parser parser = makeParser("a = b = c");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is AssignExpr(=)
    auto* root = dynamic_cast<AssignExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "=");

    auto* leftA = dynamic_cast<const IdentifierExpression*>(root->left());
    REQUIRE(leftA != nullptr);
    CHECK(leftA->name() == "a");

    // Right child is AssignExpr(=) for "b = c"
    auto* rightAssign = dynamic_cast<const AssignExpr*>(root->right());
    REQUIRE(rightAssign != nullptr);
    CHECK(rightAssign->op() == "=");

    auto* leftB = dynamic_cast<const IdentifierExpression*>(rightAssign->left());
    REQUIRE(leftB != nullptr);
    CHECK(leftB->name() == "b");

    auto* rightC = dynamic_cast<const IdentifierExpression*>(rightAssign->right());
    REQUIRE(rightC != nullptr);
    CHECK(rightC->name() == "c");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: chained compound assignment 'a += b *= c'", "[parser][assign][associativity]") {
    // Expected grouping: a += (b *= c)
    Parser parser = makeParser("a += b *= c");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* root = dynamic_cast<AssignExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "+=");

    auto* leftA = dynamic_cast<const IdentifierExpression*>(root->left());
    REQUIRE(leftA != nullptr);
    CHECK(leftA->name() == "a");

    auto* rightMul = dynamic_cast<const AssignExpr*>(root->right());
    REQUIRE(rightMul != nullptr);
    CHECK(rightMul->op() == "*=");

    auto* leftB = dynamic_cast<const IdentifierExpression*>(rightMul->left());
    REQUIRE(leftB != nullptr);
    CHECK(leftB->name() == "b");

    auto* rightC = dynamic_cast<const IdentifierExpression*>(rightMul->right());
    REQUIRE(rightC != nullptr);
    CHECK(rightC->name() == "c");

    CHECK(parser.isAtEnd());
}

// -----------------------------------------------------------------------------
// 25. Precedence: Assignment is Lower Than Binary and Unary
// -----------------------------------------------------------------------------
TEST_CASE("parseExpression: assignment lower precedence than arithmetic 'x = a + b * c'", "[parser][assign][precedence]") {
    // Expected grouping: x = (a + (b * c))
    Parser parser = makeParser("x = a + b * c");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is AssignExpr(=)
    auto* root = dynamic_cast<AssignExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "=");

    auto* leftX = dynamic_cast<const IdentifierExpression*>(root->left());
    REQUIRE(leftX != nullptr);
    CHECK(leftX->name() == "x");

    // Right is BinaryExpr(+)
    auto* rightAdd = dynamic_cast<const BinaryExpr*>(root->right());
    REQUIRE(rightAdd != nullptr);
    CHECK(rightAdd->op() == "+");

    auto* leftA = dynamic_cast<const IdentifierExpression*>(rightAdd->left());
    REQUIRE(leftA != nullptr);
    CHECK(leftA->name() == "a");

    // Right child of '+' is BinaryExpr(*)
    auto* mulBC = dynamic_cast<const BinaryExpr*>(rightAdd->right());
    REQUIRE(mulBC != nullptr);
    CHECK(mulBC->op() == "*");

    auto* leftB = dynamic_cast<const IdentifierExpression*>(mulBC->left());
    REQUIRE(leftB != nullptr);
    CHECK(leftB->name() == "b");

    auto* rightC = dynamic_cast<const IdentifierExpression*>(mulBC->right());
    REQUIRE(rightC != nullptr);
    CHECK(rightC->name() == "c");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: assignment with unary on RHS 'x = -a'", "[parser][assign][precedence]") {
    // Expected grouping: x = (-a)
    Parser parser = makeParser("x = -a");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* root = dynamic_cast<AssignExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "=");

    auto* leftX = dynamic_cast<const IdentifierExpression*>(root->left());
    REQUIRE(leftX != nullptr);
    CHECK(leftX->name() == "x");

    auto* rightUn = dynamic_cast<const UnaryExpr*>(root->right());
    REQUIRE(rightUn != nullptr);
    CHECK(rightUn->op() == "-");

    auto* rightA = dynamic_cast<const IdentifierExpression*>(rightUn->operand());
    REQUIRE(rightA != nullptr);
    CHECK(rightA->name() == "a");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: assignment with prefix increment 'x = ++y'", "[parser][assign][precedence]") {
    Parser parser = makeParser("x = ++y");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* root = dynamic_cast<AssignExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "=");

    auto* rightUn = dynamic_cast<const UnaryExpr*>(root->right());
    REQUIRE(rightUn != nullptr);
    CHECK(rightUn->op() == "++");

    auto* idY = dynamic_cast<const IdentifierExpression*>(rightUn->operand());
    REQUIRE(idY != nullptr);
    CHECK(idY->name() == "y");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: structural parsing of 'a + b = c'", "[parser][assign][precedence]") {
    // Structurally: (a + b) = c  (no semantic lvalue check yet)
    Parser parser = makeParser("a + b = c");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* root = dynamic_cast<AssignExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "=");

    auto* leftAdd = dynamic_cast<const BinaryExpr*>(root->left());
    REQUIRE(leftAdd != nullptr);
    CHECK(leftAdd->op() == "+");

    auto* rightC = dynamic_cast<const IdentifierExpression*>(root->right());
    REQUIRE(rightC != nullptr);
    CHECK(rightC->name() == "c");

    CHECK(parser.isAtEnd());
}

// -----------------------------------------------------------------------------
// 26. Parentheses Interaction with Assignment
// -----------------------------------------------------------------------------
TEST_CASE("parseExpression: parenthesized assignment '(x = 10)'", "[parser][assign][paren]") {
    Parser parser = makeParser("(x = 10)");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    auto* assign = dynamic_cast<AssignExpr*>(expr.get());
    REQUIRE(assign != nullptr);
    CHECK(assign->op() == "=");

    auto* left = dynamic_cast<const IdentifierExpression*>(assign->left());
    REQUIRE(left != nullptr);
    CHECK(left->name() == "x");

    auto* right = dynamic_cast<const LiteralExpression*>(assign->right());
    REQUIRE(right != nullptr);
    CHECK(right->value() == "10");

    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: parenthesized assignment in binary expression '(x = 5) + 1'", "[parser][assign][paren]") {
    Parser parser = makeParser("(x = 5) + 1");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());

    // Root is BinaryExpr(+)
    auto* root = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(root != nullptr);
    CHECK(root->op() == "+");

    // Left child is AssignExpr(=)
    auto* leftAssign = dynamic_cast<const AssignExpr*>(root->left());
    REQUIRE(leftAssign != nullptr);
    CHECK(leftAssign->op() == "=");

    auto* rightLit = dynamic_cast<const LiteralExpression*>(root->right());
    REQUIRE(rightLit != nullptr);
    CHECK(rightLit->value() == "1");

    CHECK(parser.isAtEnd());
}

// -----------------------------------------------------------------------------
// 27. Assignment Error Handling
// -----------------------------------------------------------------------------
TEST_CASE("parseExpression: missing RHS 'x =' at EOF fails cleanly", "[parser][assign][error]") {
    Parser parser = makeParser("x =");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '='");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: missing RHS 'x +=' at EOF fails cleanly", "[parser][assign][error]") {
    Parser parser = makeParser("x +=");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '+='");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: missing RHS 'x -=' at EOF fails cleanly", "[parser][assign][error]") {
    Parser parser = makeParser("x -=");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '-='");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: missing RHS 'x *=' at EOF fails cleanly", "[parser][assign][error]") {
    Parser parser = makeParser("x *=");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '*='");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: missing RHS 'x /=' at EOF fails cleanly", "[parser][assign][error]") {
    Parser parser = makeParser("x /=");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '/='");
    CHECK(parser.isAtEnd());
}

TEST_CASE("parseExpression: missing RHS before punctuation 'x = ;' fails cleanly", "[parser][assign][error]") {
    Parser parser = makeParser("x = ;");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '='");
    CHECK_FALSE(parser.isAtEnd());
    CHECK(parser.peek().lexeme == ";");
    CHECK(parser.peek().type == TokenType::Punctuation);
}

TEST_CASE("parseExpression: chained assignment missing RHS 'a = b =' fails cleanly", "[parser][assign][error]") {
    Parser parser = makeParser("a = b =");
    auto expr = parser.parseExpression();

    CHECK(expr == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Expected expression after '='");
    CHECK(parser.isAtEnd());
}

// -----------------------------------------------------------------------------
// 28. AssignExpr AST Node Unit Verification
// -----------------------------------------------------------------------------
TEST_CASE("AssignExpr: AST node accessors and toString", "[ast][assign]") {
    auto left = std::make_unique<IdentifierExpression>("total", SourceLocation{1, 1});
    auto right = std::make_unique<LiteralExpression>(LiteralKind::Integer, "100", SourceLocation{1, 9});

    AssignExpr assign("+=", std::move(left), std::move(right), SourceLocation{1, 1});
    CHECK(assign.op() == "+=");
    CHECK(assign.getOp() == "+=");
    CHECK(assign.location().line == 1);
    CHECK(assign.location().column == 1);

    REQUIRE(assign.left() != nullptr);
    REQUIRE(assign.getLeft() != nullptr);
    CHECK(assign.left()->toString() == "IdentifierExpression(total)");

    REQUIRE(assign.right() != nullptr);
    REQUIRE(assign.getRight() != nullptr);
    CHECK(assign.right()->toString() == "LiteralExpression(Integer, 100)");

    CHECK(assign.toString() == "AssignExpr(+=, IdentifierExpression(total), LiteralExpression(Integer, 100))");

    auto takenLeft = assign.takeLeft();
    CHECK(assign.left() == nullptr);
    CHECK(takenLeft->toString() == "IdentifierExpression(total)");

    auto takenRight = assign.takeRight();
    CHECK(assign.right() == nullptr);
    CHECK(takenRight->toString() == "LiteralExpression(Integer, 100)");
}



