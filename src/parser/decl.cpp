#include "parser/parser.h"
#include "ast/function.h"
#include "ast/variable_declaration.h"
#include "ast/program.h"

// =============================================================================
// Helper Functions for Declaration Parsing (Day 18)
// =============================================================================

static bool isTypeSpecifier(const Token& tok) {
    if (tok.type == TokenType::Keyword) {
        return tok.lexeme == "int" || tok.lexeme == "char" ||
               tok.lexeme == "float" || tok.lexeme == "double" ||
               tok.lexeme == "void";
    }
    return false;
}

// =============================================================================
// Parser::parseFunctionDeclaration()
// =============================================================================
//
// Grammar:
//   function-declaration:
//       type-specifier identifier '(' parameter-list? ')' ';'
//       type-specifier identifier '(' parameter-list? ')' '{' ... '}'
//
//   parameter-list:
//       'void'
//       parameter ( ',' parameter )*
//
//   parameter:
//       type-specifier identifier
//
std::unique_ptr<Function> Parser::parseFunctionDeclaration() {
    if (isAtEnd()) {
        return nullptr;
    }

    if (!isTypeSpecifier(peek())) {
        return nullptr;
    }

    // Check if next token is an identifier
    if (peekAt(1).type != TokenType::Identifier) {
        Token typeTok = advance();
        error("Expected identifier after '" + typeTok.lexeme + "'");
        return nullptr;
    }

    // Check if followed by '(' -> function declaration
    // If not followed by '(', it is a variable declaration (e.g. 'int x = 10;' or 'int x;').
    // Do not confuse function declaration with variable declaration: return nullptr cleanly.
    if (peekAt(2).type != TokenType::Punctuation || peekAt(2).lexeme != "(") {
        return nullptr;
    }

    // Consume return type and function name
    Token returnTypeTok = advance();
    SourceLocation loc{returnTypeTok.line, returnTypeTok.column};
    std::string returnType = returnTypeTok.lexeme;

    Token nameTok = advance();
    std::string name = nameTok.lexeme;

    auto function = std::make_unique<Function>(std::move(name), std::move(returnType), loc);

    // Consume '('
    advance();

    // Parse parameter list
    // Case 1: Empty parameter list '()'
    if (check(TokenType::Punctuation) && peek().lexeme == ")") {
        advance(); // consume ')'
    }
    // Case 2: Explicit '(void)' no-parameter list
    else if (check(TokenType::Keyword) && peek().lexeme == "void" &&
             peekNext().type == TokenType::Punctuation && peekNext().lexeme == ")") {
        advance(); // consume 'void'
        advance(); // consume ')'
    }
    // Case 3: Parameter declarations
    else {
        // Disallow 'void' with comma or as named parameter (e.g. 'void, ...' or 'void x')
        if (check(TokenType::Keyword) && peek().lexeme == "void") {
            advance(); // consume 'void'
            if (check(TokenType::Punctuation) && peek().lexeme == ",") {
                error("Expected ')' after 'void' in parameter list");
            } else {
                error("Parameter cannot have 'void' type");
            }
            return nullptr;
        }

        while (true) {
            if (isAtEnd()) {
                error("Expected ')' after parameter list");
                return nullptr;
            }

            // Parameter type
            if (!isTypeSpecifier(peek())) {
                if (peek().type == TokenType::Identifier) {
                    error("Expected parameter type before '" + peek().lexeme + "'");
                } else {
                    error("Expected parameter type");
                }
                return nullptr;
            }
            Token paramTypeTok = advance();
            std::string paramType = paramTypeTok.lexeme;
            SourceLocation paramLoc{paramTypeTok.line, paramTypeTok.column};

            if (paramType == "void") {
                error("Parameter cannot have 'void' type");
                return nullptr;
            }

            // Parameter name
            if (isAtEnd() || peek().type != TokenType::Identifier) {
                error("Expected parameter name after '" + paramType + "'");
                return nullptr;
            }
            Token paramNameTok = advance();
            std::string paramName = paramNameTok.lexeme;

            auto param = std::make_unique<VariableDeclaration>(
                std::move(paramName), std::move(paramType), nullptr, paramLoc);
            function->addParameter(std::move(param));

            if (isAtEnd()) {
                error("Expected ')' after parameter list");
                return nullptr;
            }

            if (peek().type == TokenType::Punctuation && peek().lexeme == ",") {
                advance(); // consume ','
                if (peek().type == TokenType::Punctuation && peek().lexeme == ")") {
                    error("Expected parameter declaration after ','");
                    return nullptr;
                }
                continue;
            } else if (peek().type == TokenType::Punctuation && peek().lexeme == ")") {
                advance(); // consume ')'
                break;
            } else {
                error("Expected ',' or ')' after parameter declaration");
                return nullptr;
            }
        }
    }

    // Now check for prototype ';' or definition '{ ... }'
    if (isAtEnd()) {
        error("Expected ';' or '{' after function declarator");
        return nullptr;
    }

    if (peek().type == TokenType::Punctuation && peek().lexeme == ";") {
        advance(); // consume ';'
        function->setIsDefinition(false);
        return function;
    }

    if (peek().type == TokenType::Punctuation && peek().lexeme == "{") {
        function->setIsDefinition(true);
        advance(); // consume opening '{'

        // Safely consume function body via balanced-brace tracking.
        // Full statement parsing inside function bodies is deferred to future days;
        // currently the body statements vector is left empty as a documented stub.
        int braceDepth = 1;
        while (braceDepth > 0 && !isAtEnd()) {
            const Token& tok = peek();
            if (tok.type == TokenType::Punctuation) {
                if (tok.lexeme == "{") {
                    braceDepth++;
                } else if (tok.lexeme == "}") {
                    braceDepth--;
                    if (braceDepth == 0) {
                        advance(); // consume closing '}'
                        break;
                    }
                }
            }
            advance();
        }

        if (braceDepth > 0) {
            error("Expected '}' to close function body");
            return nullptr;
        }

        return function;
    }

    error("Expected ';' or '{' after function declarator, found '" + peek().lexeme + "'");
    return nullptr;
}

// =============================================================================
// Parser::parseDeclaration()
// =============================================================================
std::unique_ptr<Function> Parser::parseDeclaration() {
    return parseFunctionDeclaration();
}

// =============================================================================
// Parser::parseProgram()
// =============================================================================
std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();
    while (!isAtEnd()) {
        auto decl = parseDeclaration();
        if (!decl) {
            if (!hasError()) {
                error("Unexpected token in translation unit: '" + peek().lexeme + "'");
            }
            return nullptr;
        }
        program->addStatement(std::move(decl));
    }
    return program;
}

