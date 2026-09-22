#pragma once

#include "lexer/token.h"

#include <string>
#include <vector>

// =============================================================================
// Lexer — Day 5
// =============================================================================
//
// Converts a C source string into a sequence of Tokens.
//
// Day 5 scope: identifier and keyword scanning only.
// Future days will add: integers, floats, strings, chars, operators,
// punctuation, comments, and preprocessor directives.
//
// Usage:
//   Lexer lexer(sourceCode);
//   std::vector<Token> tokens = lexer.tokenize();
//
// The last token in the returned vector is always TokenType::Eof.
//
// Line/column convention: 1-based, matching the Day 4 Token definition.
//   Line 1, column 1 = first character of the source string.
//
class Lexer {
public:
    // Construct a Lexer over the given source text.
    // The source string is stored by value; the Lexer owns its copy.
    explicit Lexer(std::string source);

    // Scan and return the next token from the source.
    // Repeatedly calling nextToken() drives the lexer forward.
    // Once the source is exhausted, every subsequent call returns Eof.
    Token nextToken();

    // Convenience: scan the entire source and return all tokens.
    // The returned vector always ends with a single Eof token.
    std::vector<Token> tokenize();

private:
    std::string m_source;   // full source text
    std::size_t m_pos;      // current read position (byte index into m_source)
    int         m_line;     // current 1-based line number
    int         m_column;   // current 1-based column number

    // --- Internal helpers ---------------------------------------------------

    // True if the read cursor has reached or passed the end of the source.
    bool isAtEnd() const;

    // Return the character at the current position without advancing.
    char peek() const;

    // Return the character at the current position and advance past it.
    // Updates m_line and m_column correctly for newline characters.
    char advance();

    // Advance past any whitespace characters (space, tab, \r, \n).
    // Updates line/column tracking as newlines are consumed.
    void skipWhitespace();

    // Scan a complete identifier-or-keyword token.
    // Pre-condition: peek() is a letter or underscore.
    // Collects all subsequent letters, digits, and underscores.
    // Performs keyword-table lookup to determine the final TokenType.
    Token scanIdentifierOrKeyword();
};

// =============================================================================
// lookupKeyword()
// =============================================================================
//
// Given an identifier-like lexeme, returns TokenType::Keyword if the lexeme
// is a C keyword, or TokenType::Identifier otherwise.
//
// Declared here so unit tests can verify the lookup table independently
// of the full Lexer class.
//
TokenType lookupKeyword(const std::string& text);
