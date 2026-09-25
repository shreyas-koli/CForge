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

// =============================================================================
// DAY 7 TESTS — Character & String Literals
// =============================================================================

// Helper: confirm a token is NOT a lexer error
static void requireNotError(const Token& t) {
    REQUIRE_FALSE(isLexerError(t));
}

// =============================================================================
// decodeEscape() — unit tests (tested independently of Lexer)
// =============================================================================

TEST_CASE("decodeEscape: \\n -> newline", "[lexer][escape]") {
    CHECK(decodeEscape('n') == '\n');
}
TEST_CASE("decodeEscape: \\t -> tab", "[lexer][escape]") {
    CHECK(decodeEscape('t') == '\t');
}
TEST_CASE("decodeEscape: \\\\ -> backslash", "[lexer][escape]") {
    CHECK(decodeEscape('\\') == '\\');
}
TEST_CASE("decodeEscape: \\' -> single quote", "[lexer][escape]") {
    CHECK(decodeEscape('\'') == '\'');
}
TEST_CASE("decodeEscape: \\\" -> double quote", "[lexer][escape]") {
    CHECK(decodeEscape('"') == '"');
}
TEST_CASE("decodeEscape: \\0 -> null", "[lexer][escape]") {
    CHECK(decodeEscape('0') == '\0');
}
TEST_CASE("decodeEscape: \\r -> carriage return", "[lexer][escape]") {
    CHECK(decodeEscape('r') == '\r');
}
TEST_CASE("decodeEscape: unknown escape returns \\0", "[lexer][escape]") {
    CHECK(decodeEscape('q') == '\0');
    CHECK(decodeEscape('z') == '\0');
}

// =============================================================================
// isLexerError() helper
// =============================================================================

TEST_CASE("isLexerError returns false for normal tokens", "[lexer][error]") {
    Token normal{TokenType::String, "\"hello\"", 1, 1};
    CHECK_FALSE(isLexerError(normal));

    Token kw{TokenType::Keyword, "int", 1, 1};
    CHECK_FALSE(isLexerError(kw));
}

TEST_CASE("isLexerError returns true for error sentinel lexemes", "[lexer][error]") {
    Token err{TokenType::Punctuation, "<error: unterminated string>", 1, 1};
    CHECK(isLexerError(err));
}

// =============================================================================
// String literal scanning — basic
// =============================================================================

TEST_CASE("String: \"hello\" is a single String token", "[lexer][string]") {
    auto tokens = lex("\"hello\"");
    REQUIRE(tokens.size() == 2);   // String + Eof
    CHECK(tokens[0].type   == TokenType::String);
    CHECK(tokens[0].lexeme == "\"hello\"");
    CHECK(tokens[1].type   == TokenType::Eof);
}

TEST_CASE("String: \"hello world\" (with space)", "[lexer][string]") {
    Token t = lexFirst("\"hello world\"");
    CHECK(t.type   == TokenType::String);
    CHECK(t.lexeme == "\"hello world\"");
}

TEST_CASE("String: empty string \"\"", "[lexer][string]") {
    Token t = lexFirst("\"\"");
    CHECK(t.type   == TokenType::String);
    CHECK(t.lexeme == "\"\"");
}

TEST_CASE("String: single character \"a\"", "[lexer][string]") {
    Token t = lexFirst("\"a\"");
    CHECK(t.type   == TokenType::String);
    CHECK(t.lexeme == "\"a\"");
}

// =============================================================================
// String literal — escape sequences
// =============================================================================

TEST_CASE("String escape: \"\\n\" contains newline escape", "[lexer][string][escape]") {
    // Source: "  \n  "  (the two-character source sequence backslash + n)
    Token t = lexFirst("\"\\n\"");
    CHECK(t.type == TokenType::String);
    // Lexeme preserves source spelling: opening quote, backslash, n, closing quote
    CHECK(t.lexeme == "\"\\n\"");
    requireNotError(t);
}

TEST_CASE("String escape: \"\\t\" contains tab escape", "[lexer][string][escape]") {
    Token t = lexFirst("\"\\t\"");
    CHECK(t.type   == TokenType::String);
    CHECK(t.lexeme == "\"\\t\"");
}

TEST_CASE("String escape: \"\\\\\" contains escaped backslash", "[lexer][string][escape]") {
    Token t = lexFirst("\"\\\\\"");
    CHECK(t.type   == TokenType::String);
    CHECK(t.lexeme == "\"\\\\\"");
}

TEST_CASE("String escape: \"\\\"\" contains escaped double quote", "[lexer][string][escape]") {
    Token t = lexFirst("\"\\\"\"");
    CHECK(t.type   == TokenType::String);
    CHECK(t.lexeme == "\"\\\"\"");
}

