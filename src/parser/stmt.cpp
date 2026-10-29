#include "parser/parser.h"

// =============================================================================
// parseStatement() — Day 28 Statement Parsing Entry Point
// =============================================================================
//
// Grammar:
//   statement:
//       iteration-statement
//       selection-statement
//       compound-statement
//
std::unique_ptr<Statement> Parser::parseStatement() {
    if (isAtEnd()) {
        return nullptr;
    }

    // Iteration statement 'while' (Day 31)
    if (check(TokenType::Keyword) && peek().lexeme == "while") {
        return parseWhileStatement();
    }

    // Iteration statement 'do-while' (Day 32)
    if (check(TokenType::Keyword) && peek().lexeme == "do") {
        return parseDoWhileStatement();
    }

    // Iteration statement 'for' (Day 33)
    if (check(TokenType::Keyword) && peek().lexeme == "for") {
        return parseForStatement();
    }

    // Selection statement 'if' (Day 29)
    if (check(TokenType::Keyword) && peek().lexeme == "if") {
        return parseIfStatement();
    }

    // Compound statement '{ ... }'
    if (check(TokenType::Punctuation) && peek().lexeme == "{") {
        return parseCompoundStatement();
    }

    // General statement parsing is deferred to future days.
    // Unrecognized or unsupported statement token.
    error("Expected statement, found '" + peek().lexeme + "'");
    return nullptr;
}

// =============================================================================
// parseCompoundStatement() — Day 28 Compound Statements
// =============================================================================
//
// Grammar:
//   compound-statement:
//       '{' statement-list? '}'
//
//   statement-list:
//       statement*
//
// Recognizes '{' as the start of a block, parses child statements until matching '}',
// and reports a clear diagnostic if '}' is missing or malformed.
//
std::unique_ptr<CompoundStmt> Parser::parseCompoundStatement() {
    if (isAtEnd() || !check(TokenType::Punctuation) || peek().lexeme != "{") {
        if (!hasError()) {
            error("Expected '{' to start compound statement");
        }
        return nullptr;
    }

    Token openBrace = advance(); // consume '{'
    SourceLocation loc{openBrace.line, openBrace.column};
    auto compound = std::make_unique<CompoundStmt>(loc);

    while (!isAtEnd() && !(check(TokenType::Punctuation) && peek().lexeme == "}")) {
        auto stmt = parseStatement();
        if (!stmt) {
            // parseStatement encountered an error or invalid token;
            // return nullptr cleanly with diagnostic preserved.
            return nullptr;
        }
        compound->addStatement(std::move(stmt));
    }

    if (isAtEnd()) {
        if (!hasError()) {
            error("Expected '}' to close compound statement");
        }
        return nullptr;
    }

    advance(); // consume '}'
    return compound;
}

// =============================================================================
// parseIfStatement() — Day 29 & Day 30 If / Else Statement
// =============================================================================
//
// Grammar:
//   selection-statement:
//       'if' '(' expression ')' statement
//       'if' '(' expression ')' statement 'else' statement
//
// Dangling-else resolution:
//   The 'else' keyword binds greedily to the nearest unmatched 'if'.
//   Because thenBranch is parsed recursively via parseStatement() before checking
//   for 'else' at this level, any 'else' following an inner unbraced 'if' is
//   consumed by that inner parseIfStatement() invocation, ensuring correct C binding:
//     if (a) if (b) s1 else s2  ->  if (a) { if (b) s1 else s2 }
//
std::unique_ptr<IfStmt> Parser::parseIfStatement() {
    if (isAtEnd() || !check(TokenType::Keyword) || peek().lexeme != "if") {
        if (!hasError()) {
            error("Expected 'if'");
        }
        return nullptr;
    }

    Token ifToken = advance(); // consume 'if'
    SourceLocation loc{ifToken.line, ifToken.column};

    if (isAtEnd() || !check(TokenType::Punctuation) || peek().lexeme != "(") {
        if (!hasError()) {
            error("Expected '(' after 'if'");
        }
        return nullptr;
    }
    advance(); // consume '('

    std::unique_ptr<Expression> condition = parseExpression();
    if (!condition) {
        if (!hasError()) {
            error("Expected expression in 'if' condition");
        }
        return nullptr;
    }

    if (isAtEnd() || !check(TokenType::Punctuation) || peek().lexeme != ")") {
        if (!hasError()) {
            error("Expected ')' after 'if' condition");
        }
        return nullptr;
    }
    advance(); // consume ')'

    std::unique_ptr<Statement> thenBranch = parseStatement();
    if (!thenBranch) {
        if (!hasError()) {
            error("Expected statement after 'if' condition");
        }
        return nullptr;
    }

    std::unique_ptr<Statement> elseBranch = nullptr;
    if (!isAtEnd() && check(TokenType::Keyword) && peek().lexeme == "else") {
        advance(); // consume 'else'
        elseBranch = parseStatement();
        if (!elseBranch) {
            if (!hasError()) {
                error("Expected statement after 'else'");
            }
            return nullptr;
        }
    }

    return std::make_unique<IfStmt>(
        std::move(condition), std::move(thenBranch), std::move(elseBranch), loc);
}

