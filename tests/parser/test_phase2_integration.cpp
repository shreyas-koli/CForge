#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/printer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

// Helper: construct a Parser for a source string
static Parser makeParser(const std::string& src) {
    Lexer lexer(src);
    return Parser(lexer.tokenize());
}

// =============================================================================
// PHASE 2 INTEGRATION & COMPLETE GRAMMAR TESTS
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Comprehensive Expression AST Structures
// -----------------------------------------------------------------------------
TEST_CASE("Phase 2 Integration: full arithmetic precedence and associativity", "[parser][phase2][expr]") {
    // a + b * c - d / e
    // Expected AST:
    // BinaryExpr(-)
    //   Left: BinaryExpr(+)
    //     Left: a
    //     Right: BinaryExpr(*)
    //       Left: b
    //       Right: c
    //   Right: BinaryExpr(/)
    //     Left: d
    //     Right: e
    Parser parser = makeParser("a + b * c - d / e");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());
    CHECK(parser.isAtEnd());

    auto* minusNode = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(minusNode != nullptr);
    CHECK(minusNode->op() == "-");

    auto* leftPlus = dynamic_cast<const BinaryExpr*>(minusNode->left());
    REQUIRE(leftPlus != nullptr);
    CHECK(leftPlus->op() == "+");

    auto* aId = dynamic_cast<const IdentifierExpression*>(leftPlus->left());
    REQUIRE(aId != nullptr);
    CHECK(aId->name() == "a");

    auto* bTimesC = dynamic_cast<const BinaryExpr*>(leftPlus->right());
    REQUIRE(bTimesC != nullptr);
    CHECK(bTimesC->op() == "*");

    auto* rightDiv = dynamic_cast<const BinaryExpr*>(minusNode->right());
    REQUIRE(rightDiv != nullptr);
    CHECK(rightDiv->op() == "/");

    auto* dId = dynamic_cast<const IdentifierExpression*>(rightDiv->left());
    REQUIRE(dId != nullptr);
    CHECK(dId->name() == "d");

    auto* eId = dynamic_cast<const IdentifierExpression*>(rightDiv->right());
    REQUIRE(eId != nullptr);
    CHECK(eId->name() == "e");
}

TEST_CASE("Phase 2 Integration: mixed unary, binary, and parentheses", "[parser][phase2][expr]") {
    // -a * (b + !c)
    Parser parser = makeParser("-a * (b + !c)");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());
    CHECK(parser.isAtEnd());

    auto* mult = dynamic_cast<BinaryExpr*>(expr.get());
    REQUIRE(mult != nullptr);
    CHECK(mult->op() == "*");

    auto* unMinus = dynamic_cast<const UnaryExpr*>(mult->left());
    REQUIRE(unMinus != nullptr);
    CHECK(unMinus->op() == "-");

    auto* plusNode = dynamic_cast<const BinaryExpr*>(mult->right());
    REQUIRE(plusNode != nullptr);
    CHECK(plusNode->op() == "+");

    auto* bId = dynamic_cast<const IdentifierExpression*>(plusNode->left());
    REQUIRE(bId != nullptr);
    CHECK(bId->name() == "b");

    auto* notC = dynamic_cast<const UnaryExpr*>(plusNode->right());
    REQUIRE(notC != nullptr);
    CHECK(notC->op() == "!");
}

TEST_CASE("Phase 2 Integration: complex assignment and arithmetic chaining", "[parser][phase2][expr]") {
    // total += x = a * 2 + 1
    Parser parser = makeParser("total += x = a * 2 + 1");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());
    CHECK(parser.isAtEnd());

    auto* plusEq = dynamic_cast<AssignExpr*>(expr.get());
    REQUIRE(plusEq != nullptr);
    CHECK(plusEq->op() == "+=");

    auto* totalId = dynamic_cast<const IdentifierExpression*>(plusEq->left());
    REQUIRE(totalId != nullptr);
    CHECK(totalId->name() == "total");

    auto* eqNode = dynamic_cast<const AssignExpr*>(plusEq->right());
    REQUIRE(eqNode != nullptr);
    CHECK(eqNode->op() == "=");

    auto* xId = dynamic_cast<const IdentifierExpression*>(eqNode->left());
    REQUIRE(xId != nullptr);
    CHECK(xId->name() == "x");

    auto* sumNode = dynamic_cast<const BinaryExpr*>(eqNode->right());
    REQUIRE(sumNode != nullptr);
    CHECK(sumNode->op() == "+");
}

