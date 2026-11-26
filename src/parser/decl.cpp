#include "parser/parser.h"
#include "ast/function.h"
#include "ast/variable_declaration.h"
#include "ast/var_decl.h"
#include "ast/program.h"
#include "ast/storage_class.h"
#include "types/type.h"

#include <cstdlib>
#include <vector>

// =============================================================================
// Helper Functions for Declaration Parsing (Day 18, 36, 38, 42)
// =============================================================================

static bool isTypeSpecifier(const Token& tok) {
    return isPrimitiveTypeToken(tok);
}

static bool isStorageClassSpecifier(const Token& tok) {
    if (tok.type != TokenType::Keyword) return false;
    return tok.lexeme == "static" || tok.lexeme == "extern" || tok.lexeme == "register";
}

static bool isTypeQualifier(const Token& tok) {
    if (tok.type != TokenType::Keyword) return false;
    return tok.lexeme == "const" || tok.lexeme == "volatile";
}

static bool isDeclarationSpecifier(const Token& tok) {
    return isTypeSpecifier(tok) || isTypeQualifier(tok) || isStorageClassSpecifier(tok);
}

bool Parser::isDeclarationSpecifier(const Token& tok) const {
    return ::isDeclarationSpecifier(tok);
}

struct DeclSpecifiers {
    StorageClass storageClass{StorageClass::None};
    std::string storageClassToken;
    bool isConst{false};
    bool isVolatile{false};
    TypeKind typeKind{TypeKind::Unknown};
    SourceLocation loc;
    bool hasLoc{false};
    bool hasType{false};
    Token lastSpecifierTok;
};

static bool parseDeclarationSpecifiers(Parser& parser, DeclSpecifiers& specs,
                                       bool allowStorageClass = true,
                                       bool allowRegisterOnly = false) {
    while (!parser.isAtEnd()) {
        const Token& tok = parser.peek();
        if (tok.type != TokenType::Keyword) {
            break;
        }

        // 1. Storage-class specifiers (Day 42)
        if (tok.lexeme == "static" || tok.lexeme == "extern" || tok.lexeme == "register") {
            if (!allowStorageClass) {
                parser.error("Storage class specifier not allowed here");
                return false;
            }
            if (allowRegisterOnly && tok.lexeme != "register") {
                parser.error("Parameter cannot have '" + tok.lexeme + "' storage class");
                return false;
            }

            StorageClass sc = StorageClass::None;
            if (tok.lexeme == "static") sc = StorageClass::Static;
            else if (tok.lexeme == "extern") sc = StorageClass::Extern;
            else if (tok.lexeme == "register") sc = StorageClass::Register;

            if (specs.storageClass != StorageClass::None) {
                if (specs.storageClass == sc) {
                    parser.error("Duplicate storage class specifier '" + tok.lexeme + "'");
                } else {
                    parser.error("Multiple storage class specifiers in declaration");
                }
                return false;
            }

            Token scTok = parser.advance();
            if (!specs.hasLoc) {
                specs.loc = parser.makeLocation(scTok);
                specs.hasLoc = true;
            }
            specs.storageClass = sc;
            specs.storageClassToken = scTok.lexeme;
            specs.lastSpecifierTok = scTok;
            continue;
        }

        // 2. Type qualifiers (Day 38 const, Day 42 volatile)
        if (tok.lexeme == "const") {
            Token constTok = parser.advance();
            if (!specs.hasLoc) {
                specs.loc = parser.makeLocation(constTok);
                specs.hasLoc = true;
            }
            specs.isConst = true;
            specs.lastSpecifierTok = constTok;
            continue;
        }

        if (tok.lexeme == "volatile") {
            Token volTok = parser.advance();
            if (!specs.hasLoc) {
                specs.loc = parser.makeLocation(volTok);
                specs.hasLoc = true;
            }
            specs.isVolatile = true;
            specs.lastSpecifierTok = volTok;
            continue;
        }

        // 3. Type specifier
        if (isPrimitiveTypeToken(tok)) {
            if (specs.hasType) {
                parser.error("Cannot combine with previous type specifier '" + typeKindToString(specs.typeKind) + "'");
                return false;
            }
            Token typeTok = parser.advance();
            if (!specs.hasLoc) {
                specs.loc = parser.makeLocation(typeTok);
                specs.hasLoc = true;
            }
            specs.hasType = true;
            specs.typeKind = typeKindFromToken(typeTok);
            specs.lastSpecifierTok = typeTok;
            continue;
        }

        // Other keywords (if, while, return, etc.) break declaration specifiers
        break;
    }

    if (!specs.hasType) {
        if (specs.isConst) {
            parser.error("Expected type specifier after 'const'");
        } else if (specs.isVolatile) {
            parser.error("Expected type specifier after 'volatile'");
        } else if (specs.storageClass != StorageClass::None) {
            parser.error("Expected type specifier after '" + specs.storageClassToken + "'");
        } else {
            parser.error("Expected type specifier");
        }
        return false;
    }

    return true;
}