// =============================================================================
// parseWhileStatement() — Day 31 While Loop
// =============================================================================
//
// Grammar:
//   iteration-statement:
//       'while' '(' expression ')' statement
//
// Source location records the location of the 'while' keyword.
//
std::unique_ptr<WhileStmt> Parser::parseWhileStatement() {
    if (isAtEnd() || !check(TokenType::Keyword) || peek().lexeme != "while") {
        if (!hasError()) {
            error("Expected 'while'");
        }
        return nullptr;
    }

    Token whileToken = advance(); // consume 'while'
    SourceLocation loc{whileToken.line, whileToken.column};

    if (isAtEnd() || !check(TokenType::Punctuation) || peek().lexeme != "(") {
        if (!hasError()) {
            error("Expected '(' after 'while'");
        }
        return nullptr;
    }
    advance(); // consume '('

    std::unique_ptr<Expression> condition = parseExpression();
    if (!condition) {
        if (!hasError()) {
            error("Expected expression in 'while' condition");
        }
        return nullptr;
    }

    if (isAtEnd() || !check(TokenType::Punctuation) || peek().lexeme != ")") {
        if (!hasError()) {
            error("Expected ')' after 'while' condition");
        }
        return nullptr;
    }
    advance(); // consume ')'

    std::unique_ptr<Statement> body = parseStatement();
    if (!body) {
        if (!hasError()) {
            error("Expected statement after 'while' condition");
        }
        return nullptr;
    }

    return std::make_unique<WhileStmt>(std::move(condition), std::move(body), loc);
}

// =============================================================================
// parseDoWhileStatement() — Day 32 Do-While Loop
// =============================================================================
//
// Grammar:
//   iteration-statement:
//       'do' statement 'while' '(' expression ')' ';'
//
// Source location records the location of the 'do' keyword.
//
std::unique_ptr<DoWhileStmt> Parser::parseDoWhileStatement() {
    if (isAtEnd() || !check(TokenType::Keyword) || peek().lexeme != "do") {
        if (!hasError()) {
            error("Expected 'do'");
        }
        return nullptr;
    }

    Token doToken = advance(); // consume 'do'
    SourceLocation loc{doToken.line, doToken.column};

    std::unique_ptr<Statement> body = parseStatement();
    if (!body) {
        if (!hasError()) {
            error("Expected statement in 'do-while' loop body");
        }
        return nullptr;
    }

    if (isAtEnd() || !check(TokenType::Keyword) || peek().lexeme != "while") {
        if (!hasError()) {
            error("Expected 'while' after 'do' body");
        }
        return nullptr;
    }
    advance(); // consume 'while'

    if (isAtEnd() || !check(TokenType::Punctuation) || peek().lexeme != "(") {
        if (!hasError()) {
            error("Expected '(' after 'while' in do-while loop");
        }
        return nullptr;
    }
    advance(); // consume '('

    std::unique_ptr<Expression> condition = parseExpression();
    if (!condition) {
        if (!hasError()) {
            error("Expected expression in 'do-while' condition");
        }
        return nullptr;
    }

    if (isAtEnd() || !check(TokenType::Punctuation) || peek().lexeme != ")") {
        if (!hasError()) {
            error("Expected ')' after 'do-while' condition");
        }
        return nullptr;
    }
    advance(); // consume ')'

    if (isAtEnd() || !check(TokenType::Punctuation) || peek().lexeme != ";") {
        if (!hasError()) {
            error("Expected ';' after 'do-while' statement");
        }
        return nullptr;
    }
    advance(); // consume ';'

    return std::make_unique<DoWhileStmt>(std::move(body), std::move(condition), loc);
}

