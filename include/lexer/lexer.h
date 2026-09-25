#pragma once

#include "lexer/token.h"

#include <string>
#include <vector>

// =============================================================================
// Lexer — Day 5 / Day 6 / Day 7
// =============================================================================
//
// Converts a C source string into a sequence of Tokens.
//
// Day 5 scope: identifier and keyword scanning.
// Day 6 scope: integer and floating-point literal scanning.
// Day 7 scope: string and character literal scanning with escape sequences.
// Future days will add: operators, punctuation, comments,
// and preprocessor directives.
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

    // Return the character ONE position ahead of the current cursor.
    // Returns '\0' if past end. Does not advance the cursor.
    // Used to detect two-character prefixes like '0x' and '0X'.
    char peekNext() const;

    // Scan a complete identifier-or-keyword token.
    // Pre-condition: peek() is a letter or underscore.
    // Collects all subsequent letters, digits, and underscores.
    // Performs keyword-table lookup to determine the final TokenType.
    Token scanIdentifierOrKeyword();

    // Scan a complete numeric literal (integer or float) token.
    // Pre-condition: peek() is a decimal digit '0'-'9'.
    // Handles:
    //   - Hexadecimal integers:  0x / 0X prefix followed by hex digits
    //   - Decimal integers:      one or more digits
    //   - Decimal floats:        digits . digits
    //   - Exponent floats:       digits [. digits] e/E [+/-] digits
    // Returns TokenType::Integer or TokenType::Float.
    Token scanNumericLiteral();

    // Scan a complete string literal token (Day 7).
    // Pre-condition: peek() == '"'.
    // Consumes the opening quote, all content (handling escape sequences),
    // and the closing quote. If the string is unterminated (EOF or raw
    // newline reached before closing '"'), returns an error token.
    // The lexeme includes the surrounding quotes and escape sequences
    // exactly as they appear in the source.
    Token scanStringLiteral();

    // Scan a complete character literal token (Day 7).
    // Pre-condition: peek() == '\''.
    // Consumes the opening quote, one character or escape sequence, and
    // the closing quote. Returns an error token if unterminated.
    // The lexeme includes the surrounding quotes.
    Token scanCharLiteral();
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

// =============================================================================
// decodeEscape()
// =============================================================================
//
// Given the character that follows a backslash in a C source literal,
// returns the decoded character value.
//
// Supported sequences (Day 7):
//   \n  ->  newline  ('\n')
//   \t  ->  tab      ('\t')
//   \\  ->  backslash('\\')
//   \'  ->  single-quote ('\'')
//   \"  ->  double-quote ('"')
//   \0  ->  null     ('\0')
//   \r  ->  carriage return ('\r')
//
// Unknown escape: returns '\0' (the lexer emits the source characters
// as-is in the lexeme; a later phase reports the diagnostic).
//
char decodeEscape(char escapedChar);

// =============================================================================
// isLexerError()
// =============================================================================
//
// Returns true if the given token represents a lexer-level error.
//
// Day 7 error strategy: unterminated string/character literals are emitted
// as Punctuation tokens whose lexeme begins with the sentinel prefix
// "<error:". This keeps the Token structure unchanged (no new TokenType)
// while allowing tests and callers to detect and inspect errors.
//
// Example lexeme for an unterminated string: "<error: unterminated string>"
//
bool isLexerError(const Token& t);
