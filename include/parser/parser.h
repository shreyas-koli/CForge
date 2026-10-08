#pragma once

#include "ast/ast.h"
#include "lexer/lexer.h"
#include "lexer/token.h"

#include <memory>
#include <string>
#include <vector>

// =============================================================================
// Parser — CForge Recursive-Descent Parser
// =============================================================================
//
// Converts a stream of Tokens produced by the Lexer into an Abstract Syntax Tree (AST).
//
class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    explicit Parser(Lexer& lexer);

    // Primary expression parsing (Day 12)
    // Grammar:
    //   primary-expression:
    //       identifier
    //       integer-literal
    //       float-literal
    //       char-literal
    //       string-literal
    // Returns std::unique_ptr<Expression> if current token is a primary expression,
    // or nullptr cleanly without consuming tokens if current token is not a primary expression.
    std::unique_ptr<Expression> parsePrimary();

    // Parser State & Lookahead Helpers
    bool isAtEnd() const;
    const Token& peek() const;
    const Token& previous() const;
    Token advance();
    bool check(TokenType type) const;
    bool match(TokenType type);

private:
    std::vector<Token> m_tokens;
    std::size_t m_current{0};
};
