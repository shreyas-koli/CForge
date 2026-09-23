// Do NOT define CATCH_CONFIG_MAIN here.
// The runner entry point is defined in tests/test_main.cpp.
#include <catch2/catch.hpp>

#include "lexer/lexer.h"
#include "lexer/token.h"

#include <string>
#include <vector>

// =============================================================================
// Helper: tokenize a source string and return all tokens INCLUDING Eof.
// =============================================================================
static std::vector<Token> lex(const std::string& src) {
    Lexer lexer(src);
    return lexer.tokenize();
}

// Helper: tokenize and return only the first non-Eof token.
static Token lexFirst(const std::string& src) {
    return lex(src)[0];
}

// =============================================================================
// 1. lookupKeyword() — keyword table unit tests
//    Tested independently of Lexer so the table can be verified in isolation.
// =============================================================================

TEST_CASE("lookupKeyword: all 10 C keywords return Keyword", "[lexer][keyword]") {
    CHECK(lookupKeyword("int")    == TokenType::Keyword);
    CHECK(lookupKeyword("char")   == TokenType::Keyword);
    CHECK(lookupKeyword("float")  == TokenType::Keyword);
    CHECK(lookupKeyword("double") == TokenType::Keyword);
    CHECK(lookupKeyword("void")   == TokenType::Keyword);
    CHECK(lookupKeyword("if")     == TokenType::Keyword);
    CHECK(lookupKeyword("else")   == TokenType::Keyword);
    CHECK(lookupKeyword("while")  == TokenType::Keyword);
    CHECK(lookupKeyword("for")    == TokenType::Keyword);
    CHECK(lookupKeyword("return") == TokenType::Keyword);
}

TEST_CASE("lookupKeyword: non-keywords return Identifier", "[lexer][keyword]") {
    CHECK(lookupKeyword("variable")    == TokenType::Identifier);
    CHECK(lookupKeyword("myFunction")  == TokenType::Identifier);
    CHECK(lookupKeyword("counter")     == TokenType::Identifier);
    CHECK(lookupKeyword("returnValue") == TokenType::Identifier);
    CHECK(lookupKeyword("integer")     == TokenType::Identifier);
    CHECK(lookupKeyword("iff")         == TokenType::Identifier);
    CHECK(lookupKeyword("returns")     == TokenType::Identifier);
    CHECK(lookupKeyword("_counter")    == TokenType::Identifier);
    CHECK(lookupKeyword("Int")         == TokenType::Identifier); // case-sensitive
    CHECK(lookupKeyword("INT")         == TokenType::Identifier); // case-sensitive
}

// =============================================================================
// 2. Basic identifier scanning
// =============================================================================

TEST_CASE("Lexer scans basic identifier: variable", "[lexer][identifier]") {
    Token t = lexFirst("variable");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "variable");
}

TEST_CASE("Lexer scans function-like identifier: myFunction", "[lexer][identifier]") {
    Token t = lexFirst("myFunction");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "myFunction");
}

TEST_CASE("Lexer scans identifier with trailing digits: counter2", "[lexer][identifier]") {
    Token t = lexFirst("counter2");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "counter2");
}

TEST_CASE("Lexer scans identifier beginning with underscore: _counter", "[lexer][identifier]") {
    Token t = lexFirst("_counter");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "_counter");
}

TEST_CASE("Lexer scans identifier with underscore in middle: student_name", "[lexer][identifier]") {
    Token t = lexFirst("student_name");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "student_name");
}

// =============================================================================
// 3. Keyword scanning — all 10 Day 5 keywords
// =============================================================================

TEST_CASE("Lexer scans all 10 C keywords correctly", "[lexer][keyword]") {
    struct Case { std::string src; std::string lexeme; };
    std::vector<Case> cases = {
        {"int",    "int"},
        {"char",   "char"},
        {"float",  "float"},
        {"double", "double"},
        {"void",   "void"},
        {"if",     "if"},
        {"else",   "else"},
        {"while",  "while"},
        {"for",    "for"},
        {"return", "return"},
    };

    for (const auto& c : cases) {
        Token t = lexFirst(c.src);
        CHECK(t.type   == TokenType::Keyword);
        CHECK(t.lexeme == c.lexeme);
    }
}

