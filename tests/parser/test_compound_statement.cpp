#include <catch2/catch.hpp>

#include "ast/ast.h"
#include "ast/compound_stmt.h"
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
// DAY 28 TESTS — Compound Statements ({ statement-list? })
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Basic Empty Compound Statements
// -----------------------------------------------------------------------------
TEST_CASE("CompoundStmt: empty block '{}'", "[parser][compound][stmt]") {
    SECTION("parseCompoundStatement directly") {
        Parser parser = makeParser("{}");
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        CHECK(block->empty());
        CHECK(block->size() == 0);
        CHECK(block->statementCount() == 0);
        CHECK(block->statements().empty());
        CHECK(block->getStatements().empty());
        CHECK(block->toString() == "CompoundStmt (0 statements)");
    }

    SECTION("parseStatement entry point") {
        Parser parser = makeParser("{}");
        auto stmt = parser.parseStatement();
        REQUIRE(stmt != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        auto* block = dynamic_cast<CompoundStmt*>(stmt.get());
        REQUIRE(block != nullptr);
        CHECK(block->empty());
        CHECK(block->statementCount() == 0);
    }

    SECTION("with whitespace and newlines '{\\n\\n}'") {
        Parser parser = makeParser("{\n  \n}");
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        CHECK(block->empty());
    }
}

// -----------------------------------------------------------------------------
// 2. Nested Blocks
// -----------------------------------------------------------------------------
TEST_CASE("CompoundStmt: nested blocks", "[parser][compound][nesting]") {
    SECTION("single nested empty block '{ { } }'") {
        Parser parser = makeParser("{ { } }");
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        REQUIRE(block->statementCount() == 1);
        CHECK_FALSE(block->empty());

        auto* inner = dynamic_cast<CompoundStmt*>(block->statements()[0].get());
        REQUIRE(inner != nullptr);
        CHECK(inner->empty());
        CHECK(inner->statementCount() == 0);
        CHECK(inner->toString() == "CompoundStmt (0 statements)");
        CHECK(block->toString() == "CompoundStmt (1 statements)");
    }

    SECTION("three-level nested blocks '{ { { } } }'") {
        Parser parser = makeParser("{ { { } } }");
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        REQUIRE(block->statementCount() == 1);
        auto* l2 = dynamic_cast<CompoundStmt*>(block->statements()[0].get());
        REQUIRE(l2 != nullptr);
        REQUIRE(l2->statementCount() == 1);

        auto* l3 = dynamic_cast<CompoundStmt*>(l2->statements()[0].get());
        REQUIRE(l3 != nullptr);
        CHECK(l3->empty());
        CHECK(l3->statementCount() == 0);
    }

    SECTION("multiple sibling nested blocks '{ {} {} }'") {
        Parser parser = makeParser("{ {} {} }");
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        REQUIRE(block->statementCount() == 2);

        auto* first = dynamic_cast<CompoundStmt*>(block->statements()[0].get());
        REQUIRE(first != nullptr);
        CHECK(first->empty());

        auto* second = dynamic_cast<CompoundStmt*>(block->statements()[1].get());
        REQUIRE(second != nullptr);
        CHECK(second->empty());
    }

    SECTION("mixed sibling and nested blocks '{ { {} } {} { { {} } } }'") {
        Parser parser = makeParser("{ { {} } {} { { {} } } }");
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());

        REQUIRE(block->statementCount() == 3);

        // First sibling: { {} } -> 1 child
        auto* sib1 = dynamic_cast<CompoundStmt*>(block->statements()[0].get());
        REQUIRE(sib1 != nullptr);
        REQUIRE(sib1->statementCount() == 1);
        auto* sib1_inner = dynamic_cast<CompoundStmt*>(sib1->statements()[0].get());
        REQUIRE(sib1_inner != nullptr);
        CHECK(sib1_inner->empty());

        // Second sibling: {} -> 0 children
        auto* sib2 = dynamic_cast<CompoundStmt*>(block->statements()[1].get());
        REQUIRE(sib2 != nullptr);
        CHECK(sib2->empty());

        // Third sibling: { { {} } } -> 1 child which has 1 child
        auto* sib3 = dynamic_cast<CompoundStmt*>(block->statements()[2].get());
        REQUIRE(sib3 != nullptr);
        REQUIRE(sib3->statementCount() == 1);
        auto* sib3_inner1 = dynamic_cast<CompoundStmt*>(sib3->statements()[0].get());
        REQUIRE(sib3_inner1 != nullptr);
        REQUIRE(sib3_inner1->statementCount() == 1);
        auto* sib3_inner2 = dynamic_cast<CompoundStmt*>(sib3_inner1->statements()[0].get());
        REQUIRE(sib3_inner2 != nullptr);
        CHECK(sib3_inner2->empty());
    }
}

