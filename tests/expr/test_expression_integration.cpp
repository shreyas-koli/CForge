#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/printer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

#include <memory>
#include <string>

// Helper to construct a Parser for a source string
static Parser makeParser(const std::string& src) {
    Lexer lexer(src);
    return Parser(lexer.tokenize());
}

// Helper to print AST node to string via ASTPrinter
static std::string dumpAST(const ASTNode* node) {
    ASTPrinter printer;
    return printer.print(node);
}

// =============================================================================
// DAY 27 TESTS — Phase 3 Expression Integration & Testing
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Mandatory AST-Structure Assertions (Roadmap Examples)
// -----------------------------------------------------------------------------
TEST_CASE("ExprIntegration: mandatory roadmap AST shapes", "[expr][integration][precedence]") {

    SECTION("1. 'a + b * c - d / e' -> ((a + (b * c)) - (d / e))") {
        Parser parser = makeParser("a + b * c - d / e");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "-");

        auto* leftPlus = dynamic_cast<const BinaryExpr*>(root->left());
        REQUIRE(leftPlus != nullptr);
        CHECK(leftPlus->op() == "+");
        CHECK(leftPlus->left()->toString() == "IdentifierExpression(a)");

        auto* bMulC = dynamic_cast<const BinaryExpr*>(leftPlus->right());
        REQUIRE(bMulC != nullptr);
        CHECK(bMulC->op() == "*");
        CHECK(bMulC->left()->toString() == "IdentifierExpression(b)");
        CHECK(bMulC->right()->toString() == "IdentifierExpression(c)");

        auto* rightDiv = dynamic_cast<const BinaryExpr*>(root->right());
        REQUIRE(rightDiv != nullptr);
        CHECK(rightDiv->op() == "/");
        CHECK(rightDiv->left()->toString() == "IdentifierExpression(d)");
        CHECK(rightDiv->right()->toString() == "IdentifierExpression(e)");

        CHECK(expr->toString() ==
              "BinaryExpr(-, BinaryExpr(+, IdentifierExpression(a), BinaryExpr(*, IdentifierExpression(b), IdentifierExpression(c))), BinaryExpr(/, IdentifierExpression(d), IdentifierExpression(e)))");
    }

    SECTION("2. 'a << b + c' -> a << (b + c) (Additive > Shift)") {
        Parser parser = makeParser("a << b + c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "<<");
        CHECK(root->left()->toString() == "IdentifierExpression(a)");

        auto* rightAdd = dynamic_cast<const BinaryExpr*>(root->right());
        REQUIRE(rightAdd != nullptr);
        CHECK(rightAdd->op() == "+");
        CHECK(rightAdd->left()->toString() == "IdentifierExpression(b)");
        CHECK(rightAdd->right()->toString() == "IdentifierExpression(c)");

        CHECK(expr->toString() ==
              "BinaryExpr(<<, IdentifierExpression(a), BinaryExpr(+, IdentifierExpression(b), IdentifierExpression(c)))");
    }

    SECTION("3. 'a < b << c' -> a < (b << c) (Shift > Relational)") {
        Parser parser = makeParser("a < b << c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "<");
        CHECK(root->left()->toString() == "IdentifierExpression(a)");

        auto* rightShift = dynamic_cast<const BinaryExpr*>(root->right());
        REQUIRE(rightShift != nullptr);
        CHECK(rightShift->op() == "<<");
        CHECK(rightShift->left()->toString() == "IdentifierExpression(b)");
        CHECK(rightShift->right()->toString() == "IdentifierExpression(c)");

        CHECK(expr->toString() ==
              "BinaryExpr(<, IdentifierExpression(a), BinaryExpr(<<, IdentifierExpression(b), IdentifierExpression(c)))");
    }

    SECTION("4. 'a & b == c' -> (a & b) == c (Bitwise AND > Equality)") {
        Parser parser = makeParser("a & b == c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "==");
        CHECK(root->right()->toString() == "IdentifierExpression(c)");

        auto* leftAnd = dynamic_cast<const BinaryExpr*>(root->left());
        REQUIRE(leftAnd != nullptr);
        CHECK(leftAnd->op() == "&");
        CHECK(leftAnd->left()->toString() == "IdentifierExpression(a)");
        CHECK(leftAnd->right()->toString() == "IdentifierExpression(b)");

        CHECK(expr->toString() ==
              "BinaryExpr(==, BinaryExpr(&, IdentifierExpression(a), IdentifierExpression(b)), IdentifierExpression(c))");
    }

    SECTION("5. 'a & b ^ c | d' -> ((a & b) ^ c) | d (& > ^ > |)") {
        Parser parser = makeParser("a & b ^ c | d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "|");
        CHECK(root->right()->toString() == "IdentifierExpression(d)");

        auto* leftXor = dynamic_cast<const BinaryExpr*>(root->left());
        REQUIRE(leftXor != nullptr);
        CHECK(leftXor->op() == "^");
        CHECK(leftXor->right()->toString() == "IdentifierExpression(c)");

        auto* leftAnd = dynamic_cast<const BinaryExpr*>(leftXor->left());
        REQUIRE(leftAnd != nullptr);
        CHECK(leftAnd->op() == "&");
        CHECK(leftAnd->left()->toString() == "IdentifierExpression(a)");
        CHECK(leftAnd->right()->toString() == "IdentifierExpression(b)");

        CHECK(expr->toString() ==
              "BinaryExpr(|, BinaryExpr(^, BinaryExpr(&, IdentifierExpression(a), IdentifierExpression(b)), IdentifierExpression(c)), IdentifierExpression(d))");
    }

    SECTION("6. 'a == b != c' -> (a == b) != c (Left-associative equality)") {
        Parser parser = makeParser("a == b != c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "!=");
        CHECK(root->right()->toString() == "IdentifierExpression(c)");

        auto* leftEq = dynamic_cast<const BinaryExpr*>(root->left());
        REQUIRE(leftEq != nullptr);
        CHECK(leftEq->op() == "==");
        CHECK(leftEq->left()->toString() == "IdentifierExpression(a)");
        CHECK(leftEq->right()->toString() == "IdentifierExpression(b)");

        CHECK(expr->toString() ==
              "BinaryExpr(!=, BinaryExpr(==, IdentifierExpression(a), IdentifierExpression(b)), IdentifierExpression(c))");
    }

    SECTION("7. 'a ? b : c ? d : e' -> a ? b : (c ? d : e) (Right-associative ternary)") {
        Parser parser = makeParser("a ? b : c ? d : e");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* root = dynamic_cast<ConditionalExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->condition()->toString() == "IdentifierExpression(a)");
        CHECK(root->thenExpr()->toString() == "IdentifierExpression(b)");

        auto* elseCond = dynamic_cast<const ConditionalExpr*>(root->elseExpr());
        REQUIRE(elseCond != nullptr);
        CHECK(elseCond->condition()->toString() == "IdentifierExpression(c)");
        CHECK(elseCond->thenExpr()->toString() == "IdentifierExpression(d)");
        CHECK(elseCond->elseExpr()->toString() == "IdentifierExpression(e)");

        CHECK(expr->toString() ==
              "ConditionalExpr(IdentifierExpression(a), IdentifierExpression(b), ConditionalExpr(IdentifierExpression(c), IdentifierExpression(d), IdentifierExpression(e)))");
    }

    SECTION("8. 'a ? b ? c : d : e' -> a ? (b ? c : d) : e (Nested then ternary)") {
        Parser parser = makeParser("a ? b ? c : d : e");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* root = dynamic_cast<ConditionalExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->condition()->toString() == "IdentifierExpression(a)");
        CHECK(root->elseExpr()->toString() == "IdentifierExpression(e)");

        auto* thenCond = dynamic_cast<const ConditionalExpr*>(root->thenExpr());
        REQUIRE(thenCond != nullptr);
        CHECK(thenCond->condition()->toString() == "IdentifierExpression(b)");
        CHECK(thenCond->thenExpr()->toString() == "IdentifierExpression(c)");
        CHECK(thenCond->elseExpr()->toString() == "IdentifierExpression(d)");

        CHECK(expr->toString() ==
              "ConditionalExpr(IdentifierExpression(a), ConditionalExpr(IdentifierExpression(b), IdentifierExpression(c), IdentifierExpression(d)), IdentifierExpression(e))");
    }

    SECTION("9. 'x = condition ? a : b' -> x = (condition ? a : b)") {
        Parser parser = makeParser("x = condition ? a : b");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* root = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "=");
        CHECK(root->left()->toString() == "IdentifierExpression(x)");

        auto* cond = dynamic_cast<const ConditionalExpr*>(root->right());
        REQUIRE(cond != nullptr);
        CHECK(cond->condition()->toString() == "IdentifierExpression(condition)");
        CHECK(cond->thenExpr()->toString() == "IdentifierExpression(a)");
        CHECK(cond->elseExpr()->toString() == "IdentifierExpression(b)");

        CHECK(expr->toString() ==
              "AssignExpr(=, IdentifierExpression(x), ConditionalExpr(IdentifierExpression(condition), IdentifierExpression(a), IdentifierExpression(b)))");
    }

    SECTION("10. 'x = y = a + b * c' -> x = (y = (a + (b * c))) (Right-associative assignment)") {
        Parser parser = makeParser("x = y = a + b * c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* root = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "=");
        CHECK(root->left()->toString() == "IdentifierExpression(x)");

        auto* innerAssign = dynamic_cast<const AssignExpr*>(root->right());
        REQUIRE(innerAssign != nullptr);
        CHECK(innerAssign->op() == "=");
        CHECK(innerAssign->left()->toString() == "IdentifierExpression(y)");

        auto* add = dynamic_cast<const BinaryExpr*>(innerAssign->right());
        REQUIRE(add != nullptr);
        CHECK(add->op() == "+");
        CHECK(add->left()->toString() == "IdentifierExpression(a)");

        auto* mul = dynamic_cast<const BinaryExpr*>(add->right());
        REQUIRE(mul != nullptr);
        CHECK(mul->op() == "*");
        CHECK(mul->left()->toString() == "IdentifierExpression(b)");
        CHECK(mul->right()->toString() == "IdentifierExpression(c)");

        CHECK(expr->toString() ==
              "AssignExpr(=, IdentifierExpression(x), AssignExpr(=, IdentifierExpression(y), BinaryExpr(+, IdentifierExpression(a), BinaryExpr(*, IdentifierExpression(b), IdentifierExpression(c)))))");
    }

    SECTION("11. '-a * b + ~c' -> ((-a) * b) + (~c) (Unary > Multiplicative > Additive)") {
        Parser parser = makeParser("-a * b + ~c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "+");

        auto* mul = dynamic_cast<const BinaryExpr*>(root->left());
        REQUIRE(mul != nullptr);
        CHECK(mul->op() == "*");

        auto* negA = dynamic_cast<const UnaryExpr*>(mul->left());
        REQUIRE(negA != nullptr);
        CHECK(negA->op() == "-");
        CHECK(negA->operand()->toString() == "IdentifierExpression(a)");
        CHECK(mul->right()->toString() == "IdentifierExpression(b)");

        auto* notC = dynamic_cast<const UnaryExpr*>(root->right());
        REQUIRE(notC != nullptr);
        CHECK(notC->op() == "~");
        CHECK(notC->operand()->toString() == "IdentifierExpression(c)");

        CHECK(expr->toString() ==
              "BinaryExpr(+, BinaryExpr(*, UnaryExpr(-, IdentifierExpression(a)), IdentifierExpression(b)), UnaryExpr(~, IdentifierExpression(c)))");
    }

    SECTION("12. '(a + b) * (c - d)' -> (a + b) * (c - d) (Parens override precedence)") {
        Parser parser = makeParser("(a + b) * (c - d)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "*");

        auto* add = dynamic_cast<const BinaryExpr*>(root->left());
        REQUIRE(add != nullptr);
        CHECK(add->op() == "+");
        CHECK(add->left()->toString() == "IdentifierExpression(a)");
        CHECK(add->right()->toString() == "IdentifierExpression(b)");

        auto* sub = dynamic_cast<const BinaryExpr*>(root->right());
        REQUIRE(sub != nullptr);
        CHECK(sub->op() == "-");
        CHECK(sub->left()->toString() == "IdentifierExpression(c)");
        CHECK(sub->right()->toString() == "IdentifierExpression(d)");

        CHECK(expr->toString() ==
              "BinaryExpr(*, BinaryExpr(+, IdentifierExpression(a), IdentifierExpression(b)), BinaryExpr(-, IdentifierExpression(c), IdentifierExpression(d)))");
    }

    SECTION("13. 'a + b < c * d == e' -> ((a + b) < (c * d)) == e") {
        Parser parser = makeParser("a + b < c * d == e");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "==");
        CHECK(root->right()->toString() == "IdentifierExpression(e)");

        auto* rel = dynamic_cast<const BinaryExpr*>(root->left());
        REQUIRE(rel != nullptr);
        CHECK(rel->op() == "<");

        auto* add = dynamic_cast<const BinaryExpr*>(rel->left());
        REQUIRE(add != nullptr);
        CHECK(add->op() == "+");
        CHECK(add->left()->toString() == "IdentifierExpression(a)");
        CHECK(add->right()->toString() == "IdentifierExpression(b)");

        auto* mul = dynamic_cast<const BinaryExpr*>(rel->right());
        REQUIRE(mul != nullptr);
        CHECK(mul->op() == "*");
        CHECK(mul->left()->toString() == "IdentifierExpression(c)");
        CHECK(mul->right()->toString() == "IdentifierExpression(d)");

        CHECK(expr->toString() ==
              "BinaryExpr(==, BinaryExpr(<, BinaryExpr(+, IdentifierExpression(a), IdentifierExpression(b)), BinaryExpr(*, IdentifierExpression(c), IdentifierExpression(d))), IdentifierExpression(e))");
    }

    SECTION("14. 'a & b | c ^ d' -> (a & b) | (c ^ d) (& > ^ > |)") {
        Parser parser = makeParser("a & b | c ^ d");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "|");

        auto* leftAnd = dynamic_cast<const BinaryExpr*>(root->left());
        REQUIRE(leftAnd != nullptr);
        CHECK(leftAnd->op() == "&");
        CHECK(leftAnd->left()->toString() == "IdentifierExpression(a)");
        CHECK(leftAnd->right()->toString() == "IdentifierExpression(b)");

        auto* rightXor = dynamic_cast<const BinaryExpr*>(root->right());
        REQUIRE(rightXor != nullptr);
        CHECK(rightXor->op() == "^");
        CHECK(rightXor->left()->toString() == "IdentifierExpression(c)");
        CHECK(rightXor->right()->toString() == "IdentifierExpression(d)");

        CHECK(expr->toString() ==
              "BinaryExpr(|, BinaryExpr(&, IdentifierExpression(a), IdentifierExpression(b)), BinaryExpr(^, IdentifierExpression(c), IdentifierExpression(d)))");
    }
}

// -----------------------------------------------------------------------------
// 2. Full Hierarchy Cross-Tier Integration Tests
// -----------------------------------------------------------------------------
TEST_CASE("ExprIntegration: full hierarchy cross-tier combinations", "[expr][integration][cross_tier]") {

    SECTION("Full expression across unary, arithmetic, shift, ternary, and assignment") {
        // x = !flag ? a << 2 + 1 : b >> 1 & 0xF
        // + binds tighter than << : a << (2 + 1)
        // >> binds tighter than & : (b >> 1) & 0xF
        // ! binds tighter than ? : (!flag) ? (a << (2 + 1)) : ((b >> 1) & 0xF)
        // = has lowest precedence
        Parser parser = makeParser("x = !flag ? a << 2 + 1 : b >> 1 & 0xF");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* assign = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(assign != nullptr);
        CHECK(assign->op() == "=");
        CHECK(assign->left()->toString() == "IdentifierExpression(x)");

        auto* cond = dynamic_cast<const ConditionalExpr*>(assign->right());
        REQUIRE(cond != nullptr);

        auto* notFlag = dynamic_cast<const UnaryExpr*>(cond->condition());
        REQUIRE(notFlag != nullptr);
        CHECK(notFlag->op() == "!");

        auto* shiftAdd = dynamic_cast<const BinaryExpr*>(cond->thenExpr());
        REQUIRE(shiftAdd != nullptr);
        CHECK(shiftAdd->op() == "<<");
        auto* add = dynamic_cast<const BinaryExpr*>(shiftAdd->right());
        REQUIRE(add != nullptr);
        CHECK(add->op() == "+");

        auto* bitAnd = dynamic_cast<const BinaryExpr*>(cond->elseExpr());
        REQUIRE(bitAnd != nullptr);
        CHECK(bitAnd->op() == "&");
        auto* shiftRight = dynamic_cast<const BinaryExpr*>(bitAnd->left());
        REQUIRE(shiftRight != nullptr);
        CHECK(shiftRight->op() == ">>");
    }

    SECTION("Compound assignment chaining: 'a += b = c *= d + e'") {
        // a += (b = (c *= (d + e)))
        Parser parser = makeParser("a += b = c *= d + e");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* a1 = dynamic_cast<AssignExpr*>(expr.get());
        REQUIRE(a1 != nullptr);
        CHECK(a1->op() == "+=");
        CHECK(a1->left()->toString() == "IdentifierExpression(a)");

        auto* a2 = dynamic_cast<const AssignExpr*>(a1->right());
        REQUIRE(a2 != nullptr);
        CHECK(a2->op() == "=");
        CHECK(a2->left()->toString() == "IdentifierExpression(b)");

        auto* a3 = dynamic_cast<const AssignExpr*>(a2->right());
        REQUIRE(a3 != nullptr);
        CHECK(a3->op() == "*=");
        CHECK(a3->left()->toString() == "IdentifierExpression(c)");

        auto* add = dynamic_cast<const BinaryExpr*>(a3->right());
        REQUIRE(add != nullptr);
        CHECK(add->op() == "+");
    }

    SECTION("Parenthesized ternary in condition: '(a ? b : c) ? d : e'") {
        Parser parser = makeParser("(a ? b : c) ? d : e");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* outer = dynamic_cast<ConditionalExpr*>(expr.get());
        REQUIRE(outer != nullptr);

        auto* inner = dynamic_cast<const ConditionalExpr*>(outer->condition());
        REQUIRE(inner != nullptr);
        CHECK(inner->condition()->toString() == "IdentifierExpression(a)");
        CHECK(inner->thenExpr()->toString() == "IdentifierExpression(b)");
        CHECK(inner->elseExpr()->toString() == "IdentifierExpression(c)");

        CHECK(outer->thenExpr()->toString() == "IdentifierExpression(d)");
        CHECK(outer->elseExpr()->toString() == "IdentifierExpression(e)");
    }

    SECTION("Chained unary prefix operators: '!~-a'") {
        Parser parser = makeParser("!~-a");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* u1 = dynamic_cast<UnaryExpr*>(expr.get());
        REQUIRE(u1 != nullptr);
        CHECK(u1->op() == "!");

        auto* u2 = dynamic_cast<const UnaryExpr*>(u1->operand());
        REQUIRE(u2 != nullptr);
        CHECK(u2->op() == "~");

        auto* u3 = dynamic_cast<const UnaryExpr*>(u2->operand());
        REQUIRE(u3 != nullptr);
        CHECK(u3->op() == "-");
        CHECK(u3->operand()->toString() == "IdentifierExpression(a)");
    }
}

// -----------------------------------------------------------------------------
// 3. Negative Tests and Robustness
// -----------------------------------------------------------------------------
TEST_CASE("ExprIntegration: negative syntax error tests", "[expr][integration][negative]") {

    SECTION("Missing right operands for arithmetic and shift") {
        for (const std::string& op : {"+", "-", "*", "/", "%", "<<", ">>"}) {
            Parser parser = makeParser("a " + op);
            auto expr = parser.parseExpression();
            CHECK(expr == nullptr);
            CHECK(parser.hasError());
            CHECK(parser.errorMessage().find("Expected expression after '" + op + "'") != std::string::npos);
        }
    }

    SECTION("Missing right operands for equality") {
        for (const std::string& op : {"==", "!="}) {
            Parser parser = makeParser("a " + op);
            auto expr = parser.parseExpression();
            CHECK(expr == nullptr);
            CHECK(parser.hasError());
            CHECK(parser.errorMessage().find("Expected expression after '" + op + "'") != std::string::npos);
        }
    }

    SECTION("Missing right operands for bitwise") {
        for (const std::string& op : {"&", "^", "|"}) {
            Parser parser = makeParser("a " + op);
            auto expr = parser.parseExpression();
            CHECK(expr == nullptr);
            CHECK(parser.hasError());
            CHECK(parser.errorMessage().find("Expected expression after '" + op + "'") != std::string::npos);
        }
    }

    SECTION("Missing ternary colon: 'a ? b'") {
        Parser parser = makeParser("a ? b");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ':'") != std::string::npos);
    }

    SECTION("Missing ternary true branch: 'a ? : c'") {
        Parser parser = makeParser("a ? : c");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression after '?'") != std::string::npos);
    }

    SECTION("Missing ternary false branch: 'a ? b :'") {
        Parser parser = makeParser("a ? b :");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression after ':'") != std::string::npos);
    }

    SECTION("Nested malformed expressions in parentheses") {
        {
            Parser parser = makeParser("a + (b *)");
            auto expr = parser.parseExpression();
            CHECK(expr == nullptr);
            CHECK(parser.hasError());
        }
        {
            Parser parser = makeParser("x = (a ? : b)");
            auto expr = parser.parseExpression();
            CHECK(expr == nullptr);
            CHECK(parser.hasError());
        }
        {
            Parser parser = makeParser("a & (b ==)");
            auto expr = parser.parseExpression();
            CHECK(expr == nullptr);
            CHECK(parser.hasError());
        }
    }

    SECTION("Trailing unexpected tokens cleanly unconsumed") {
        {
            Parser parser = makeParser("a + b c");
            auto expr = parser.parseExpression();
            REQUIRE(expr != nullptr);
            CHECK_FALSE(parser.hasError());
            CHECK_FALSE(parser.isAtEnd());
            CHECK(parser.peek().lexeme == "c");
        }
        {
            Parser parser = makeParser("x = 10 20");
            auto expr = parser.parseExpression();
            REQUIRE(expr != nullptr);
            CHECK_FALSE(parser.hasError());
            CHECK_FALSE(parser.isAtEnd());
            CHECK(parser.peek().lexeme == "20");
        }
        {
            Parser parser = makeParser("(a + b) )");
            auto expr = parser.parseExpression();
            REQUIRE(expr != nullptr);
            CHECK_FALSE(parser.hasError());
            CHECK_FALSE(parser.isAtEnd());
            CHECK(parser.peek().lexeme == ")");
        }
    }

    SECTION("Deeply nested parentheses do not stack overflow") {
        std::string src = "((((((((((a + b))))))))))";
        Parser parser = makeParser(src);
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* root = dynamic_cast<BinaryExpr*>(expr.get());
        REQUIRE(root != nullptr);
        CHECK(root->op() == "+");
        CHECK(root->left()->toString() == "IdentifierExpression(a)");
        CHECK(root->right()->toString() == "IdentifierExpression(b)");
    }

    SECTION("Deeply chained unary expressions") {
        std::string src = "!!!!!!a";
        Parser parser = makeParser(src);
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        const Expression* curr = expr.get();
        for (int i = 0; i < 6; ++i) {
            auto* un = dynamic_cast<const UnaryExpr*>(curr);
            REQUIRE(un != nullptr);
            CHECK(un->op() == "!");
            curr = un->operand();
        }
        auto* id = dynamic_cast<const IdentifierExpression*>(curr);
        REQUIRE(id != nullptr);
        CHECK(id->name() == "a");
    }
}

// -----------------------------------------------------------------------------
// 4. ASTPrinter Integration
// -----------------------------------------------------------------------------
TEST_CASE("ExprIntegration: ASTPrinter output verification", "[expr][integration][printer]") {
    SECTION("Formatted print of complex ternary conditional with arithmetic") {
        Parser parser = makeParser("result = a > b ? a + 1 : b * 2");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        std::string printed = dumpAST(expr.get());
        CHECK(printed.find("AssignExpr: =") != std::string::npos);
        CHECK(printed.find("ConditionalExpr") != std::string::npos);
        CHECK(printed.find("Condition:") != std::string::npos);
        CHECK(printed.find("BinaryExpr: >") != std::string::npos);
        CHECK(printed.find("Then:") != std::string::npos);
        CHECK(printed.find("BinaryExpr: +") != std::string::npos);
        CHECK(printed.find("Else:") != std::string::npos);
        CHECK(printed.find("BinaryExpr: *") != std::string::npos);
    }
}