// =============================================================================
// Parser::parseFunctionDeclaration()
// =============================================================================
//
// Grammar:
//   function-declaration:
//       storage-class-specifier? type-qualifier* type-specifier identifier '(' parameter-list? ')' ';'
//       storage-class-specifier? type-qualifier* type-specifier identifier '(' parameter-list? ')' '{' ... '}'
//
//   parameter-list:
//       'void'
//       parameter ( ',' parameter )*
//
//   parameter:
//       'register'? type-qualifier* type-specifier identifier?
//
std::unique_ptr<Function> Parser::parseFunctionDeclaration() {
    if (isAtEnd()) {
        return nullptr;
    }

    if (!isDeclarationSpecifier(peek())) {
        return nullptr;
    }

    // Lookahead scan: count declaration specifier tokens to find the identifier and '('
    std::size_t offset = 0;
    while (isDeclarationSpecifier(peekAt(offset))) {
        offset++;
    }

    if (offset == 0) {
        return nullptr;
    }

    // Special check for error cases where identifier was expected:
    // e.g. 'int 123();' or 'void ();'
    if (peekAt(offset).type != TokenType::Identifier) {
        if (peekAt(offset).type == TokenType::Punctuation && peekAt(offset).lexeme == "(") {
            Token typeTok = advance();
            error("Expected identifier after '" + typeTok.lexeme + "'");
            return nullptr;
        }
        if (peekAt(offset + 1).type == TokenType::Punctuation && peekAt(offset + 1).lexeme == "(") {
            Token typeTok = advance();
            error("Expected identifier after '" + typeTok.lexeme + "'");
            return nullptr;
        }
        // It's a variable declaration or other statement (e.g. 'int x;', 'const int x = 10;').
        // Return nullptr without consuming any tokens.
        return nullptr;
    }

    // Check if token after identifier is '('
    if (peekAt(offset + 1).type != TokenType::Punctuation || peekAt(offset + 1).lexeme != "(") {
        // Not a function declaration!
        return nullptr;
    }

    // It is a function declaration: parse declaration specifiers
    DeclSpecifiers specs;
    if (!parseDeclarationSpecifiers(*this, specs, /*allowStorageClass=*/true, /*allowRegisterOnly=*/false)) {
        return nullptr;
    }

    if (specs.storageClass == StorageClass::Register) {
        error("Function declaration cannot have 'register' storage class");
        return nullptr;
    }

    Token nameTok = advance();
    std::string name = nameTok.lexeme;

    auto returnTypeObj = getPrimitiveType(specs.typeKind, specs.isConst, specs.isVolatile);
    auto function = std::make_unique<Function>(std::move(name), std::move(returnTypeObj), specs.loc, false,
                                               ParameterListKind::Unspecified, specs.storageClass);

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

            if (!isDeclarationSpecifier(peek())) {
                if (peek().type == TokenType::Identifier) {
                    error("Expected parameter type before '" + peek().lexeme + "'");
                } else {
                    error("Expected parameter type");
                }
                return nullptr;
            }

            DeclSpecifiers paramSpecs;
            if (!parseDeclarationSpecifiers(*this, paramSpecs, /*allowStorageClass=*/true, /*allowRegisterOnly=*/true)) {
                return nullptr;
            }

            if (paramSpecs.typeKind == TypeKind::Void) {
                error("Parameter cannot have 'void' type");
                return nullptr;
            }

            auto typeObj = getPrimitiveType(paramSpecs.typeKind, paramSpecs.isConst, paramSpecs.isVolatile);

            // Parameter name: optional in prototypes, required in definitions
            std::string paramName;
            if (!isAtEnd() && peek().type == TokenType::Identifier) {
                Token paramNameTok = advance();
                paramName = paramNameTok.lexeme;
            }

            auto param = std::make_unique<VariableDeclaration>(
                std::move(typeObj), std::move(paramName), nullptr, paramSpecs.loc, paramSpecs.storageClass);
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
// Parser::parseVarDecl() — Day 37, Day 38, Day 42
// =============================================================================
//
// Grammar:
//   variable-declaration:
//       storage-class-specifier? type-qualifier* type-specifier type-qualifier* identifier ( '=' expression )? ';'
//
std::unique_ptr<VarDecl> Parser::parseVarDecl() {
    if (isAtEnd()) {
        return nullptr;
    }

    if (!isDeclarationSpecifier(peek())) {
        error("Expected type specifier");
        return nullptr;
    }

    DeclSpecifiers specs;
    if (!parseDeclarationSpecifiers(*this, specs, /*allowStorageClass=*/true, /*allowRegisterOnly=*/false)) {
        return nullptr;
    }

    auto type = getPrimitiveType(specs.typeKind, specs.isConst, specs.isVolatile);

    if (isAtEnd() || peek().type != TokenType::Identifier) {
        error("Expected identifier after type specifier");
        return nullptr;
    }

    Token nameTok = advance();
    std::string name = nameTok.lexeme;

    // Array declarator: one or more consecutive '[' size ']' dimensions (Day 54 / Day 56)
    if (!isAtEnd() && check(TokenType::Punctuation) && peek().lexeme == "[") {
        // Collect all dimension sizes in declaration order: int a[D0][D1][D2]
        std::vector<std::size_t> dims;

        while (!isAtEnd() && check(TokenType::Punctuation) && peek().lexeme == "[") {
            advance(); // consume '['

            if (isAtEnd()) {
                error("Expected array size before end of input");
                return nullptr;
            }

            // Empty array size: '[]'
            if (check(TokenType::Punctuation) && peek().lexeme == "]") {
                error("Expected array size between '[' and ']'");
                return nullptr;
            }

            // Negative array size: '[-5]'
            if (check(TokenType::Operator) && peek().lexeme == "-") {
                advance(); // consume '-'
                error("Array size must be positive");
                return nullptr;
            }

            // Identifier / variable-length array: '[size]'
            if (check(TokenType::Identifier)) {
                error("Variable-length arrays are not supported");
                return nullptr;
            }

            // Must be an integer literal
            if (!check(TokenType::Integer)) {
                error("Expected integer constant for array size");
                return nullptr;
            }

            Token sizeTok = advance();
            char* endPtr = nullptr;
            unsigned long long parsedSize = std::strtoull(sizeTok.lexeme.c_str(), &endPtr, 0);
            if (parsedSize == 0) {
                error("Array size must be greater than zero");
                return nullptr;
            }

            // Expect closing ']'
            if (isAtEnd() || !check(TokenType::Punctuation) || peek().lexeme != "]") {
                error("Expected ']' after array size");
                return nullptr;
            }
            advance(); // consume ']'

            dims.push_back(static_cast<std::size_t>(parsedSize));
        }

        // Construct nested ArrayType from innermost dimension outward so that
        // the outermost dimension (dims[0]) becomes the outermost ArrayType.
        // For int a[3][4]: dims = {3, 4}
        //   Step 1: ArrayType(int,   4)         <- innermost
        //   Step 2: ArrayType(above, 3)         <- outermost
        // Result: ArrayType(size=3, elem=ArrayType(size=4, elem=int))
        for (auto it = dims.rbegin(); it != dims.rend(); ++it) {
            type = std::make_shared<ArrayType>(std::move(type), *it);
        }
    }

    std::unique_ptr<Expression> initializer = nullptr;
    if (!isAtEnd() && check(TokenType::Operator) && peek().lexeme == "=") {
        if (type && type->isArray()) {
            error("Array initialization is not supported");
            return nullptr;
        }
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

    return std::make_unique<VarDecl>(std::move(type), std::move(name), std::move(initializer), specs.loc, specs.storageClass);
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