// =============================================================================
// 4. Critical boundary: returnValue must NOT be split at "return"
// =============================================================================

TEST_CASE("returnValue is classified as Identifier, not split at 'return'", "[lexer][boundary]") {
    Token t = lexFirst("returnValue");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "returnValue");
}

// =============================================================================
// 5. Keyword-prefix boundary cases
//    Words that START WITH a keyword but are NOT keywords.
// =============================================================================

TEST_CASE("'integer' is Identifier, not split at 'int'", "[lexer][boundary]") {
    Token t = lexFirst("integer");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "integer");
}

TEST_CASE("'iff' is Identifier, not the keyword 'if'", "[lexer][boundary]") {
    Token t = lexFirst("iff");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "iff");
}

TEST_CASE("'returns' is Identifier, not the keyword 'return'", "[lexer][boundary]") {
    Token t = lexFirst("returns");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "returns");
}

TEST_CASE("'whileTrue' is Identifier, not the keyword 'while'", "[lexer][boundary]") {
    Token t = lexFirst("whileTrue");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "whileTrue");
}

TEST_CASE("'forLoop' is Identifier, not the keyword 'for'", "[lexer][boundary]") {
    Token t = lexFirst("forLoop");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "forLoop");
}

TEST_CASE("'elseif' is Identifier, not the keyword 'else'", "[lexer][boundary]") {
    Token t = lexFirst("elseif");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "elseif");
}

TEST_CASE("'doubled' is Identifier, not the keyword 'double'", "[lexer][boundary]") {
    Token t = lexFirst("doubled");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "doubled");
}

// =============================================================================
// 6. Keyword lookup is case-sensitive (C is case-sensitive)
// =============================================================================

TEST_CASE("Keywords are case-sensitive: 'Int' is Identifier", "[lexer][keyword]") {
    Token t = lexFirst("Int");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "Int");
}

TEST_CASE("Keywords are case-sensitive: 'RETURN' is Identifier", "[lexer][keyword]") {
    Token t = lexFirst("RETURN");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "RETURN");
}

// =============================================================================
// 7. Lexeme preservation — exact source text
// =============================================================================

TEST_CASE("Lexemes are preserved exactly as they appear in source", "[lexer][lexeme]") {
    CHECK(lexFirst("counter").lexeme    == "counter");
    CHECK(lexFirst("counter2").lexeme   == "counter2");
    CHECK(lexFirst("_count").lexeme     == "_count");
    CHECK(lexFirst("myVar").lexeme      == "myVar");
    CHECK(lexFirst("int").lexeme        == "int");
    CHECK(lexFirst("returnValue").lexeme == "returnValue");
}

// =============================================================================
// 8. Line and column tracking (1-based convention)
// =============================================================================

TEST_CASE("First token on first line starts at line=1, column=1", "[lexer][location]") {
    Token t = lexFirst("int");
    CHECK(t.line   == 1);
    CHECK(t.column == 1);
}

TEST_CASE("Token after leading spaces has correct column", "[lexer][location]") {
    // "    int" — 4 spaces then 'i', so column should be 5
    Token t = lexFirst("    int");
    CHECK(t.type   == TokenType::Keyword);
    CHECK(t.lexeme == "int");
    CHECK(t.line   == 1);
    CHECK(t.column == 5);
}

TEST_CASE("Token on second line has correct line number", "[lexer][location]") {
    // First token: "x" on line 1. Second token: "y" on line 2.
    auto tokens = lex("x\ny");
    REQUIRE(tokens.size() >= 2);

    CHECK(tokens[0].type   == TokenType::Identifier);
    CHECK(tokens[0].lexeme == "x");
    CHECK(tokens[0].line   == 1);
    CHECK(tokens[0].column == 1);

    CHECK(tokens[1].type   == TokenType::Identifier);
    CHECK(tokens[1].lexeme == "y");
    CHECK(tokens[1].line   == 2);
    CHECK(tokens[1].column == 1);
}

