#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/index_expr.h"
#include "ast/identifier_expression.h"
#include "ast/literal_expression.h"
#include "ast/binary_expr.h"
#include "ast/call_expr.h"
#include "ast/printer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

static Parser makeParser(const std::string& src) {
    Lexer lexer(src);
    return Parser(lexer.tokenize());
}

// =============================================================================
// DAY 55 TESTS — Array Indexing (Subscript Expression Parsing)
// =============================================================================

TEST_CASE("ArrayIndexing: Area A - Basic indexing", "[parser][expr][array][indexing]") {
    SECTION("numbers[0]") {
        Parser parser = makeParser("numbers[0]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* idx = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(idx != nullptr);

        // Base check
        REQUIRE(idx->base() != nullptr);
        auto* baseId = dynamic_cast<const IdentifierExpression*>(idx->base());
        REQUIRE(baseId != nullptr);
        CHECK(baseId->name() == "numbers");
        CHECK(idx->getBase() == idx->base());
        CHECK(idx->baseRef().toString() == baseId->toString());

        // Index check
        REQUIRE(idx->index() != nullptr);
        auto* indexLit = dynamic_cast<const LiteralExpression*>(idx->index());
        REQUIRE(indexLit != nullptr);
        CHECK(indexLit->kind() == LiteralKind::Integer);
        CHECK(indexLit->value() == "0");
        CHECK(idx->getIndex() == idx->index());
        CHECK(idx->indexRef().toString() == indexLit->toString());
    }

    SECTION("numbers[5]") {
        Parser parser = makeParser("numbers[5]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* idx = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(idx != nullptr);

        auto* baseId = dynamic_cast<const IdentifierExpression*>(idx->base());
        REQUIRE(baseId != nullptr);
        CHECK(baseId->name() == "numbers");

        auto* indexLit = dynamic_cast<const LiteralExpression*>(idx->index());
        REQUIRE(indexLit != nullptr);
        CHECK(indexLit->value() == "5");
    }

    SECTION("Large integer index: buffer[1024]") {
        Parser parser = makeParser("buffer[1024]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* idx = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(idx != nullptr);
        auto* baseId = dynamic_cast<const IdentifierExpression*>(idx->base());
        REQUIRE(baseId != nullptr);
        CHECK(baseId->name() == "buffer");

        auto* indexLit = dynamic_cast<const LiteralExpression*>(idx->index());
        REQUIRE(indexLit != nullptr);
        CHECK(indexLit->value() == "1024");
    }
}

TEST_CASE("ArrayIndexing: Area B - Identifier indexing", "[parser][expr][array][indexing]") {
    SECTION("numbers[i]") {
        Parser parser = makeParser("numbers[i]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* idx = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(idx != nullptr);

        auto* baseId = dynamic_cast<const IdentifierExpression*>(idx->base());
        REQUIRE(baseId != nullptr);
        CHECK(baseId->name() == "numbers");

        auto* indexId = dynamic_cast<const IdentifierExpression*>(idx->index());
        REQUIRE(indexId != nullptr);
        CHECK(indexId->name() == "i");
    }

    SECTION("table[index_var]") {
        Parser parser = makeParser("table[index_var]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* idx = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(idx != nullptr);

        auto* baseId = dynamic_cast<const IdentifierExpression*>(idx->base());
        REQUIRE(baseId != nullptr);
        CHECK(baseId->name() == "table");

        auto* indexId = dynamic_cast<const IdentifierExpression*>(idx->index());
        REQUIRE(indexId != nullptr);
        CHECK(indexId->name() == "index_var");
    }
}

TEST_CASE("ArrayIndexing: Area C - Arithmetic index expressions", "[parser][expr][array][indexing]") {
    SECTION("numbers[i + 1]") {
        Parser parser = makeParser("numbers[i + 1]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* idx = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(idx != nullptr);

        auto* baseId = dynamic_cast<const IdentifierExpression*>(idx->base());
        REQUIRE(baseId != nullptr);
        CHECK(baseId->name() == "numbers");

        auto* bin = dynamic_cast<const BinaryExpr*>(idx->index());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");

        auto* leftId = dynamic_cast<const IdentifierExpression*>(bin->left());
        REQUIRE(leftId != nullptr);
        CHECK(leftId->name() == "i");

        auto* rightLit = dynamic_cast<const LiteralExpression*>(bin->right());
        REQUIRE(rightLit != nullptr);
        CHECK(rightLit->value() == "1");
    }

    SECTION("numbers[2 * 3]") {
        Parser parser = makeParser("numbers[2 * 3]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* idx = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(idx != nullptr);

        auto* bin = dynamic_cast<const BinaryExpr*>(idx->index());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "*");

        auto* leftLit = dynamic_cast<const LiteralExpression*>(bin->left());
        REQUIRE(leftLit != nullptr);
        CHECK(leftLit->value() == "2");

        auto* rightLit = dynamic_cast<const LiteralExpression*>(bin->right());
        REQUIRE(rightLit != nullptr);
        CHECK(rightLit->value() == "3");
    }

    SECTION("numbers[(i + 1) * 2]") {
        Parser parser = makeParser("numbers[(i + 1) * 2]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* idx = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(idx != nullptr);

        auto* mulBin = dynamic_cast<const BinaryExpr*>(idx->index());
        REQUIRE(mulBin != nullptr);
        CHECK(mulBin->op() == "*");

        auto* addBin = dynamic_cast<const BinaryExpr*>(mulBin->left());
        REQUIRE(addBin != nullptr);
        CHECK(addBin->op() == "+");

        auto* iId = dynamic_cast<const IdentifierExpression*>(addBin->left());
        REQUIRE(iId != nullptr);
        CHECK(iId->name() == "i");

        auto* oneLit = dynamic_cast<const LiteralExpression*>(addBin->right());
        REQUIRE(oneLit != nullptr);
        CHECK(oneLit->value() == "1");

        auto* twoLit = dynamic_cast<const LiteralExpression*>(mulBin->right());
        REQUIRE(twoLit != nullptr);
        CHECK(twoLit->value() == "2");
    }

    SECTION("Complex arithmetic in index: arr[a + b * c - d / e]") {
        Parser parser = makeParser("arr[a + b * c - d / e]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* idx = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(idx != nullptr);
        REQUIRE(idx->index() != nullptr);
        // Precedence ensures subtraction at top of additive level
        auto* subBin = dynamic_cast<const BinaryExpr*>(idx->index());
        REQUIRE(subBin != nullptr);
        CHECK(subBin->op() == "-");
    }
}

TEST_CASE("ArrayIndexing: Area D - Nested indexing", "[parser][expr][array][indexing]") {
    SECTION("numbers[i][j]") {
        Parser parser = makeParser("numbers[i][j]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        // Outer IndexExpr: (numbers[i])[j]
        auto* outerIdx = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(outerIdx != nullptr);

        // Outer index is 'j'
        auto* jId = dynamic_cast<const IdentifierExpression*>(outerIdx->index());
        REQUIRE(jId != nullptr);
        CHECK(jId->name() == "j");

        // Outer base is inner IndexExpr: numbers[i]
        auto* innerIdx = dynamic_cast<const IndexExpr*>(outerIdx->base());
        REQUIRE(innerIdx != nullptr);

        auto* iId = dynamic_cast<const IdentifierExpression*>(innerIdx->index());
        REQUIRE(iId != nullptr);
        CHECK(iId->name() == "i");

        auto* baseId = dynamic_cast<const IdentifierExpression*>(innerIdx->base());
        REQUIRE(baseId != nullptr);
        CHECK(baseId->name() == "numbers");
    }

    SECTION("Triple nested indexing: matrix[x][y][z]") {
        Parser parser = makeParser("matrix[x][y][z]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* idx3 = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(idx3 != nullptr);
        auto* zId = dynamic_cast<const IdentifierExpression*>(idx3->index());
        REQUIRE(zId != nullptr);
        CHECK(zId->name() == "z");

        auto* idx2 = dynamic_cast<const IndexExpr*>(idx3->base());
        REQUIRE(idx2 != nullptr);
        auto* yId = dynamic_cast<const IdentifierExpression*>(idx2->index());
        REQUIRE(yId != nullptr);
        CHECK(yId->name() == "y");

        auto* idx1 = dynamic_cast<const IndexExpr*>(idx2->base());
        REQUIRE(idx1 != nullptr);
        auto* xId = dynamic_cast<const IdentifierExpression*>(idx1->index());
        REQUIRE(xId != nullptr);
        CHECK(xId->name() == "x");

        auto* matId = dynamic_cast<const IdentifierExpression*>(idx1->base());
        REQUIRE(matId != nullptr);
        CHECK(matId->name() == "matrix");
    }

    SECTION("Indexed element in function call: foo(arr[i])") {
        Parser parser = makeParser("foo(arr[i])");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* call = dynamic_cast<const CallExpr*>(expr.get());
        REQUIRE(call != nullptr);
        REQUIRE(call->argumentCount() == 1);

        auto* argIdx = dynamic_cast<const IndexExpr*>(call->argument(0));
        REQUIRE(argIdx != nullptr);
        auto* bId = dynamic_cast<const IdentifierExpression*>(argIdx->base());
        REQUIRE(bId != nullptr);
        CHECK(bId->name() == "arr");
    }

    SECTION("Function call result indexed: getArray()[0]") {
        Parser parser = makeParser("getArray()[0]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* idx = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(idx != nullptr);

        auto* call = dynamic_cast<const CallExpr*>(idx->base());
        REQUIRE(call != nullptr);
        CHECK(call->calleeName() == "getArray");

        auto* lit = dynamic_cast<const LiteralExpression*>(idx->index());
        REQUIRE(lit != nullptr);
        CHECK(lit->value() == "0");
    }
}

TEST_CASE("ArrayIndexing: Area E - Parenthesized expressions", "[parser][expr][array][indexing]") {
    SECTION("(numbers)[5]") {
        Parser parser = makeParser("(numbers)[5]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* idx = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(idx != nullptr);

        auto* baseId = dynamic_cast<const IdentifierExpression*>(idx->base());
        REQUIRE(baseId != nullptr);
        CHECK(baseId->name() == "numbers");

        auto* lit = dynamic_cast<const LiteralExpression*>(idx->index());
        REQUIRE(lit != nullptr);
        CHECK(lit->value() == "5");
    }

    SECTION("numbers[(i + 1)]") {
        Parser parser = makeParser("numbers[(i + 1)]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* idx = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(idx != nullptr);

        auto* bin = dynamic_cast<const BinaryExpr*>(idx->index());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");
    }

    SECTION("((arr))[((k))]") {
        Parser parser = makeParser("((arr))[((k))]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* idx = dynamic_cast<const IndexExpr*>(expr.get());
        REQUIRE(idx != nullptr);

        auto* baseId = dynamic_cast<const IdentifierExpression*>(idx->base());
        REQUIRE(baseId != nullptr);
        CHECK(baseId->name() == "arr");

        auto* idxId = dynamic_cast<const IdentifierExpression*>(idx->index());
        REQUIRE(idxId != nullptr);
        CHECK(idxId->name() == "k");
    }
}

TEST_CASE("ArrayIndexing: Area F - Error handling", "[parser][expr][array][indexing][errors]") {
    SECTION("Unterminated bracket at EOF: 'numbers['") {
        Parser parser = makeParser("numbers[");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression after '['") != std::string::npos);
    }

    SECTION("Missing closing bracket at EOF: 'numbers[5'") {
        Parser parser = makeParser("numbers[5");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ']'") != std::string::npos);
    }

    SECTION("Missing closing bracket before identifier: 'numbers[5 x'") {
        Parser parser = makeParser("numbers[5 x");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected ']'") != std::string::npos);
    }

    SECTION("Empty brackets: 'numbers[]'") {
        Parser parser = makeParser("numbers[]");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression between '[' and ']'") != std::string::npos);
    }

    SECTION("Dangling operator inside brackets: 'numbers[1 + ]'") {
        Parser parser = makeParser("numbers[1 + ]");
        auto expr = parser.parseExpression();
        CHECK(expr == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected expression after '+'") != std::string::npos);
    }
}

TEST_CASE("ArrayIndexing: Area G - Regression testing with existing expressions", "[parser][expr][array][regression]") {
    SECTION("Binary addition: 'x + y'") {
        Parser parser = makeParser("x + y");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<const BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");
    }

    SECTION("Function call with argument: 'foo(10)'") {
        Parser parser = makeParser("foo(10)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* call = dynamic_cast<const CallExpr*>(expr.get());
        REQUIRE(call != nullptr);
        CHECK(call->calleeName() == "foo");
        REQUIRE(call->argumentCount() == 1);
    }

    SECTION("Function call with binary argument: 'foo(x + 1)'") {
        Parser parser = makeParser("foo(x + 1)");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* call = dynamic_cast<const CallExpr*>(expr.get());
        REQUIRE(call != nullptr);
        CHECK(call->calleeName() == "foo");
    }

    SECTION("Parenthesized precedence: '(a + b) * c'") {
        Parser parser = makeParser("(a + b) * c");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* mulBin = dynamic_cast<const BinaryExpr*>(expr.get());
        REQUIRE(mulBin != nullptr);
        CHECK(mulBin->op() == "*");

        auto* addBin = dynamic_cast<const BinaryExpr*>(mulBin->left());
        REQUIRE(addBin != nullptr);
        CHECK(addBin->op() == "+");
    }

    SECTION("Indexed expression in binary operations: 'numbers[i] + 10'") {
        Parser parser = makeParser("numbers[i] + 10");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);
        CHECK_FALSE(parser.hasError());

        auto* bin = dynamic_cast<const BinaryExpr*>(expr.get());
        REQUIRE(bin != nullptr);
        CHECK(bin->op() == "+");

        auto* leftIdx = dynamic_cast<const IndexExpr*>(bin->left());
        REQUIRE(leftIdx != nullptr);

        auto* rightLit = dynamic_cast<const LiteralExpression*>(bin->right());
        REQUIRE(rightLit != nullptr);
        CHECK(rightLit->value() == "10");
    }
}

TEST_CASE("ArrayIndexing: ASTPrinter output verification", "[parser][expr][array][printer]") {
    SECTION("numbers[5] ASTPrinter output") {
        Parser parser = makeParser("numbers[5]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        ASTPrinter printer;
        std::string tree = printer.print(expr.get());

        std::string expected =
            "IndexExpr\n"
            "  Base:\n"
            "    IdentifierExpression: numbers\n"
            "  Index:\n"
            "    LiteralExpression (Integer): 5\n";

        CHECK(tree == expected);
    }

    SECTION("numbers[i + 1] ASTPrinter output") {
        Parser parser = makeParser("numbers[i + 1]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        ASTPrinter printer;
        std::string tree = printer.print(expr.get());

        CHECK(tree.find("IndexExpr\n") == 0);
        CHECK(tree.find("IdentifierExpression: numbers") != std::string::npos);
        CHECK(tree.find("BinaryExpr: +") != std::string::npos);
        CHECK(tree.find("IdentifierExpression: i") != std::string::npos);
        CHECK(tree.find("LiteralExpression (Integer): 1") != std::string::npos);
    }

    SECTION("IndexExpr::toString() format") {
        Parser parser = makeParser("arr[idx]");
        auto expr = parser.parseExpression();
        REQUIRE(expr != nullptr);

        std::string s = expr->toString();
        CHECK(s.find("IndexExpr(") != std::string::npos);
        CHECK(s.find("arr") != std::string::npos);
        CHECK(s.find("idx") != std::string::npos);
    }
}
