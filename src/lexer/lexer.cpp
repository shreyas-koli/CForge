#include "lexer/lexer.h"

#include <cctype>       // std::isalpha, std::isdigit, std::isalnum
#include <stdexcept>    // std::runtime_error
#include <unordered_map>

// =============================================================================
// lookupKeyword()
// =============================================================================
//
// Single authoritative keyword table for CForge.
//
// Design:
//   - std::unordered_map gives O(1) average-case lookup.
//   - Declared static const inside the function: the map is constructed once
//     on first call and reused on every subsequent call (no overhead).
//   - To add a new C keyword, add one line to the initializer list below.
//     No other file needs to change.
//
// C keywords supported in Day 5:
//   int, char, float, double, void,
//   if, else, while, for, return
//
// Additional keywords (do, struct, typedef, etc.) will be added in future days
// as the semantic analyzer requires them.
//
TokenType lookupKeyword(const std::string& text) {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"int",    TokenType::Keyword},
        {"char",   TokenType::Keyword},
        {"float",  TokenType::Keyword},
        {"double", TokenType::Keyword},
        {"void",   TokenType::Keyword},
        {"if",     TokenType::Keyword},
        {"else",   TokenType::Keyword},
        {"while",  TokenType::Keyword},
        {"for",    TokenType::Keyword},
        {"return", TokenType::Keyword},
    };

    auto it = keywords.find(text);
    if (it != keywords.end()) {
        return it->second; // found -> Keyword
    }
    return TokenType::Identifier; // not found -> Identifier
}

// =============================================================================
// Lexer constructor
// =============================================================================

Lexer::Lexer(std::string source)
    : m_source(std::move(source))
    , m_pos(0)
    , m_line(1)      // 1-based: first line is line 1
    , m_column(1)    // 1-based: first column is column 1
{}

// =============================================================================
// isAtEnd()
// =============================================================================

bool Lexer::isAtEnd() const {
    return m_pos >= m_source.size();
}

// =============================================================================
// peek()
// =============================================================================
//
// Returns the current character without advancing the position.
// Returns '\0' when at end of source so callers can test for end-of-input
// without a separate isAtEnd() call in most cases.
//
char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return m_source[m_pos];
}

// =============================================================================
// advance()
// =============================================================================
//
// Returns the current character and moves the read cursor forward by one.
// Maintains the 1-based line/column counters:
//   - A newline '\n' increments m_line and resets m_column to 1.
//   - Any other character increments m_column.
//
char Lexer::advance() {
    char c = m_source[m_pos++];
    if (c == '\n') {
        ++m_line;
        m_column = 1;
    } else {
        ++m_column;
    }
    return c;
}

// =============================================================================
// skipWhitespace()
// =============================================================================
//
// Advances past all consecutive whitespace characters.
// Whitespace is: space ' ', tab '\t', carriage return '\r', newline '\n'.
// Line/column tracking is handled inside advance().
//
void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else {
            break;
        }
    }
}

// =============================================================================
// scanIdentifierOrKeyword()
// =============================================================================
//
// Pre-condition: peek() is a letter (a-z, A-Z) or underscore ('_').
//
// Algorithm:
//   1. Record the start position for the lexeme and the token's source location.
//   2. Consume characters while they are letters, digits, or underscores.
//      (This is the complete identifier-like sequence.)
//   3. Extract the lexeme from the source text (no character-by-character copy).
//   4. Perform keyword-table lookup on the complete lexeme.
//   5. Return a Token with the resolved type, the exact lexeme, and location.
//
// Why scan first, then look up?
//   "returnValue" must scan all 11 characters before lookup.
//   Lookup of "returnValue" finds no match -> Identifier.
//   If we stopped at "return" (6 chars) and returned a Keyword, we would
//   incorrectly split "returnValue" into two tokens.
//
Token Lexer::scanIdentifierOrKeyword() {
    // Record location of the FIRST character of this token.
    int startLine   = m_line;
    int startColumn = m_column;
    std::size_t startPos = m_pos;

    // Consume the first character (already verified to be letter or underscore).
    advance();

    // Consume all subsequent letters, digits, and underscores.
    while (!isAtEnd()) {
        char c = peek();
        if (std::isalpha(static_cast<unsigned char>(c)) ||
            std::isdigit(static_cast<unsigned char>(c)) ||
            c == '_') {
            advance();
        } else {
            break;
        }
    }

    // Extract lexeme as a substring of the original source.
    // This avoids building the string character-by-character.
    std::string lexeme = m_source.substr(startPos, m_pos - startPos);

    // Determine whether this lexeme is a keyword or an identifier.
    TokenType type = lookupKeyword(lexeme);

    return Token{type, lexeme, startLine, startColumn};
}

// =============================================================================
// nextToken()
// =============================================================================
//
// Returns the next token from the source.
// Day 5 handles: whitespace (skip), identifiers, keywords, and end-of-file.
// Unknown characters are skipped with a warning to avoid crashing tests
// that include punctuation like ';' for realistic source snippets.
//
Token Lexer::nextToken() {
    skipWhitespace();

    if (isAtEnd()) {
        return Token{TokenType::Eof, "", m_line, m_column};
    }

    char c = peek();

    // --- Identifier or Keyword ---
    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        return scanIdentifierOrKeyword();
    }

    // --- Unknown character (Day 5: skip, to allow realistic test snippets) ---
    // Later days will replace this branch with proper operator/punctuation/
    // number scanning. For now, we advance past unknown characters silently
    // so that a snippet like "int counter;" doesn't block the test.
    int unknownLine   = m_line;
    int unknownColumn = m_column;
    advance();
    return Token{TokenType::Punctuation, std::string(1, c), unknownLine, unknownColumn};
}

// =============================================================================
// tokenize()
// =============================================================================
//
// Drives nextToken() until Eof and collects all tokens into a vector.
// The Eof token is included as the final element.
//
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token t = nextToken();
        tokens.push_back(t);
        if (t.type == TokenType::Eof) {
            break;
        }
    }
    return tokens;
}
