#include "parser/parser.h"

// =============================================================================
// parseExpression() & parseBinaryExpression() — Day 15 Operator Precedence
// =============================================================================
//
// Precedence-climbing binary expression parser.
// Precedence levels:
//   Level 2 (higher): '*', '/'
//   Level 1 (lower):  '+', '-'
//
// All four operators are left-associative. Left-associativity is preserved by
// passing (precedence + 1) as the minimum precedence for the right-hand operand.
//
static int getOperatorPrecedence(const Token& tok) {
    if (tok.type != TokenType::Operator) {
        return 0;
    }
    if (tok.lexeme == "*" || tok.lexeme == "/") {
        return 2;
    }
    if (tok.lexeme == "+" || tok.lexeme == "-") {
        return 1;
    }
    return 0;
}

std::unique_ptr<Expression> Parser::parseBinaryExpression(int minPrecedence) {
    std::unique_ptr<Expression> left = parsePrimary();
    if (!left) {
        return nullptr;
    }

    while (!isAtEnd()) {
        const Token& opToken = peek();
        int prec = getOperatorPrecedence(opToken);
        if (prec < minPrecedence) {
            break;
        }

        Token op = advance();
        SourceLocation loc = left->location();

        // Left-associative: pass prec + 1
        std::unique_ptr<Expression> right = parseBinaryExpression(prec + 1);
        if (!right) {
            if (!hasError()) {
                error("Expected expression after '" + op.lexeme + "'");
            }
            return nullptr;
        }

        left = std::make_unique<BinaryExpr>(op.lexeme, std::move(left), std::move(right), loc);
    }

    return left;
}

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseBinaryExpression(1);
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
