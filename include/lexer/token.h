#pragma once

#include <string>

// =============================================================================
// TokenType — Day 4
// =============================================================================
//
// Represents WHAT KIND of token a lexical unit is.
//
// Design notes:
//   - enum class is used for strong typing: there is no implicit conversion
//     to int, and different enum classes cannot be accidentally compared.
//   - Names are PascalCase to read naturally as TokenType::Keyword, etc.
//   - All 9 categories correspond to categories found in the C language.
//   - Eof does not correspond to any source character; it is a sentinel
//     value the lexer emits at end-of-file so later stages can detect
//     the end of the token stream without checking out-of-range indices.
//
// Categories:
//   Identifier  — user-defined names: variables, functions, type names
//   Integer     — integer literals: 42, 0, 0xFF, 077
//   Float       — floating-point literals: 3.14, 1.0f, 2.5e3
//   Char        — character literals: 'a', '\n'
//   String      — string literals: "hello", "world\n"
//   Keyword     — reserved words: int, return, if, while, ...
//   Operator    — operators: +, -, *, /, =, ==, !=, <, >, &&, ||, ...
//   Punctuation — structural characters: (, ), {, }, [, ], ;, ,, .
//   Eof         — end-of-file sentinel; lexeme is always ""
//
enum class TokenType {
    Identifier,
    Integer,
    Float,
    Char,
    String,
    Keyword,
    Operator,
    Punctuation,
    Eof,
};

// =============================================================================
// Token — Day 4
// =============================================================================
//
// Represents ONE lexical token produced by the lexer.
//
// Fields:
//   type    — what kind of token this is (see TokenType above)
//   lexeme  — the exact source text that produced this token
//             stored so the parser can inspect the actual value
//             (e.g. to distinguish "int" from "return" both being Keyword,
//              or "42" from "100" both being Integer)
//   line    — 1-based line number of the first character of the token
//   column  — 1-based column number of the first character of the token
//
// Line/column convention: ONE-BASED throughout CForge.
//   Line 1, column 1 is the first character of the source file.
//   This matches GCC/Clang diagnostic output and most text editors,
//   making error messages intuitive for users.
//
// Token is a plain aggregate struct.
//   No virtual functions, no heap allocation, no inheritance.
//   Cheap to copy; designed to be stored in a std::vector<Token>.
//
struct Token {
    TokenType   type;
    std::string lexeme;
    int         line;    // 1-based
    int         column;  // 1-based
};

// =============================================================================
// tokenTypeName()
// =============================================================================
//
// Returns a human-readable name for a TokenType value.
// Used for debug output, error messages, and test failure diagnostics.
//
// Example: tokenTypeName(TokenType::Keyword) -> "Keyword"
//
const char* tokenTypeName(TokenType t);
