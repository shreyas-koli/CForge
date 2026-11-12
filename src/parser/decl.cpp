#include "parser/parser.h"
#include "ast/function.h"
#include "ast/variable_declaration.h"
#include "ast/var_decl.h"
#include "ast/program.h"
#include "types/type.h"

// =============================================================================
// Helper Functions for Declaration Parsing (Day 18 & Day 36)
// =============================================================================

static bool isTypeSpecifier(const Token& tok) {
    return isPrimitiveTypeToken(tok);
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
    // Case 1: Empty parameter list '()' — C17 unspecified parameters
    if (check(TokenType::Punctuation) && peek().lexeme == ")") {
        advance(); // consume ')'
        function->setParameterListKind(ParameterListKind::Unspecified);
    }
    // Case 2: Explicit '(void)' no-parameter list — C17 explicit zero parameters
    else if (check(TokenType::Keyword) && peek().lexeme == "void" &&
             peekNext().type == TokenType::Punctuation && peekNext().lexeme == ")") {
        advance(); // consume 'void'
        advance(); // consume ')'
        function->setParameterListKind(ParameterListKind::Void);
    }
    // Case 3: Parameter declarations
    else {
        function->setParameterListKind(ParameterListKind::Typed);

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

            bool isConst = false;
            SourceLocation paramLoc;
            bool hasParamLoc = false;

            // Check for leading 'const' qualifier (Day 38 & Day 40)
            while (!isAtEnd() && check(TokenType::Keyword) && peek().lexeme == "const") {
                Token constTok = advance();
                if (!hasParamLoc) {
                    paramLoc = {constTok.line, constTok.column};
                    hasParamLoc = true;
                }
                isConst = true;
            }

            // Parameter type
            if (isAtEnd() || !isTypeSpecifier(peek())) {
                if (isConst) {
                    error("Expected type specifier after 'const'");
                } else if (!isAtEnd() && peek().type == TokenType::Identifier) {
                    error("Expected parameter type before '" + peek().lexeme + "'");
                } else {
                    error("Expected parameter type");
                }
                return nullptr;
            }

            Token paramTypeTok = advance();
            if (!hasParamLoc) {
                paramLoc = {paramTypeTok.line, paramTypeTok.column};
                hasParamLoc = true;
            }

            Token lastSpecifierTok = paramTypeTok;

            // Support trailing 'const' qualifier (e.g. 'int const x' or 'int const')
            while (!isAtEnd() && check(TokenType::Keyword) && peek().lexeme == "const") {
                lastSpecifierTok = advance();
                isConst = true;
            }

            TypeKind kind = typeKindFromToken(paramTypeTok);
            if (kind == TypeKind::Void) {
                error("Parameter cannot have 'void' type");
                return nullptr;
            }

            auto typeObj = getPrimitiveType(kind, isConst);

            // Parameter name: optional in prototypes, required in definitions
            std::string paramName;
            if (!isAtEnd() && peek().type == TokenType::Identifier) {
                Token paramNameTok = advance();
                paramName = paramNameTok.lexeme;
            }

            auto param = std::make_unique<VariableDeclaration>(
                std::move(typeObj), std::move(paramName), nullptr, paramLoc);
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
        // Function definitions require named parameters
        for (std::size_t i = 0; i < function->parameterCount(); ++i) {
            const auto* param = function->parameter(i);
            if (param && !param->hasName()) {
                error("Function definition requires parameter names");
                return nullptr;
            }
        }

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
// Parser::parseVarDecl() — Day 37 & Day 38
// =============================================================================
//
// Grammar:
//   variable-declaration:
//       'const'? type-specifier 'const'? identifier ( '=' expression )? ';'
//
std::unique_ptr<VarDecl> Parser::parseVarDecl() {
    if (isAtEnd()) {
        return nullptr;
    }

    bool isConst = false;
    SourceLocation loc;
    bool hasLoc = false;

    // Check for leading 'const' qualifier (Day 38)
    while (!isAtEnd() && check(TokenType::Keyword) && peek().lexeme == "const") {
        Token constTok = advance();
        if (!hasLoc) {
            loc = {constTok.line, constTok.column};
            hasLoc = true;
        }
        isConst = true;
    }

    if (isAtEnd() || !isPrimitiveTypeToken(peek())) {
        if (isConst) {
            error("Expected type specifier after 'const'");
        } else {
            error("Expected type specifier");
        }
        return nullptr;
    }

    Token typeTok = advance();
    if (!hasLoc) {
        loc = {typeTok.line, typeTok.column};
        hasLoc = true;
    }

    // Support trailing 'const' qualifier (e.g. 'int const x;')
    while (!isAtEnd() && check(TokenType::Keyword) && peek().lexeme == "const") {
        advance();
        isConst = true;
    }

    auto type = getPrimitiveType(typeKindFromToken(typeTok), isConst);

    if (isAtEnd() || peek().type != TokenType::Identifier) {
        error("Expected identifier after type specifier");
        return nullptr;
    }

    Token nameTok = advance();
    std::string name = nameTok.lexeme;

    std::unique_ptr<Expression> initializer = nullptr;
    if (!isAtEnd() && check(TokenType::Operator) && peek().lexeme == "=") {
        advance(); // consume '='

        if (isAtEnd() || (check(TokenType::Punctuation) && peek().lexeme == ";")) {
            error("Expected expression after '='");
            return nullptr;
        }

        initializer = parseExpression();
        if (!initializer) {
            if (!hasError()) {
                error("Expected expression after '='");
            }
            return nullptr;
        }
    }

    if (isAtEnd() || !check(TokenType::Punctuation) || peek().lexeme != ";") {
        error("Expected ';' after variable declaration");
        return nullptr;
    }
    advance(); // consume ';'

    return std::make_unique<VarDecl>(std::move(type), std::move(name), std::move(initializer), loc);
}

std::unique_ptr<VarDecl> Parser::parseVariableDeclaration() {
    return parseVarDecl();
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

