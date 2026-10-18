#include "parser/parser.h"

// =============================================================================
// parseExpression() — Day 13 & Day 14
// =============================================================================
//
// Top-level expression parsing function.
// Day 14 implements naive left-associative binary expression parsing:
//   expression:
//       primary-expression ( ('+' | '-' | '*' | '/') primary-expression )*
//
// Grouping is strictly left-to-right without operator precedence:
//   "a + b * c" -> (a + b) * c
//
static bool isBinaryOperator(const Token& tok) {
    if (tok.type != TokenType::Operator) {
        return false;
    }
    return tok.lexeme == "+" || tok.lexeme == "-" ||
           tok.lexeme == "*" || tok.lexeme == "/";
}

std::unique_ptr<Expression> Parser::parseExpression() {
    std::unique_ptr<Expression> left = parsePrimary();
    if (!left) {
        return nullptr;
    }

    while (!isAtEnd() && isBinaryOperator(peek())) {
        Token opToken = advance();
        SourceLocation loc = left->location();

        std::unique_ptr<Expression> right = parsePrimary();
        if (!right) {
            error("Expected expression after '" + opToken.lexeme + "'");
            return nullptr;
        }

        left = std::make_unique<BinaryExpr>(opToken.lexeme, std::move(left), std::move(right), loc);
    }

    return left;
}

// =============================================================================
// parsePrimary() — Day 12 & Day 13
// =============================================================================
//
// Grammar:
//   primary-expression:
//       identifier
//       integer-literal
//       float-literal
//       char-literal
//       string-literal
//       ( expression )
//
// Behavior:
//   If current token matches one of the primary categories, advances the token
//   stream and returns a std::unique_ptr to the corresponding AST node.
//   If current token is not a primary expression (or at EOF), returns nullptr
//   without advancing the token stream.
//
std::unique_ptr<Expression> Parser::parsePrimary() {
    if (isAtEnd()) {
        return nullptr;
    }

    const Token& tok = peek();

    // 1. Integer Literal
    if (tok.type == TokenType::Integer) {
        advance();
        return std::make_unique<LiteralExpression>(
            LiteralKind::Integer, tok.lexeme, SourceLocation{tok.line, tok.column});
    }

    // 2. Float Literal
    if (tok.type == TokenType::Float) {
        advance();
        return std::make_unique<LiteralExpression>(
            LiteralKind::Float, tok.lexeme, SourceLocation{tok.line, tok.column});
    }

    // 3. Char Literal
    if (tok.type == TokenType::Char) {
        advance();
        return std::make_unique<LiteralExpression>(
            LiteralKind::Char, tok.lexeme, SourceLocation{tok.line, tok.column});
    }

    // 4. String Literal
    if (tok.type == TokenType::String) {
        advance();
        return std::make_unique<LiteralExpression>(
            LiteralKind::String, tok.lexeme, SourceLocation{tok.line, tok.column});
    }

    // 5. Bare Identifier
    if (tok.type == TokenType::Identifier) {
        advance();
        return std::make_unique<IdentifierExpression>(
            tok.lexeme, SourceLocation{tok.line, tok.column});
    }

    // 6. Parenthesized Expression: ( expression )
    if (tok.type == TokenType::Punctuation && tok.lexeme == "(") {
        advance(); // consume '('

        // Check for empty parentheses: ()
        if (check(TokenType::Punctuation) && peek().lexeme == ")") {
            advance(); // consume ')'
            error("Empty parenthesized expression");
            return nullptr;
        }

        std::unique_ptr<Expression> expr = parseExpression();
        if (!expr) {
            if (!hasError()) {
                error("Expected expression after '('");
            }
            return nullptr;
        }

        if (!check(TokenType::Punctuation) || peek().lexeme != ")") {
            error("Expected ')' after expression");
            return nullptr;
        }

        advance(); // consume ')'
        return expr;
    }

    // Not a primary expression: fail cleanly without consuming tokens.
    return nullptr;
}