// =============================================================================
// parseForStatement() — Day 33 For Loop
// =============================================================================
//
// Grammar:
//   iteration-statement:
//       'for' '(' expression? ';' expression? ';' expression? ')' statement
//
// Source location records the location of the 'for' keyword.
// Omitted clauses are represented by nullptr.
//
std::unique_ptr<ForStmt> Parser::parseForStatement() {
    if (isAtEnd() || !check(TokenType::Keyword) || peek().lexeme != "for") {
        if (!hasError()) {
            error("Expected 'for'");
        }
        return nullptr;
    }

    Token forToken = advance(); // consume 'for'
    SourceLocation loc{forToken.line, forToken.column};

    if (isAtEnd() || !check(TokenType::Punctuation) || peek().lexeme != "(") {
        if (!hasError()) {
            error("Expected '(' after 'for'");
        }
        return nullptr;
    }
    advance(); // consume '('

    // 1. Initializer expression (optional)
    std::unique_ptr<Expression> initializer = nullptr;
    if (isAtEnd()) {
        if (!hasError()) {
            error("Expected ';' after 'for' initializer");
        }
        return nullptr;
    }
    if (!check(TokenType::Punctuation) || peek().lexeme != ";") {
        initializer = parseExpression();
        if (!initializer) {
            return nullptr;
        }
    }
    if (isAtEnd() || !check(TokenType::Punctuation) || peek().lexeme != ";") {
        if (!hasError()) {
            error("Expected ';' after 'for' initializer");
        }
        return nullptr;
    }
    advance(); // consume ';'

    // 2. Condition expression (optional)
    std::unique_ptr<Expression> condition = nullptr;
    if (isAtEnd()) {
        if (!hasError()) {
            error("Expected ';' after 'for' condition");
        }
        return nullptr;
    }
    if (!check(TokenType::Punctuation) || peek().lexeme != ";") {
        condition = parseExpression();
        if (!condition) {
            return nullptr;
        }
    }
    if (isAtEnd() || !check(TokenType::Punctuation) || peek().lexeme != ";") {
        if (!hasError()) {
            error("Expected ';' after 'for' condition");
        }
        return nullptr;
    }
    advance(); // consume ';'

    // 3. Post expression (optional)
    std::unique_ptr<Expression> post = nullptr;
    if (isAtEnd()) {
        if (!hasError()) {
            error("Expected ')' after 'for' clauses");
        }
        return nullptr;
    }
    if (!check(TokenType::Punctuation) || peek().lexeme != ")") {
        post = parseExpression();
        if (!post) {
            return nullptr;
        }
    }
    if (isAtEnd() || !check(TokenType::Punctuation) || peek().lexeme != ")") {
        if (!hasError()) {
            error("Expected ')' after 'for' clauses");
        }
        return nullptr;
    }
    advance(); // consume ')'

    // 4. Body statement
    std::unique_ptr<Statement> body = parseStatement();
    if (!body) {
        if (!hasError()) {
            error("Expected statement in 'for' loop body");
        }
        return nullptr;
    }

    return std::make_unique<ForStmt>(
        std::move(initializer), std::move(condition), std::move(post), std::move(body), loc);
}





