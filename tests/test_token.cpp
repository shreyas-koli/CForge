// Do NOT define CATCH_CONFIG_MAIN here.
// The runner entry point is defined in tests/test_main.cpp.
#include <catch2/catch.hpp>

#include "lexer/token.h"

#include <string>

// =============================================================================
// tokenTypeName() helper tests
// =============================================================================

TEST_CASE("tokenTypeName returns correct string for every TokenType", "[token][tokenTypeName]") {
    CHECK(std::string(tokenTypeName(TokenType::Identifier))  == "Identifier");
    CHECK(std::string(tokenTypeName(TokenType::Integer))     == "Integer");
    CHECK(std::string(tokenTypeName(TokenType::Float))       == "Float");
    CHECK(std::string(tokenTypeName(TokenType::Char))        == "Char");
    CHECK(std::string(tokenTypeName(TokenType::String))      == "String");
    CHECK(std::string(tokenTypeName(TokenType::Keyword))     == "Keyword");
    CHECK(std::string(tokenTypeName(TokenType::Operator))    == "Operator");
    CHECK(std::string(tokenTypeName(TokenType::Punctuation)) == "Punctuation");
    CHECK(std::string(tokenTypeName(TokenType::Eof))         == "Eof");
}

// =============================================================================
// TokenType distinctness
// =============================================================================

TEST_CASE("All TokenType values are mutually distinct", "[token][TokenType]") {
    // Verify that the enum class assigns unique values to each enumerator.
    // This is guaranteed by C++ but making it explicit documents intent.
    CHECK(TokenType::Identifier  != TokenType::Integer);
    CHECK(TokenType::Identifier  != TokenType::Float);
    CHECK(TokenType::Identifier  != TokenType::Char);
    CHECK(TokenType::Identifier  != TokenType::String);
    CHECK(TokenType::Identifier  != TokenType::Keyword);
    CHECK(TokenType::Identifier  != TokenType::Operator);
    CHECK(TokenType::Identifier  != TokenType::Punctuation);
    CHECK(TokenType::Identifier  != TokenType::Eof);
    CHECK(TokenType::Integer     != TokenType::Float);
    CHECK(TokenType::Integer     != TokenType::Char);
    CHECK(TokenType::Integer     != TokenType::String);
    CHECK(TokenType::Integer     != TokenType::Keyword);
    CHECK(TokenType::Integer     != TokenType::Operator);
    CHECK(TokenType::Integer     != TokenType::Punctuation);
    CHECK(TokenType::Integer     != TokenType::Eof);
    CHECK(TokenType::Float       != TokenType::Char);
    CHECK(TokenType::Float       != TokenType::String);
    CHECK(TokenType::Float       != TokenType::Keyword);
    CHECK(TokenType::Float       != TokenType::Operator);
    CHECK(TokenType::Float       != TokenType::Punctuation);
    CHECK(TokenType::Float       != TokenType::Eof);
    CHECK(TokenType::Char        != TokenType::String);
    CHECK(TokenType::Char        != TokenType::Keyword);
    CHECK(TokenType::Char        != TokenType::Operator);
    CHECK(TokenType::Char        != TokenType::Punctuation);
    CHECK(TokenType::Char        != TokenType::Eof);
    CHECK(TokenType::String      != TokenType::Keyword);
    CHECK(TokenType::String      != TokenType::Operator);
    CHECK(TokenType::String      != TokenType::Punctuation);
    CHECK(TokenType::String      != TokenType::Eof);
    CHECK(TokenType::Keyword     != TokenType::Operator);
    CHECK(TokenType::Keyword     != TokenType::Punctuation);
    CHECK(TokenType::Keyword     != TokenType::Eof);
    CHECK(TokenType::Operator    != TokenType::Punctuation);
    CHECK(TokenType::Operator    != TokenType::Eof);
    CHECK(TokenType::Punctuation != TokenType::Eof);
}

// =============================================================================
// Token construction — one test per category
//
// Convention verified: line and column are 1-based.
//   Line 1, column 1 = first character of the source file.
// =============================================================================

TEST_CASE("Token: Identifier", "[token][Token]") {
    // Models the 'count' identifier starting at column 5 of line 1.
    Token t{TokenType::Identifier, "count", 1, 5};

    CHECK(t.type    == TokenType::Identifier);
    CHECK(t.lexeme  == "count");
    CHECK(t.line    == 1);
    CHECK(t.column  == 5);
}

TEST_CASE("Token: Integer literal", "[token][Token]") {
    // Models the literal '42' in: int x = 42;
    Token t{TokenType::Integer, "42", 3, 10};

    CHECK(t.type    == TokenType::Integer);
    CHECK(t.lexeme  == "42");
    CHECK(t.line    == 3);
    CHECK(t.column  == 10);
}

TEST_CASE("Token: Float literal", "[token][Token]") {
    // Models the literal '3.14' in: float pi = 3.14;
    Token t{TokenType::Float, "3.14", 2, 12};

    CHECK(t.type    == TokenType::Float);
    CHECK(t.lexeme  == "3.14");
    CHECK(t.line    == 2);
    CHECK(t.column  == 12);
}