TEST_CASE("String escape: \"hello\\n\" mixed content", "[lexer][string][escape]") {
    Token t = lexFirst("\"hello\\n\"");
    CHECK(t.type   == TokenType::String);
    CHECK(t.lexeme == "\"hello\\n\"");
}

TEST_CASE("String escape: \"hello\\tworld\" with tab escape", "[lexer][string][escape]") {
    Token t = lexFirst("\"hello\\tworld\"");
    CHECK(t.type   == TokenType::String);
    CHECK(t.lexeme == "\"hello\\tworld\"");
}

TEST_CASE("String escape: \"C:\\\\code\" with double backslash", "[lexer][string][escape]") {
    Token t = lexFirst("\"C:\\\\code\"");
    CHECK(t.type   == TokenType::String);
    CHECK(t.lexeme == "\"C:\\\\code\"");
}

// =============================================================================
// Character literal scanning — basic
// =============================================================================

TEST_CASE("Char: 'a' is a single Char token", "[lexer][char]") {
    auto tokens = lex("'a'");
    REQUIRE(tokens.size() == 2);   // Char + Eof
    CHECK(tokens[0].type   == TokenType::Char);
    CHECK(tokens[0].lexeme == "'a'");
    CHECK(tokens[1].type   == TokenType::Eof);
}

TEST_CASE("Char: 'b'", "[lexer][char]") {
    Token t = lexFirst("'b'");
    CHECK(t.type   == TokenType::Char);
    CHECK(t.lexeme == "'b'");
}

TEST_CASE("Char: '1' (digit character)", "[lexer][char]") {
    Token t = lexFirst("'1'");
    CHECK(t.type   == TokenType::Char);
    CHECK(t.lexeme == "'1'");
}

TEST_CASE("Char: 'Z' (uppercase letter)", "[lexer][char]") {
    Token t = lexFirst("'Z'");
    CHECK(t.type   == TokenType::Char);
    CHECK(t.lexeme == "'Z'");
}

// =============================================================================
// Character literal — escape sequences
// =============================================================================

TEST_CASE("Char escape: '\\n' newline escape", "[lexer][char][escape]") {
    Token t = lexFirst("'\\n'");
    CHECK(t.type   == TokenType::Char);
    CHECK(t.lexeme == "'\\n'");
    requireNotError(t);
}

TEST_CASE("Char escape: '\\t' tab escape", "[lexer][char][escape]") {
    Token t = lexFirst("'\\t'");
    CHECK(t.type   == TokenType::Char);
    CHECK(t.lexeme == "'\\t'");
}

TEST_CASE("Char escape: '\\\\' backslash escape", "[lexer][char][escape]") {
    Token t = lexFirst("'\\\\' ");
    CHECK(t.type   == TokenType::Char);
    CHECK(t.lexeme == "'\\\\'");
}

TEST_CASE("Char escape: '\\'' single-quote escape", "[lexer][char][escape]") {
    Token t = lexFirst("'\\''");
    CHECK(t.type   == TokenType::Char);
    CHECK(t.lexeme == "'\\''");
}

TEST_CASE("Char escape: '\\\"' double-quote escape", "[lexer][char][escape]") {
    Token t = lexFirst("'\\\"'");
    CHECK(t.type   == TokenType::Char);
    CHECK(t.lexeme == "'\\\"'");
}

// =============================================================================
// Lexeme preservation — quoted literals store source text including quotes
// =============================================================================

TEST_CASE("String lexeme includes surrounding double quotes", "[lexer][string][lexeme]") {
    Token t = lexFirst("\"abc\"");
    // Lexeme must start and end with '"'
    REQUIRE(t.lexeme.size() >= 2);
    CHECK(t.lexeme.front() == '"');
    CHECK(t.lexeme.back()  == '"');
    CHECK(t.lexeme == "\"abc\"");
}

TEST_CASE("Char lexeme includes surrounding single quotes", "[lexer][char][lexeme]") {
    Token t = lexFirst("'x'");
    REQUIRE(t.lexeme.size() >= 3);
    CHECK(t.lexeme.front() == '\'');
    CHECK(t.lexeme.back()  == '\'');
    CHECK(t.lexeme == "'x'");
}

// =============================================================================
// Location tracking — 1-based, points to opening quote
// =============================================================================

TEST_CASE("String token at start: line=1, column=1", "[lexer][string][location]") {
    Token t = lexFirst("\"hello\"");
    CHECK(t.type   == TokenType::String);
    CHECK(t.line   == 1);
    CHECK(t.column == 1);
}

TEST_CASE("String token after spaces: correct column", "[lexer][string][location]") {
    // "   \"hi\"" -> 3 spaces then opening quote at column 4
    Token t = lexFirst("   \"hi\"");
    CHECK(t.type   == TokenType::String);
    CHECK(t.line   == 1);
    CHECK(t.column == 4);
}