TEST_CASE("Second token on same line has correct column", "[lexer][location]") {
    // "int x" — 'i' at col 1, space at col 4, 'x' at col 5
    auto tokens = lex("int x");
    REQUIRE(tokens.size() >= 2);

    CHECK(tokens[0].lexeme == "int");
    CHECK(tokens[0].column == 1);

    CHECK(tokens[1].lexeme == "x");
    CHECK(tokens[1].column == 5);
}

// =============================================================================
// 9. Realistic source snippet: "int counter;"
//    Tests identifier + keyword in sequence.
// =============================================================================

TEST_CASE("Source 'int counter;' produces correct token sequence", "[lexer][sequence]") {
    auto tokens = lex("int counter;");

    // Expected: Keyword("int"), Identifier("counter"), Punctuation(";"), Eof
    REQUIRE(tokens.size() >= 3);

    CHECK(tokens[0].type   == TokenType::Keyword);
    CHECK(tokens[0].lexeme == "int");
    CHECK(tokens[0].line   == 1);
    CHECK(tokens[0].column == 1);

    CHECK(tokens[1].type   == TokenType::Identifier);
    CHECK(tokens[1].lexeme == "counter");
    CHECK(tokens[1].line   == 1);
    CHECK(tokens[1].column == 5);

    // Last token must be Eof
    CHECK(tokens.back().type == TokenType::Eof);
}

// =============================================================================
// 10. tokenize() always ends with Eof
// =============================================================================

TEST_CASE("Empty source tokenizes to just Eof", "[lexer][eof]") {
    auto tokens = lex("");
    REQUIRE(tokens.size() == 1);
    CHECK(tokens[0].type == TokenType::Eof);
}

TEST_CASE("Whitespace-only source tokenizes to just Eof", "[lexer][eof]") {
    auto tokens = lex("   \t\n  ");
    REQUIRE(tokens.size() == 1);
    CHECK(tokens[0].type == TokenType::Eof);
}

TEST_CASE("Token stream always ends with exactly one Eof", "[lexer][eof]") {
    auto tokens = lex("int x");
    CHECK(tokens.back().type == TokenType::Eof);
    // Verify there's exactly one Eof
    int eofCount = 0;
    for (const auto& t : tokens) {
        if (t.type == TokenType::Eof) ++eofCount;
    }
    CHECK(eofCount == 1);
}

// =============================================================================
// 11. Multi-keyword sequence
// =============================================================================

TEST_CASE("Multi-keyword sequence: 'int char float'", "[lexer][keyword]") {
    auto tokens = lex("int char float");

    REQUIRE(tokens.size() >= 4); // 3 keywords + Eof

    CHECK(tokens[0].type   == TokenType::Keyword);
    CHECK(tokens[0].lexeme == "int");

    CHECK(tokens[1].type   == TokenType::Keyword);
    CHECK(tokens[1].lexeme == "char");

    CHECK(tokens[2].type   == TokenType::Keyword);
    CHECK(tokens[2].lexeme == "float");

    CHECK(tokens.back().type == TokenType::Eof);
}

// =============================================================================
// 12. Mixed identifier/keyword sequence
// =============================================================================

TEST_CASE("'return counter' produces Keyword then Identifier", "[lexer][sequence]") {
    auto tokens = lex("return counter");
    REQUIRE(tokens.size() >= 3);

    CHECK(tokens[0].type   == TokenType::Keyword);
    CHECK(tokens[0].lexeme == "return");

    CHECK(tokens[1].type   == TokenType::Identifier);
    CHECK(tokens[1].lexeme == "counter");

    CHECK(tokens.back().type == TokenType::Eof);
}

// =============================================================================
// DAY 6 TESTS — Numeric Literals
// =============================================================================

// -----------------------------------------------------------------------------
// Decimal integers
// -----------------------------------------------------------------------------

TEST_CASE("Integer: 0", "[lexer][numeric][integer]") {
    Token t = lexFirst("0");
    CHECK(t.type   == TokenType::Integer);
    CHECK(t.lexeme == "0");
}