TEST_CASE("Token: Char literal", "[token][Token]") {
    // Models the literal 'a' in: char c = 'a';
    Token t{TokenType::Char, "'a'", 4, 11};

    CHECK(t.type    == TokenType::Char);
    CHECK(t.lexeme  == "'a'");
    CHECK(t.line    == 4);
    CHECK(t.column  == 11);
}

TEST_CASE("Token: String literal", "[token][Token]") {
    // Models the literal "hello" in: char* s = "hello";
    Token t{TokenType::String, "\"hello\"", 5, 13};

    CHECK(t.type    == TokenType::String);
    CHECK(t.lexeme  == "\"hello\"");
    CHECK(t.line    == 5);
    CHECK(t.column  == 13);
}

TEST_CASE("Token: Keyword", "[token][Token]") {
    // Models 'int' in: int count = 42;
    Token t{TokenType::Keyword, "int", 1, 1};

    CHECK(t.type    == TokenType::Keyword);
    CHECK(t.lexeme  == "int");
    CHECK(t.line    == 1);
    CHECK(t.column  == 1);
}

TEST_CASE("Token: Operator", "[token][Token]") {
    // Models '=' in: int count = 42;
    Token t{TokenType::Operator, "=", 1, 11};

    CHECK(t.type    == TokenType::Operator);
    CHECK(t.lexeme  == "=");
    CHECK(t.line    == 1);
    CHECK(t.column  == 11);
}

TEST_CASE("Token: Punctuation", "[token][Token]") {
    // Models ';' at the end of: int count = 42;
    Token t{TokenType::Punctuation, ";", 1, 14};

    CHECK(t.type    == TokenType::Punctuation);
    CHECK(t.lexeme  == ";");
    CHECK(t.line    == 1);
    CHECK(t.column  == 14);
}

TEST_CASE("Token: Eof", "[token][Token]") {
    // Eof has no corresponding source character.
    // By convention the lexeme is empty and location is the position
    // immediately after the last real character.
    Token t{TokenType::Eof, "", 10, 1};

    CHECK(t.type    == TokenType::Eof);
    CHECK(t.lexeme  == "");
    CHECK(t.line    == 10);
    CHECK(t.column  == 1);
}

// =============================================================================
// Conceptual sequence: int count = 42;
// =============================================================================
// Verifies that a sequence of tokens models a real C statement correctly.
// This also implicitly checks that Token is copyable and storable in a vector.

#include <vector>

TEST_CASE("Token sequence models 'int count = 42;'", "[token][Token]") {
    std::vector<Token> tokens = {
        {TokenType::Keyword,     "int",   1,  1},
        {TokenType::Identifier,  "count", 1,  5},
        {TokenType::Operator,    "=",     1, 11},
        {TokenType::Integer,     "42",    1, 13},
        {TokenType::Punctuation, ";",     1, 15},
        {TokenType::Eof,         "",      1, 16},
    };

    REQUIRE(tokens.size() == 6);

    CHECK(tokens[0].type   == TokenType::Keyword);
    CHECK(tokens[0].lexeme == "int");

    CHECK(tokens[1].type   == TokenType::Identifier);
    CHECK(tokens[1].lexeme == "count");

    CHECK(tokens[2].type   == TokenType::Operator);
    CHECK(tokens[2].lexeme == "=");

    CHECK(tokens[3].type   == TokenType::Integer);
    CHECK(tokens[3].lexeme == "42");

    CHECK(tokens[4].type   == TokenType::Punctuation);
    CHECK(tokens[4].lexeme == ";");

    CHECK(tokens[5].type   == TokenType::Eof);
    CHECK(tokens[5].lexeme == "");

    // Verify 1-based line convention — everything on line 1.
    for (const auto& tok : tokens) {
        CHECK(tok.line == 1);
    }
}

// =============================================================================
// Line/column convention verification
// =============================================================================

TEST_CASE("Line and column are stored exactly as provided (1-based)", "[token][convention]") {
    // First character of file -> line 1, column 1 (not 0, 0)
    Token first{TokenType::Keyword, "return", 1, 1};
    CHECK(first.line   == 1);
    CHECK(first.column == 1);

    // Deeply nested token on a later line
    Token deep{TokenType::Integer, "255", 42, 16};
    CHECK(deep.line   == 42);
    CHECK(deep.column == 16);
}

// =============================================================================
// Lexeme stores exact source text (not a transformed value)
// =============================================================================

TEST_CASE("Token lexeme preserves exact source text", "[token][lexeme]") {
    // Hexadecimal integer — lexeme keeps "0xFF", not 255
    Token hex{TokenType::Integer, "0xFF", 1, 1};
    CHECK(hex.lexeme == "0xFF");

    // Octal integer — lexeme keeps "077", not 63
    Token oct{TokenType::Integer, "077", 1, 1};
    CHECK(oct.lexeme == "077");

    // Float with exponent — lexeme keeps full source text
    Token sci{TokenType::Float, "2.5e3", 1, 1};
    CHECK(sci.lexeme == "2.5e3");

    // Escape sequence inside string — lexeme keeps raw source text
    Token str{TokenType::String, "\"hello\\n\"", 1, 1};
    CHECK(str.lexeme == "\"hello\\n\"");
}
