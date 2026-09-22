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
