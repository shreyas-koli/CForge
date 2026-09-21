#include "lexer/token.h"

// =============================================================================
// tokenTypeName()
// =============================================================================
//
// Maps every TokenType enumerator to its human-readable name string.
// This function is the single authoritative place to add new type names as
// future token categories are introduced.
//
// Returns a string literal (pointer into read-only data segment); the caller
// must not free it.
//
const char* tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::Identifier:  return "Identifier";
        case TokenType::Integer:     return "Integer";
        case TokenType::Float:       return "Float";
        case TokenType::Char:        return "Char";
        case TokenType::String:      return "String";
        case TokenType::Keyword:     return "Keyword";
        case TokenType::Operator:    return "Operator";
        case TokenType::Punctuation: return "Punctuation";
        case TokenType::Eof:         return "Eof";
    }
    return "Unknown"; // unreachable, but silences compiler warnings
}
