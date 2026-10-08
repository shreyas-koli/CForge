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