// -----------------------------------------------------------------------------
// 3. Source Location Tracking
// -----------------------------------------------------------------------------
TEST_CASE("CompoundStmt: source location tracking", "[parser][compound][location]") {
    SECTION("top-level block location") {
        Parser parser = makeParser("\n  {}");
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK(block->location().line == 2);
        CHECK(block->location().column == 3);
    }

    SECTION("nested block locations") {
        Parser parser = makeParser("{\n    {}\n}");
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK(block->location().line == 1);
        CHECK(block->location().column == 1);

        REQUIRE(block->statementCount() == 1);
        auto* inner = block->statements()[0].get();
        CHECK(inner->location().line == 2);
        CHECK(inner->location().column == 5);
    }
}

// -----------------------------------------------------------------------------
// 4. ASTPrinter Output Verification
// -----------------------------------------------------------------------------
TEST_CASE("CompoundStmt: AST printer formatting", "[parser][compound][printer]") {
    SECTION("empty block format") {
        Parser parser = makeParser("{}");
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        std::string expected = "CompoundStmt\n";
        CHECK(dumpAST(block.get()) == expected);
    }

    SECTION("nested block format") {
        Parser parser = makeParser("{ { } }");
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        std::string expected =
            "CompoundStmt\n"
            "  CompoundStmt\n";
        CHECK(dumpAST(block.get()) == expected);
    }

    SECTION("sibling blocks format") {
        Parser parser = makeParser("{ {} {} }");
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);

        std::string expected =
            "CompoundStmt\n"
            "  CompoundStmt\n"
            "  CompoundStmt\n";
        CHECK(dumpAST(block.get()) == expected);
    }
}

// -----------------------------------------------------------------------------
// 5. Negative Tests and Error Reporting
// -----------------------------------------------------------------------------
TEST_CASE("CompoundStmt: error reporting and malformed blocks", "[parser][compound][error]") {
    SECTION("missing closing brace: '{'") {
        Parser parser = makeParser("{");
        auto block = parser.parseCompoundStatement();
        CHECK(block == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '}'") != std::string::npos);
    }

    SECTION("missing outer closing brace: '{ {}'") {
        Parser parser = makeParser("{ {}");
        auto block = parser.parseCompoundStatement();
        CHECK(block == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '}'") != std::string::npos);
    }

    SECTION("missing inner closing brace: '{ {'") {
        Parser parser = makeParser("{ {");
        auto block = parser.parseCompoundStatement();
        CHECK(block == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '}'") != std::string::npos);
    }

    SECTION("unexpected closing brace on parseCompoundStatement: '}'") {
        Parser parser = makeParser("}");
        auto block = parser.parseCompoundStatement();
        CHECK(block == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '{'") != std::string::npos);
    }

    SECTION("unexpected closing brace on parseStatement: '}'") {
        Parser parser = makeParser("}");
        auto stmt = parser.parseStatement();
        CHECK(stmt == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement, found '}'") != std::string::npos);
    }

    SECTION("empty token stream") {
        Parser parser = makeParser("");
        auto block = parser.parseCompoundStatement();
        CHECK(block == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected '{'") != std::string::npos);
    }

    SECTION("unsupported statement token inside block: '{ int }'") {
        Parser parser = makeParser("{ int }");
        auto block = parser.parseCompoundStatement();
        CHECK(block == nullptr);
        CHECK(parser.hasError());
        CHECK(parser.errorMessage().find("Expected statement, found 'int'") != std::string::npos);
    }

    SECTION("trailing unexpected tokens after valid compound statement") {
        Parser parser = makeParser("{} extra");
        auto block = parser.parseCompoundStatement();
        REQUIRE(block != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK_FALSE(parser.isAtEnd());
        CHECK(parser.peek().lexeme == "extra");
    }
}

// -----------------------------------------------------------------------------
// 6. Function-Body and AST Compatibility
// -----------------------------------------------------------------------------
TEST_CASE("CompoundStmt: Function-body AST compatibility", "[parser][compound][function]") {
    SECTION("Function AST node correctly stores CompoundStmt in body") {
        auto fn = std::make_unique<Function>("main", "int", SourceLocation{1, 1}, true);
        CHECK(fn->body().empty());

        auto body = std::make_unique<CompoundStmt>(SourceLocation{1, 12});
        body->addStatement(std::make_unique<CompoundStmt>(SourceLocation{1, 14}));

        fn->addBodyStatement(std::move(body));
        REQUIRE(fn->bodyStatementCount() == 1);

        auto* storedBody = dynamic_cast<const CompoundStmt*>(fn->body()[0].get());
        REQUIRE(storedBody != nullptr);
        REQUIRE(storedBody->statementCount() == 1);

        // Verify ASTPrinter formatting on function with CompoundStmt body
        std::string tree = dumpAST(fn.get());
        CHECK(tree.find("Function (definition): int main") != std::string::npos);
        CHECK(tree.find("Body:\n") != std::string::npos);
        CHECK(tree.find("CompoundStmt\n") != std::string::npos);
    }

    SECTION("Existing function declarations parse definitions preserving existing behavior") {
        Parser parser = makeParser("void test(void) { { int x = 1; } }");
        auto fn = parser.parseDeclaration();
        REQUIRE(fn != nullptr);
        CHECK_FALSE(parser.hasError());
        CHECK(parser.isAtEnd());
        CHECK(fn->name() == "test");
        CHECK(fn->isDefinition());
    }
}