// -----------------------------------------------------------------------------
// 2. Full Translation Unit (Program) Integration
// -----------------------------------------------------------------------------
TEST_CASE("Phase 2 Integration: translation unit with prototypes and definitions", "[parser][phase2][program]") {
    const std::string source =
        "int square(int n);\n"
        "void log_msg(char tag);\n\n"
        "int square(int n) {\n"
        "    return n * n;\n"
        "}\n\n"
        "float average(float a, float b) {\n"
        "    return (a + b) / 2.0;\n"
        "}\n\n"
        "void noop(void) {\n"
        "}\n";

    Parser parser = makeParser(source);
    auto program = parser.parseProgram();

    REQUIRE(program != nullptr);
    CHECK_FALSE(parser.hasError());
    CHECK(parser.isAtEnd());
    CHECK(program->statementCount() == 5);

    // 1. Prototype square
    auto* sProto = dynamic_cast<Function*>(program->statements()[0].get());
    REQUIRE(sProto != nullptr);
    CHECK(sProto->name() == "square");
    CHECK(sProto->returnType() == "int");
    CHECK(sProto->isPrototype());
    CHECK(sProto->parameterCount() == 1);

    // 2. Prototype log_msg
    auto* lProto = dynamic_cast<Function*>(program->statements()[1].get());
    REQUIRE(lProto != nullptr);
    CHECK(lProto->name() == "log_msg");
    CHECK(lProto->returnType() == "void");
    CHECK(lProto->isPrototype());
    CHECK(lProto->parameterCount() == 1);

    // 3. Definition square
    auto* sDef = dynamic_cast<Function*>(program->statements()[2].get());
    REQUIRE(sDef != nullptr);
    CHECK(sDef->name() == "square");
    CHECK(sDef->isDefinition());
    CHECK(sDef->parameterCount() == 1);
    CHECK(sDef->body().empty()); // skipped stub

    // 4. Definition average
    auto* aDef = dynamic_cast<Function*>(program->statements()[3].get());
    REQUIRE(aDef != nullptr);
    CHECK(aDef->name() == "average");
    CHECK(aDef->returnType() == "float");
    CHECK(aDef->isDefinition());
    CHECK(aDef->parameterCount() == 2);

    // 5. Definition noop
    auto* nDef = dynamic_cast<Function*>(program->statements()[4].get());
    REQUIRE(nDef != nullptr);
    CHECK(nDef->name() == "noop");
    CHECK(nDef->returnType() == "void");
    CHECK(nDef->isDefinition());
    CHECK(nDef->parameters().empty());

    // Verify AST Printer output on this program
    std::string tree = printAST(program.get());
    CHECK(tree.find("Program\n") == 0);
    CHECK(tree.find("Function (prototype): int square") != std::string::npos);
    CHECK(tree.find("Function (definition): float average") != std::string::npos);
    CHECK(tree.find("[body not parsed]") != std::string::npos);
}

// -----------------------------------------------------------------------------
// 3. Negative and Malformed Input Tests (Clean Failures & Error Reporting)
// -----------------------------------------------------------------------------
TEST_CASE("Phase 2 Negative: missing binary operands", "[parser][phase2][error]") {
    SECTION("missing right operand at EOF: 'a +'") {
        Parser parser = makeParser("a +");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '+'");
    }

    SECTION("missing right operand at EOF: 'a *'") {
        Parser parser = makeParser("a *");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '*'");
    }

    SECTION("missing right operand before semicolon: 'a + ;'") {
        Parser parser = makeParser("a + ;");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '+'");
        CHECK(parser.peek().lexeme == ";");
    }

    SECTION("missing right operand before closing paren: '(a +)'") {
        Parser parser = makeParser("(a +)");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '+'");
    }
}

TEST_CASE("Phase 2 Negative: missing unary operands", "[parser][phase2][error]") {
    SECTION("trailing unary minus at EOF: '-'") {
        Parser parser = makeParser("-");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '-'");
    }

    SECTION("trailing unary not at EOF: '!'") {
        Parser parser = makeParser("!");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '!'");
    }

    SECTION("trailing prefix increment before punctuation: '++ ;'") {
        Parser parser = makeParser("++ ;");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '++'");
    }
}