TEST_CASE("Char token at start: line=1, column=1", "[lexer][char][location]") {
    Token t = lexFirst("'a'");
    CHECK(t.type   == TokenType::Char);
    CHECK(t.line   == 1);
    CHECK(t.column == 1);
}

TEST_CASE("Char token after spaces: correct column", "[lexer][char][location]") {
    // "  'a'" -> 2 spaces then '\'' at column 3
    Token t = lexFirst("  'a'");
    CHECK(t.type   == TokenType::Char);
    CHECK(t.line   == 1);
    CHECK(t.column == 3);
}

// =============================================================================
// Unterminated literal error handling
// =============================================================================

TEST_CASE("Unterminated string returns error token, does not crash", "[lexer][string][error]") {
    // "hello  (no closing quote, no newline before EOF)
    Token t = lexFirst("\"hello");
    CHECK(isLexerError(t));
    // Must not crash and must be safe to inspect
    CHECK_FALSE(t.lexeme.empty());
}

TEST_CASE("String terminated by newline is unterminated error", "[lexer][string][error]") {
    // "hello\n  (raw newline before closing quote)
    Token t = lexFirst("\"hello\n");
    CHECK(isLexerError(t));
}

TEST_CASE("Unterminated char literal returns error token, does not crash", "[lexer][char][error]") {
    // 'a  (no closing quote)
    Token t = lexFirst("'a");
    CHECK(isLexerError(t));
}

TEST_CASE("Empty char literal '' returns error token", "[lexer][char][error]") {
    Token t = lexFirst("''");
    CHECK(isLexerError(t));
}

// =============================================================================
// Day 5–6 interactions preserved
// =============================================================================

TEST_CASE("Identifiers still scanned correctly after Day 7", "[lexer][string][interaction]") {
    Token t = lexFirst("counter");
    CHECK(t.type   == TokenType::Identifier);
    CHECK(t.lexeme == "counter");
}

TEST_CASE("Keywords still scanned correctly after Day 7", "[lexer][char][interaction]") {
    Token t = lexFirst("int");
    CHECK(t.type   == TokenType::Keyword);
    CHECK(t.lexeme == "int");
}

TEST_CASE("Numeric literals still scanned correctly after Day 7", "[lexer][string][interaction]") {
    CHECK(lexFirst("42").type   == TokenType::Integer);
    CHECK(lexFirst("3.14").type == TokenType::Float);
    CHECK(lexFirst("0xFF").type == TokenType::Integer);
}

// =============================================================================
// Realistic source snippets
// =============================================================================

TEST_CASE("'char c = 'a';' — keyword, identifier, char literal in sequence",
          "[lexer][char][sequence]") {
    auto tokens = lex("char c = 'a';");
    // Expected: Keyword("char"), Identifier("c"), Punct("="), Char("'a'"), Punct(";"), Eof
    REQUIRE(tokens.size() >= 5);

    CHECK(tokens[0].type   == TokenType::Keyword);
    CHECK(tokens[0].lexeme == "char");

    CHECK(tokens[1].type   == TokenType::Identifier);
    CHECK(tokens[1].lexeme == "c");

    // tokens[2] = '=' (Punctuation fallthrough)

    CHECK(tokens[3].type   == TokenType::Char);
    CHECK(tokens[3].lexeme == "'a'");

    CHECK(tokens.back().type == TokenType::Eof);
}

TEST_CASE("String assignment: identifier and string literal in sequence",
          "[lexer][string][sequence]") {
    // char* msg = "hello\n";
    auto tokens = lex("msg = \"hello\\n\";");
    // Identifier("msg"), Punct("="), String("\"hello\\n\""), Punct(";"), Eof
    REQUIRE(tokens.size() >= 4);

    CHECK(tokens[0].type   == TokenType::Identifier);
    CHECK(tokens[0].lexeme == "msg");

    // tokens[1] = '=' (Punctuation fallthrough)

    CHECK(tokens[2].type   == TokenType::String);
    CHECK(tokens[2].lexeme == "\"hello\\n\"");

    CHECK(tokens.back().type == TokenType::Eof);
}

TEST_CASE("Multiple string tokens in sequence", "[lexer][string][sequence]") {
    auto tokens = lex("\"hello\" \"world\"");
    REQUIRE(tokens.size() >= 3);  // String + String + Eof

    CHECK(tokens[0].type   == TokenType::String);
    CHECK(tokens[0].lexeme == "\"hello\"");

    CHECK(tokens[1].type   == TokenType::String);
    CHECK(tokens[1].lexeme == "\"world\"");

    CHECK(tokens.back().type == TokenType::Eof);
}
