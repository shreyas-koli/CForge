#include "parser/parser.h"

Parser::Parser(std::vector<Token> tokens)
    : m_tokens(std::move(tokens))
    , m_current(0) {}

Parser::Parser(Lexer& lexer)
    : m_tokens(lexer.tokenize())
    , m_current(0) {}

bool Parser::isAtEnd() const {
    if (m_current >= m_tokens.size()) {
        return true;
    }
    return m_tokens[m_current].type == TokenType::Eof;
}

const Token& Parser::peek() const {
    if (m_current >= m_tokens.size()) {
        static const Token eofToken{TokenType::Eof, "", 0, 0};
        return eofToken;
    }
    return m_tokens[m_current];
}

const Token& Parser::previous() const {
    if (m_current == 0) {
        static const Token eofToken{TokenType::Eof, "", 0, 0};
        return eofToken;
    }
    return m_tokens[m_current - 1];
}

Token Parser::advance() {
    if (!isAtEnd()) {
        m_current++;
    }
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) {
        return type == TokenType::Eof;
    }
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}