TEST_CASE("Phase 2 Negative: missing assignment operands", "[parser][phase2][error]") {
    SECTION("missing RHS at EOF: 'x ='") {
        Parser parser = makeParser("x =");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '='");
    }

    SECTION("missing RHS before punctuation: 'x += )'") {
        Parser parser = makeParser("x += )");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected expression after '+='");
    }
}

TEST_CASE("Phase 2 Negative: parentheses errors", "[parser][phase2][error]") {
    SECTION("empty parentheses '()'") {
        Parser parser = makeParser("()");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Empty parenthesized expression");
    }

    SECTION("unmatched opening parenthesis: '(a + b'") {
        Parser parser = makeParser("(a + b");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected ')' after expression");
    }

    SECTION("unmatched nested opening parenthesis: '((a)'") {
        Parser parser = makeParser("((a)");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected ')' after expression");
    }
}

TEST_CASE("Phase 2 Negative: function declaration and definition syntax errors", "[parser][phase2][error]") {
    SECTION("trailing comma in parameter list: 'int f(int a, );'") {
        Parser parser = makeParser("int f(int a, );");
        auto decl = parser.parseDeclaration();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected parameter declaration after ','");
    }

    SECTION("missing delimiter between parameters: 'int f(int a int b);'") {
        Parser parser = makeParser("int f(int a int b);");
        auto decl = parser.parseDeclaration();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected ',' or ')' after parameter declaration");
    }

    SECTION("missing closing parenthesis in parameter list: 'int f(int a;'") {
        Parser parser = makeParser("int f(int a;");
        auto decl = parser.parseDeclaration();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected ',' or ')' after parameter declaration");
    }

    SECTION("missing parameter name: 'int f(int, float b);'") {
        Parser parser = makeParser("int f(int, float b);");
        auto decl = parser.parseDeclaration();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected parameter name after 'int'");
    }

    SECTION("missing parameter type: 'int f(a);'") {
        Parser parser = makeParser("int f(a);");
        auto decl = parser.parseDeclaration();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected parameter type before 'a'");
    }

    SECTION("unclosed function body at EOF: 'int f() {'") {
        Parser parser = makeParser("int f() {");
        auto decl = parser.parseDeclaration();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected '}' to close function body");
    }

    SECTION("missing prototype or definition terminator at EOF: 'int f()'") {
        Parser parser = makeParser("int f()");
        auto decl = parser.parseDeclaration();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected ';' or '{' after function declarator");
    }

    SECTION("unexpected character after function declarator: 'int f() = 0;'") {
        Parser parser = makeParser("int f() = 0;");
        auto decl = parser.parseDeclaration();
        CHECK(decl == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage() == "Expected ';' or '{' after function declarator, found '='");
    }
}

TEST_CASE("Phase 2 Negative: translation unit unexpected token error", "[parser][phase2][error]") {
    // Translation unit where a non-declaration appears
    Parser parser = makeParser("int f(); 123");
    auto prog = parser.parseProgram();
    CHECK(prog == nullptr);
    CHECK(parser.hasError());
    CHECK(parser.errorMessage() == "Unexpected token in translation unit: '123'");
}

// -----------------------------------------------------------------------------
// 4. Token Consumption and Trailing Token Detection
// -----------------------------------------------------------------------------
TEST_CASE("Phase 2: expression parser stops cleanly at unexpected trailing tokens", "[parser][phase2][trailing]") {
    Parser parser = makeParser("x + 10 extra_token");
    auto expr = parser.parseExpression();

    REQUIRE(expr != nullptr);
    CHECK_FALSE(parser.hasError());
    CHECK_FALSE(parser.isAtEnd());
    CHECK(parser.peek().lexeme == "extra_token");
}

TEST_CASE("Phase 2: variable declaration is left unconsumed by function parser", "[parser][phase2][trailing]") {
    Parser parser = makeParser("int count = 42;");
    auto fn = parser.parseFunctionDeclaration();

    CHECK(fn == nullptr);
    CHECK_FALSE(parser.hasError());
    CHECK_FALSE(parser.isAtEnd());
    CHECK(parser.peek().lexeme == "int");
}