TEST_CASE("Integer: 10", "[lexer][numeric][integer]") {
    Token t = lexFirst("10");
    CHECK(t.type   == TokenType::Integer);
    CHECK(t.lexeme == "10");
}

TEST_CASE("Integer: 123", "[lexer][numeric][integer]") {
    Token t = lexFirst("123");
    CHECK(t.type   == TokenType::Integer);
    CHECK(t.lexeme == "123");
}

TEST_CASE("Integer: 42", "[lexer][numeric][integer]") {
    Token t = lexFirst("42");
    CHECK(t.type   == TokenType::Integer);
    CHECK(t.lexeme == "42");
}

TEST_CASE("Integer: large value 99999", "[lexer][numeric][integer]") {
    Token t = lexFirst("99999");
    CHECK(t.type   == TokenType::Integer);
    CHECK(t.lexeme == "99999");
}

// -----------------------------------------------------------------------------
// Hexadecimal integers
// -----------------------------------------------------------------------------

TEST_CASE("Integer hex: 0xFF", "[lexer][numeric][integer][hex]") {
    Token t = lexFirst("0xFF");
    CHECK(t.type   == TokenType::Integer);
    CHECK(t.lexeme == "0xFF");
}

TEST_CASE("Integer hex: 0xff (lowercase)", "[lexer][numeric][integer][hex]") {
    Token t = lexFirst("0xff");
    CHECK(t.type   == TokenType::Integer);
    CHECK(t.lexeme == "0xff");
}

TEST_CASE("Integer hex: 0X10 (uppercase X)", "[lexer][numeric][integer][hex]") {
    Token t = lexFirst("0X10");
    CHECK(t.type   == TokenType::Integer);
    CHECK(t.lexeme == "0X10");
}

TEST_CASE("Integer hex: 0xabcdef", "[lexer][numeric][integer][hex]") {
    Token t = lexFirst("0xabcdef");
    CHECK(t.type   == TokenType::Integer);
    CHECK(t.lexeme == "0xabcdef");
}

TEST_CASE("Integer hex: 0XCAFE", "[lexer][numeric][integer][hex]") {
    Token t = lexFirst("0XCAFE");
    CHECK(t.type   == TokenType::Integer);
    CHECK(t.lexeme == "0XCAFE");
}

TEST_CASE("Integer hex: 0x0 (zero in hex)", "[lexer][numeric][integer][hex]") {
    Token t = lexFirst("0x0");
    CHECK(t.type   == TokenType::Integer);
    CHECK(t.lexeme == "0x0");
}

// -----------------------------------------------------------------------------
// Decimal floats (decimal point)
// -----------------------------------------------------------------------------

TEST_CASE("Float: 3.14", "[lexer][numeric][float]") {
    Token t = lexFirst("3.14");
    CHECK(t.type   == TokenType::Float);
    CHECK(t.lexeme == "3.14");
}

TEST_CASE("Float: 0.5", "[lexer][numeric][float]") {
    Token t = lexFirst("0.5");
    CHECK(t.type   == TokenType::Float);
    CHECK(t.lexeme == "0.5");
}

TEST_CASE("Float: 10.0", "[lexer][numeric][float]") {
    Token t = lexFirst("10.0");
    CHECK(t.type   == TokenType::Float);
    CHECK(t.lexeme == "10.0");
}

TEST_CASE("Float: 123.456", "[lexer][numeric][float]") {
    Token t = lexFirst("123.456");
    CHECK(t.type   == TokenType::Float);
    CHECK(t.lexeme == "123.456");
}

// -----------------------------------------------------------------------------
// Exponent floats
// -----------------------------------------------------------------------------

TEST_CASE("Float exponent: 1e10", "[lexer][numeric][float][exponent]") {
    Token t = lexFirst("1e10");
    CHECK(t.type   == TokenType::Float);
    CHECK(t.lexeme == "1e10");
}

TEST_CASE("Float exponent: 1E10 (uppercase E)", "[lexer][numeric][float][exponent]") {
    Token t = lexFirst("1E10");
    CHECK(t.type   == TokenType::Float);
    CHECK(t.lexeme == "1E10");
}

