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

    // Program & Declaration parsing (Day 18)
    // Entry point for full translation unit parsing.
    std::unique_ptr<Program> parseProgram();

    // Declaration parsing (Day 18)
    // Top-level declaration parser delegating to parseFunctionDeclaration().
    std::unique_ptr<Function> parseDeclaration();

    // Function declaration and definition parsing (Day 18)
    // Grammar:
    //   function-declaration:
    //       type-specifier identifier '(' parameter-list? ')' ';'
    //       type-specifier identifier '(' parameter-list? ')' '{' ... '}'
    std::unique_ptr<Function> parseFunctionDeclaration();

    // Expression parsing (Day 13 — Day 17)
    // Top-level expression entry point delegating to assignment parsing.
    std::unique_ptr<Expression> parseExpression();

    // Assignment expression parsing (Day 17)
    // Operators: '=', '+=', '-=', '*=', '/='
    // Right-associative. Lowest expression precedence level.
    std::unique_ptr<Expression> parseAssignment();

    // Precedence-climbing binary expression parser (Day 15, Day 21, Day 22, Day 23)
    // Precedence levels:
    //   Level 4 (highest): '*', '/', '%'
    //   Level 3:           '+', '-'
    //   Level 2:           '<', '>', '<=', '>='
    //   Level 1:           '==', '!='
    // Left-associative: recursive right-hand calls use precedence + 1.
    std::unique_ptr<Expression> parseBinaryExpression(int minPrecedence = 1);

    // Unary expression parsing (Day 16)
    // Grammar:
    //   unary-expression:
    //       ('+' | '-' | '!' | '~' | '++' | '--') unary-expression
    //       primary-expression
    std::unique_ptr<Expression> parseUnary();

    // Primary expression parsing (Day 12 & Day 13)
    // Grammar:
    //   primary-expression:
    //       identifier
    //       integer-literal
    //       float-literal
    //       char-literal
    //       string-literal
    //       ( expression )
    // Returns std::unique_ptr<Expression> if current token begins a primary expression,
    // or nullptr cleanly without consuming tokens if current token is not a primary expression.
    std::unique_ptr<Expression> parsePrimary();

    // Parser State & Lookahead Helpers
    bool isAtEnd() const;
    const Token& peek() const;
    const Token& peekAt(std::size_t offset) const;
    const Token& peekNext() const;
    const Token& previous() const;
    Token advance();
    bool check(TokenType type) const;
    bool match(TokenType type);

    // Error handling (Day 13)
    bool hasError() const { return m_hasError; }
    const std::string& errorMessage() const { return m_errorMessage; }
    void error(const std::string& message);

private:
    std::vector<Token> m_tokens;
    std::size_t m_current{0};
    bool m_hasError{false};
    std::string m_errorMessage;
};