TEST_CASE("Float exponent: 3.14e5", "[lexer][numeric][float][exponent]") {
    Token t = lexFirst("3.14e5");
    CHECK(t.type   == TokenType::Float);
    CHECK(t.lexeme == "3.14e5");
}

TEST_CASE("Float exponent: 3.14E5 (uppercase E)", "[lexer][numeric][float][exponent]") {
    Token t = lexFirst("3.14E5");
    CHECK(t.type   == TokenType::Float);
    CHECK(t.lexeme == "3.14E5");
}

TEST_CASE("Float exponent with negative sign: 2.5e-3", "[lexer][numeric][float][exponent]") {
    Token t = lexFirst("2.5e-3");
    CHECK(t.type   == TokenType::Float);
    CHECK(t.lexeme == "2.5e-3");
}

TEST_CASE("Float exponent with positive sign: 2.5e+3", "[lexer][numeric][float][exponent]") {
    Token t = lexFirst("2.5e+3");
    CHECK(t.type   == TokenType::Float);
    CHECK(t.lexeme == "2.5e+3");
}

TEST_CASE("Float exponent: integer base with exponent 1e10", "[lexer][numeric][float][exponent]") {
    // No decimal point, but has exponent -> Float
    Token t = lexFirst("1e10");
    CHECK(t.type   == TokenType::Float);
}

// -----------------------------------------------------------------------------
// INTEGER vs FLOAT distinction — critical cases
// -----------------------------------------------------------------------------

TEST_CASE("'123' is Integer, '123.0' is Float", "[lexer][numeric][boundary]") {
    CHECK(lexFirst("123").type  == TokenType::Integer);
    CHECK(lexFirst("123.0").type == TokenType::Float);
}

TEST_CASE("'0' is Integer, '0.5' is Float", "[lexer][numeric][boundary]") {
    CHECK(lexFirst("0").type   == TokenType::Integer);
    CHECK(lexFirst("0.5").type == TokenType::Float);
}

TEST_CASE("'0xFF' is Integer (hex never becomes float)", "[lexer][numeric][boundary]") {
    CHECK(lexFirst("0xFF").type == TokenType::Integer);
    CHECK(lexFirst("0XFF").type == TokenType::Integer);
}

// -----------------------------------------------------------------------------
// Critical: '3.14' must be ONE Float token, NOT three tokens
// -----------------------------------------------------------------------------

TEST_CASE("'3.14' is a single Float token, not split into 3/./14", "[lexer][numeric][boundary]") {
    auto tokens = lex("3.14");
    // Should be: Float("3.14"), Eof
    REQUIRE(tokens.size() == 2);
    CHECK(tokens[0].type   == TokenType::Float);
    CHECK(tokens[0].lexeme == "3.14");
    CHECK(tokens[1].type   == TokenType::Eof);
}

// -----------------------------------------------------------------------------
// Lexeme preservation — exact source text stored, no conversion
// -----------------------------------------------------------------------------

TEST_CASE("Numeric lexemes preserve exact source text", "[lexer][numeric][lexeme]") {
    CHECK(lexFirst("123").lexeme    == "123");
    CHECK(lexFirst("0xFF").lexeme   == "0xFF");
    CHECK(lexFirst("3.14").lexeme   == "3.14");
    CHECK(lexFirst("2.5e-3").lexeme == "2.5e-3");
    CHECK(lexFirst("1E10").lexeme   == "1E10");
    CHECK(lexFirst("0.5").lexeme    == "0.5");
}

// -----------------------------------------------------------------------------
// Location tracking for numeric tokens
// -----------------------------------------------------------------------------

TEST_CASE("Numeric token at start has line=1, column=1", "[lexer][numeric][location]") {
    Token t = lexFirst("42");
    CHECK(t.line   == 1);
    CHECK(t.column == 1);
}

TEST_CASE("Numeric token after spaces has correct column", "[lexer][numeric][location]") {
    // "   123" -> 3 spaces, then '1' at column 4
    Token t = lexFirst("   123");
    CHECK(t.type   == TokenType::Integer);
    CHECK(t.lexeme == "123");
    CHECK(t.line   == 1);
    CHECK(t.column == 4);
}

TEST_CASE("Numeric token on second line has correct line number", "[lexer][numeric][location]") {
    // "x\n42" -> identifier on line 1, integer on line 2
    auto tokens = lex("x\n42");
    REQUIRE(tokens.size() >= 2);
    CHECK(tokens[0].line == 1);
    CHECK(tokens[1].type == TokenType::Integer);
    CHECK(tokens[1].line == 2);
    CHECK(tokens[1].column == 1);
}

// -----------------------------------------------------------------------------
// Realistic source snippets
// -----------------------------------------------------------------------------

TEST_CASE("'int x = 123;' produces Keyword Identifier Punct Integer Punct Eof",
          "[lexer][numeric][sequence]") {
    auto tokens = lex("int x = 123;");
    // Expected types: Keyword, Identifier, Punct('='), Integer, Punct(';'), Eof
    REQUIRE(tokens.size() >= 5);

    CHECK(tokens[0].type   == TokenType::Keyword);
    CHECK(tokens[0].lexeme == "int");

    CHECK(tokens[1].type   == TokenType::Identifier);
    CHECK(tokens[1].lexeme == "x");

    // tokens[2] is '=' (Punctuation fallthrough)

    CHECK(tokens[3].type   == TokenType::Integer);
    CHECK(tokens[3].lexeme == "123");

    CHECK(tokens.back().type == TokenType::Eof);
}

TEST_CASE("'float value = 3.14;' produces Keyword Identifier Punct Float Punct Eof",
          "[lexer][numeric][sequence]") {
    auto tokens = lex("float value = 3.14;");
    REQUIRE(tokens.size() >= 5);

    CHECK(tokens[0].type   == TokenType::Keyword);
    CHECK(tokens[0].lexeme == "float");

    CHECK(tokens[1].type   == TokenType::Identifier);
    CHECK(tokens[1].lexeme == "value");

    // tokens[2] is '=' (Punctuation fallthrough)

    CHECK(tokens[3].type   == TokenType::Float);
    CHECK(tokens[3].lexeme == "3.14");

    CHECK(tokens.back().type == TokenType::Eof);
}

TEST_CASE("Sequence of mixed numeric literals tokenizes correctly",
          "[lexer][numeric][sequence]") {
    // "10 3.14 0xFF 1e10"
    auto tokens = lex("10 3.14 0xFF 1e10");
    REQUIRE(tokens.size() >= 5);  // 4 numbers + Eof

    CHECK(tokens[0].type   == TokenType::Integer);
    CHECK(tokens[0].lexeme == "10");

    CHECK(tokens[1].type   == TokenType::Float);
    CHECK(tokens[1].lexeme == "3.14");

    CHECK(tokens[2].type   == TokenType::Integer);
    CHECK(tokens[2].lexeme == "0xFF");

    CHECK(tokens[3].type   == TokenType::Float);
    CHECK(tokens[3].lexeme == "1e10");

    CHECK(tokens.back().type == TokenType::Eof);
}

// -----------------------------------------------------------------------------
// Identifier interaction — digits do NOT make an identifier start valid
// Day 5 behavior preserved: identifiers/keywords still scan correctly
// alongside numeric literals.
// -----------------------------------------------------------------------------

TEST_CASE("'counter123' after integer is still Identifier", "[lexer][numeric][interaction]") {
    // A word starting with a letter is always an identifier.
    Token t = lexFirst("counter123");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "counter123");
}

TEST_CASE("Integer and identifier are separate tokens: '42abc'",
          "[lexer][numeric][interaction]") {
    // In C, '42abc' is not a valid token. The lexer scans '42' as Integer,
    // then 'abc' as Identifier (the parser/semantic phase will reject it).
    auto tokens = lex("42abc");
    REQUIRE(tokens.size() >= 3);  // Integer + Identifier + Eof
    CHECK(tokens[0].type   == TokenType::Integer);
    CHECK(tokens[0].lexeme == "42");
    CHECK(tokens[1].type   == TokenType::Identifier);
    CHECK(tokens[1].lexeme == "abc");
}
