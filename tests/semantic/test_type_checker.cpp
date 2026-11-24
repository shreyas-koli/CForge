#include <catch2/catch.hpp>

#include "semantic/type_checker.h"
#include "semantic/symbol_table.h"
#include "ast/assign_expr.h"
#include "ast/binary_expr.h"
#include "ast/compound_stmt.h"
#include "ast/conditional_expr.h"
#include "ast/for_stmt.h"
#include "ast/function.h"
#include "ast/identifier_expression.h"
#include "ast/if_stmt.h"
#include "ast/literal_expression.h"
#include "ast/program.h"
#include "ast/unary_expr.h"
#include "ast/var_decl.h"
#include "ast/while_stmt.h"
#include "ast/do_while_stmt.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "types/conversions.h"
#include "types/type.h"

#include <memory>
#include <string>
#include <vector>

// Helper to construct a Parser from a source string
static Parser makeParser(const std::string& src) {
    Lexer lexer(src);
    return Parser(lexer.tokenize());
}

// =============================================================================
// DAY 49 TESTS — Expression Type Checking & Inference
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Integer Arithmetic Inference
// -----------------------------------------------------------------------------
TEST_CASE("TypeChecker: 1 - Integer arithmetic inference", "[semantic][type_check][arithmetic][int]") {
    SymbolTable table;
    table.insert(Symbol::createVariable("a", Type::getInt()));
    table.insert(Symbol::createVariable("b", Type::getInt()));
    table.insert(Symbol::createVariable("c1", Type::getChar()));
    table.insert(Symbol::createVariable("c2", Type::getChar()));

    TypeChecker checker(table);

    SECTION("int + int yields int") {
        Parser p = makeParser("a + b");
        auto expr = p.parseExpression();
        auto res = checker.inferType(*expr);
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK(res->isInt());
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("int - int, int * int, int / int yield int") {
        for (const char* opStr : {"a - b", "a * b", "a / b"}) {
            Parser p = makeParser(opStr);
            auto res = checker.inferType(*p.parseExpression());
            REQUIRE(res != nullptr);
            CHECK(res->kind() == TypeKind::Int);
        }
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("int % int yields int") {
        Parser p = makeParser("a % b");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("char + char promotes to int (integer promotion)") {
        Parser p = makeParser("c1 + c2");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("char + int yields int") {
        Parser p = makeParser("c1 + a");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Bitwise operators on integers yield int") {
        for (const char* opStr : {"a & b", "a | b", "a ^ b", "a << 2", "a >> 1"}) {
            Parser p = makeParser(opStr);
            auto res = checker.inferType(*p.parseExpression());
            REQUIRE(res != nullptr);
            CHECK(res->kind() == TypeKind::Int);
        }
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 2. Mixed Integer and Floating-Point Arithmetic
// -----------------------------------------------------------------------------
TEST_CASE("TypeChecker: 2 - Mixed integer and floating-point arithmetic", "[semantic][type_check][mixed]") {
    SymbolTable table;
    table.insert(Symbol::createVariable("i", Type::getInt()));
    table.insert(Symbol::createVariable("ch", Type::getChar()));
    table.insert(Symbol::createVariable("flt", Type::getFloat()));
    table.insert(Symbol::createVariable("dbl", Type::getDouble()));

    TypeChecker checker(table);

    SECTION("int + float yields float (roadmap example)") {
        Parser p = makeParser("i + flt");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Float);
        CHECK(res->isFloat());
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("float + int yields float") {
        Parser p = makeParser("flt + i");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Float);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("char + float yields float") {
        Parser p = makeParser("ch + flt");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Float);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("int + double yields double") {
        Parser p = makeParser("i + dbl");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Double);
        CHECK(res->isDouble());
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("float + double yields double") {
        Parser p = makeParser("flt + dbl");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Double);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Chained arithmetic: i + flt * dbl yields double") {
        Parser p = makeParser("i + flt * dbl");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Double);
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 3. Valid Relational Comparisons
// -----------------------------------------------------------------------------
TEST_CASE("TypeChecker: 3 - Valid relational comparisons yield int", "[semantic][type_check][relational]") {
    SymbolTable table;
    table.insert(Symbol::createVariable("i", Type::getInt()));
    table.insert(Symbol::createVariable("flt", Type::getFloat()));
    table.insert(Symbol::createVariable("dbl", Type::getDouble()));
    table.insert(Symbol::createVariable("ch", Type::getChar()));

    TypeChecker checker(table);

    SECTION("int < int yields int") {
        Parser p = makeParser("i < 10");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("int <= float yields int") {
        Parser p = makeParser("i <= flt");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("float > double yields int") {
        Parser p = makeParser("flt > dbl");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("char >= int yields int") {
        Parser p = makeParser("ch >= i");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 4. Valid Equality Comparisons
// -----------------------------------------------------------------------------
TEST_CASE("TypeChecker: 4 - Valid equality comparisons yield int", "[semantic][type_check][equality]") {
    SymbolTable table;
    table.insert(Symbol::createVariable("a", Type::getInt()));
    table.insert(Symbol::createVariable("b", Type::getFloat()));
    table.insert(Symbol::createVariable("c", Type::getChar()));
    table.insert(Symbol::createVariable("d", Type::getDouble()));

    TypeChecker checker(table);

    SECTION("int == int yields int") {
        Parser p = makeParser("a == 42");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("int != float yields int") {
        Parser p = makeParser("a != b");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("char == char yields int") {
        Parser p = makeParser("c == 'Z'");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("double == int yields int") {
        Parser p = makeParser("d == a");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 5. Unary Expressions
// -----------------------------------------------------------------------------
TEST_CASE("TypeChecker: 5 - Unary expression types", "[semantic][type_check][unary]") {
    SymbolTable table;
    table.insert(Symbol::createVariable("i", Type::getInt()));
    table.insert(Symbol::createVariable("f", Type::getFloat()));
    table.insert(Symbol::createVariable("d", Type::getDouble()));
    table.insert(Symbol::createVariable("c", Type::getChar()));

    TypeChecker checker(table);

    SECTION("Unary plus and minus preserve arithmetic kind with char promotion") {
        Parser p1 = makeParser("+i");
        CHECK(checker.inferType(*p1.parseExpression())->kind() == TypeKind::Int);

        Parser p2 = makeParser("-f");
        CHECK(checker.inferType(*p2.parseExpression())->kind() == TypeKind::Float);

        Parser p3 = makeParser("-d");
        CHECK(checker.inferType(*p3.parseExpression())->kind() == TypeKind::Double);

        Parser p4 = makeParser("-c");
        CHECK(checker.inferType(*p4.parseExpression())->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Bitwise NOT ~ yields int for integer operands") {
        Parser p1 = makeParser("~i");
        CHECK(checker.inferType(*p1.parseExpression())->kind() == TypeKind::Int);

        Parser p2 = makeParser("~c");
        CHECK(checker.inferType(*p2.parseExpression())->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Logical NOT ! yields int for all scalar types") {
        for (const char* opStr : {"!i", "!f", "!d", "!c"}) {
            Parser p = makeParser(opStr);
            auto res = checker.inferType(*p.parseExpression());
            REQUIRE(res != nullptr);
            CHECK(res->kind() == TypeKind::Int);
        }
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Prefix ++ and -- yield operand type for modifiable lvalue") {
        Parser p1 = makeParser("++i");
        CHECK(checker.inferType(*p1.parseExpression())->kind() == TypeKind::Int);

        Parser p2 = makeParser("--f");
        CHECK(checker.inferType(*p2.parseExpression())->kind() == TypeKind::Float);
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 6. Assignment Compatibility
// -----------------------------------------------------------------------------
TEST_CASE("TypeChecker: 6 - Assignment compatibility and result types", "[semantic][type_check][assign]") {
    SymbolTable table;
    table.insert(Symbol::createVariable("x", Type::getInt()));
    table.insert(Symbol::createVariable("y", Type::getFloat()));
    table.insert(Symbol::createVariable("z", Type::getDouble()));
    table.insert(Symbol::createVariable("ch", Type::getChar()));

    TypeChecker checker(table);

    SECTION("int = int is valid, yields int") {
        Parser p = makeParser("x = 42");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("int = float is valid per Day 39 implicit conversions (truncating)") {
        Parser p = makeParser("x = y");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("float = int is valid (widening/lossless or lossy conversion)") {
        Parser p = makeParser("y = x");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Float);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("double = float is valid") {
        Parser p = makeParser("z = y");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Double);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("char = int is valid (narrowing integer conversion)") {
        Parser p = makeParser("ch = x");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Char);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Compound assignment x += y is valid, yields left operand type") {
        Parser p = makeParser("x += y");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Compound assignment y *= z is valid, yields float") {
        Parser p = makeParser("y *= z");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Float);
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 7. Invalid Arithmetic Operations
// -----------------------------------------------------------------------------
TEST_CASE("TypeChecker: 7 - Invalid arithmetic operations are diagnosed", "[semantic][type_check][invalid_arith]") {
    SymbolTable table;
    table.insert(Symbol::createVariable("i", Type::getInt()));
    table.insert(Symbol::createVariable("f", Type::getFloat()));
    table.insert(Symbol::createVariable("d", Type::getDouble()));
    table.insert(Symbol::createVariable("v", Type::getVoid()));

    TypeChecker checker(table);

    SECTION("void operand in binary addition: v + i") {
        Parser p = makeParser("v + i");
        auto res = checker.inferType(*p.parseExpression());
        CHECK(res == nullptr);
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() >= 1);
        CHECK(checker.diagnostics()[0].message.find("invalid operands to binary operator '+'") != std::string::npos);
    }

    SECTION("void operand in binary subtraction: i - v") {
        Parser p = makeParser("i - v");
        CHECK(checker.inferType(*p.parseExpression()) == nullptr);
        CHECK(checker.hasErrors());
    }

    SECTION("Floating-point modulo: f % i is invalid") {
        Parser p = makeParser("f % i");
        CHECK(checker.inferType(*p.parseExpression()) == nullptr);
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() >= 1);
        CHECK(checker.diagnostics()[0].message.find("invalid operands to binary operator '%'") != std::string::npos);
    }

    SECTION("Double modulo: i % d is invalid") {
        Parser p = makeParser("i % d");
        CHECK(checker.inferType(*p.parseExpression()) == nullptr);
        CHECK(checker.hasErrors());
    }

    SECTION("Bitwise AND on float: f & i is invalid") {
        Parser p = makeParser("f & i");
        CHECK(checker.inferType(*p.parseExpression()) == nullptr);
        CHECK(checker.hasErrors());
        CHECK(checker.diagnostics()[0].message.find("invalid operands to binary operator '&'") != std::string::npos);
    }

    SECTION("Bitwise NOT on float: ~f is invalid") {
        Parser p = makeParser("~f");
        CHECK(checker.inferType(*p.parseExpression()) == nullptr);
        CHECK(checker.hasErrors());
        CHECK(checker.diagnostics()[0].message.find("invalid argument type 'float' to unary operator '~'") != std::string::npos);
    }

    SECTION("Unary plus on void: +v is invalid") {
        Parser p = makeParser("+v");
        CHECK(checker.inferType(*p.parseExpression()) == nullptr);
        CHECK(checker.hasErrors());
    }

    SECTION("Logical NOT on void: !v is invalid") {
        Parser p = makeParser("!v");
        CHECK(checker.inferType(*p.parseExpression()) == nullptr);
        CHECK(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 8. Invalid Assignment Compatibility
// -----------------------------------------------------------------------------
TEST_CASE("TypeChecker: 8 - Invalid assignment operations are diagnosed", "[semantic][type_check][invalid_assign]") {
    SymbolTable table;
    table.insert(Symbol::createVariable("x", Type::getInt()));
    table.insert(Symbol::createVariable("c_val", Type::getInt(true))); // const int
    table.insert(Symbol::createVariable("v", Type::getVoid()));

    TypeChecker checker(table);

    SECTION("Assignment of void to int: x = v is invalid") {
        Parser p = makeParser("x = v");
        CHECK(checker.inferType(*p.parseExpression()) == nullptr);
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() >= 1);
        CHECK(checker.diagnostics()[0].message.find("incompatible types in assignment") != std::string::npos);
    }

    SECTION("Assignment to const-qualified variable: c_val = 10 is invalid") {
        Parser p = makeParser("c_val = 10");
        CHECK(checker.inferType(*p.parseExpression()) == nullptr);
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() >= 1);
        CHECK(checker.diagnostics()[0].message.find("cannot assign to variable with const-qualified type 'const int'") != std::string::npos);
    }

    SECTION("Compound assignment to const-qualified variable: c_val += 5 is invalid") {
        Parser p = makeParser("c_val += 5");
        CHECK(checker.inferType(*p.parseExpression()) == nullptr);
        CHECK(checker.hasErrors());
        CHECK(checker.diagnostics()[0].message.find("cannot assign to variable with const-qualified type") != std::string::npos);
    }

    SECTION("Prefix increment on const-qualified variable: ++c_val is invalid") {
        Parser p = makeParser("++c_val");
        CHECK(checker.inferType(*p.parseExpression()) == nullptr);
        CHECK(checker.hasErrors());
        CHECK(checker.diagnostics()[0].message.find("cannot modify constant value of type 'const int'") != std::string::npos);
    }

    SECTION("Assignment to non-lvalue literal: 10 = x is invalid") {
        Parser p = makeParser("10 = x");
        CHECK(checker.inferType(*p.parseExpression()) == nullptr);
        CHECK(checker.hasErrors());
        CHECK(checker.diagnostics()[0].message.find("expression is not assignable") != std::string::npos);
    }
}

// -----------------------------------------------------------------------------
// 9. Conditional Expression Types
// -----------------------------------------------------------------------------
TEST_CASE("TypeChecker: 9 - Conditional expression typing", "[semantic][type_check][conditional]") {
    SymbolTable table;
    table.insert(Symbol::createVariable("i", Type::getInt()));
    table.insert(Symbol::createVariable("flt", Type::getFloat()));
    table.insert(Symbol::createVariable("dbl", Type::getDouble()));
    table.insert(Symbol::createVariable("ch", Type::getChar()));
    table.insert(Symbol::createVariable("v", Type::getVoid()));

    TypeChecker checker(table);

    SECTION("1 ? int : int yields int") {
        Parser p = makeParser("1 ? i : i");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("1 ? int : float yields float") {
        Parser p = makeParser("1 ? i : flt");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Float);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("1 ? float : double yields double") {
        Parser p = makeParser("1 ? flt : dbl");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Double);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("1 ? char : char yields int (promoted common type)") {
        Parser p = makeParser("1 ? ch : ch");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Condition with void type is rejected") {
        Parser p = makeParser("v ? i : flt");
        CHECK(checker.inferType(*p.parseExpression()) == nullptr);
        CHECK(checker.hasErrors());
        CHECK(checker.diagnostics()[0].message.find("condition in conditional expression must have scalar type") != std::string::npos);
    }

    SECTION("Incompatible branches (void vs int) are rejected") {
        Parser p = makeParser("1 ? v : i");
        CHECK(checker.inferType(*p.parseExpression()) == nullptr);
        CHECK(checker.hasErrors());
        CHECK(checker.diagnostics()[0].message.find("incompatible operand types in conditional expression") != std::string::npos);
    }

    SECTION("Both branches void is valid and yields void") {
        Parser p = makeParser("1 ? v : v");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Void);
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 10. Literal Expression Types & Floating-Point Literal Suffix Verification
// -----------------------------------------------------------------------------
TEST_CASE("TypeChecker: 10 - Literal expression types", "[semantic][type_check][literals]") {
    TypeChecker checker;

    SECTION("Integer literal yields int") {
        Parser p = makeParser("12345");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK(res->isInt());
    }

    SECTION("Char literal yields char") {
        Parser p = makeParser("'c'");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Char);
        CHECK(res->isChar());
    }

    SECTION("1. Unsuffixed float literal '3.14' must infer double") {
        Parser p = makeParser("3.14");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);
        auto res = checker.inferType(*expr);
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Double);
        CHECK(res->isDouble());
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("2. Float literal with 'f' suffix '3.14f' must infer float") {
        Parser p = makeParser("3.14f");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);
        auto res = checker.inferType(*expr);
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Float);
        CHECK(res->isFloat());
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("3. Float literal with 'F' suffix '3.14F' must infer float") {
        Parser p = makeParser("3.14F");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);
        auto res = checker.inferType(*expr);
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Float);
        CHECK(res->isFloat());
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("4. Float literal with 'L' or 'l' suffix '3.14L' must not silently infer double") {
        // CForge does not support 'long double' (only Float and Double in TypeKind).
        // Therefore '3.14L' must report an error and not silently infer double.
        Parser p1 = makeParser("3.14L");
        auto expr1 = p1.parseExpression();
        REQUIRE(expr1 != nullptr);
        auto res1 = checker.inferType(*expr1);
        CHECK(res1 == nullptr);
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() >= 1);
        CHECK(checker.diagnostics()[0].message.find("unsupported literal suffix 'L'") != std::string::npos);
        CHECK(checker.diagnostics()[0].message.find("'long double' is not supported") != std::string::npos);

        // Lowercase 'l' suffix must likewise not silently infer double
        checker.clear();
        Parser p2 = makeParser("3.14l");
        auto expr2 = p2.parseExpression();
        REQUIRE(expr2 != nullptr);
        auto res2 = checker.inferType(*expr2);
        CHECK(res2 == nullptr);
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() >= 1);
        CHECK(checker.diagnostics()[0].message.find("unsupported literal suffix 'l'") != std::string::npos);
        CHECK(checker.diagnostics()[0].message.find("'long double' is not supported") != std::string::npos);
    }

    SECTION("5. Mixed float literal and double literal '2.5f + 3.5' must infer double") {
        Parser p = makeParser("2.5f + 3.5");
        auto expr = p.parseExpression();
        REQUIRE(expr != nullptr);
        auto res = checker.inferType(*expr);
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Double);
        CHECK(res->isDouble());
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 11. Identifier Expression Types & Avoid Cascading Diagnostics
// -----------------------------------------------------------------------------
TEST_CASE("TypeChecker: 11 - Identifier types and error suppression for undeclared", "[semantic][type_check][identifier]") {
    SymbolTable table;
    table.insert(Symbol::createVariable("known_int", Type::getInt()));
    table.insert(Symbol::createVariable("known_float", Type::getFloat()));
    table.insert(Symbol::createVariable("known_const", Type::getInt(true)));

    TypeChecker checker(table);

    SECTION("Declared identifiers return correct type") {
        Parser p1 = makeParser("known_int");
        CHECK(checker.inferType(*p1.parseExpression())->kind() == TypeKind::Int);

        Parser p2 = makeParser("known_float");
        CHECK(checker.inferType(*p2.parseExpression())->kind() == TypeKind::Float);

        Parser p3 = makeParser("known_const");
        auto constType = checker.inferType(*p3.parseExpression());
        REQUIRE(constType != nullptr);
        CHECK(constType->kind() == TypeKind::Int);
        CHECK(constType->isConst());
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Undeclared identifier returns nullptr without cascading type error") {
        Parser p = makeParser("missing_var");
        auto res = checker.inferType(*p.parseExpression());
        CHECK(res == nullptr);
        // Requirement 5: Do not report type errors for identifiers that cannot be resolved
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
    }

    SECTION("Expression containing undeclared identifier suppresses cascading errors") {
        Parser p = makeParser("missing_var + 10");
        auto res = checker.inferType(*p.parseExpression());
        CHECK(res == nullptr);
        CHECK_FALSE(checker.hasErrors());
    }
}

// -----------------------------------------------------------------------------
// 12. Existing Const-Qualified Type Behavior
// -----------------------------------------------------------------------------
TEST_CASE("TypeChecker: 12 - Const-qualified type behavior in expressions", "[semantic][type_check][const]") {
    SymbolTable table;
    table.insert(Symbol::createVariable("c_int", Type::getInt(true))); // const int
    table.insert(Symbol::createVariable("c_flt", Type::getFloat(true))); // const float

    TypeChecker checker(table);

    SECTION("Reading const variable in arithmetic yields unqualified rvalue type") {
        Parser p = makeParser("c_int + 5");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Int);
        CHECK_FALSE(res->isConst()); // rvalues are not const-qualified
    }

    SECTION("Mixed const int and const float yields unqualified float") {
        Parser p = makeParser("c_int + c_flt");
        auto res = checker.inferType(*p.parseExpression());
        REQUIRE(res != nullptr);
        CHECK(res->kind() == TypeKind::Float);
        CHECK_FALSE(res->isConst());
    }

    SECTION("Cannot assign to const variable") {
        Parser p = makeParser("c_int = 100");
        CHECK(checker.inferType(*p.parseExpression()) == nullptr);
        CHECK(checker.hasErrors());
        CHECK(checker.diagnostics()[0].message.find("cannot assign to variable with const-qualified type 'const int'") != std::string::npos);
    }
}

// -----------------------------------------------------------------------------
// 13. Regression Verification for Day 39 Conversion Rules
// -----------------------------------------------------------------------------
TEST_CASE("TypeChecker: 13 - Regression verification of Day 39 conversion rules", "[semantic][type_check][day39]") {
    auto tInt = Type::getInt();
    auto tFloat = Type::getFloat();
    auto tDouble = Type::getDouble();
    auto tChar = Type::getChar();
    auto tVoid = Type::getVoid();

    SECTION("Reusing isAssignable (isImplicitlyConvertible)") {
        CHECK(TypeChecker::isAssignable(tChar, tInt));
        CHECK(TypeChecker::isAssignable(tInt, tFloat));
        CHECK(TypeChecker::isAssignable(tFloat, tInt));
        CHECK(TypeChecker::isAssignable(tDouble, tFloat));
        CHECK_FALSE(TypeChecker::isAssignable(tVoid, tInt));
        CHECK_FALSE(TypeChecker::isAssignable(tInt, tVoid));
    }

    SECTION("Reusing isStrictlyImplicitConversion") {
        CHECK(TypeChecker::isStrictlyImplicitConversion(tChar, tInt));
        CHECK_FALSE(TypeChecker::isStrictlyImplicitConversion(tInt, tInt));
        CHECK(TypeChecker::isStrictlyImplicitConversion(tInt, tFloat));
    }

    SECTION("commonType static method verification") {
        CHECK(TypeChecker::commonType(tInt, tInt)->kind() == TypeKind::Int);
        CHECK(TypeChecker::commonType(tInt, tFloat)->kind() == TypeKind::Float);
        CHECK(TypeChecker::commonType(tFloat, tDouble)->kind() == TypeKind::Double);
        CHECK(TypeChecker::commonType(tChar, tChar)->kind() == TypeKind::Int);
        CHECK(TypeChecker::commonType(tInt, tVoid) == nullptr);
    }
}

// -----------------------------------------------------------------------------
// 14. Statement, Block, and Program Integration Tests
// -----------------------------------------------------------------------------
TEST_CASE("TypeChecker: 14 - Statement and program integration", "[semantic][type_check][integration]") {
    SECTION("Variable declaration with compatible initializer passes") {
        const std::string src =
            "{\n"
            "    int a = 10;\n"
            "    float b = a + 2.5;\n"
            "    double c = b;\n"
            "}";

        Parser p = makeParser(src);
        auto block = p.parseCompoundStatement();
        REQUIRE(block != nullptr);

        TypeChecker checker;
        CHECK(checker.check(*block));
        CHECK_FALSE(checker.hasErrors());
    }

    SECTION("Variable declaration with incompatible initializer is rejected") {
        SymbolTable table;
        table.insert(Symbol::createVariable("void_val", Type::getVoid()));

        TypeChecker checker(table);
        Parser p = makeParser("int x = void_val;");
        auto decl = p.parseVarDecl();
        REQUIRE(decl != nullptr);

        CHECK_FALSE(checker.check(*decl));
        CHECK(checker.hasErrors());
        REQUIRE(checker.errorCount() >= 1);
        CHECK(checker.diagnostics()[0].message.find("incompatible types in initialization") != std::string::npos);
    }

    SECTION("IfStmt with valid condition passes, invalid condition fails") {
        TypeChecker checker;

        Parser p1 = makeParser("10 > 5");
        auto ifGood = std::make_unique<IfStmt>(p1.parseExpression(), std::make_unique<CompoundStmt>());
        CHECK(checker.check(*ifGood));

        SymbolTable table;
        table.insert(Symbol::createVariable("v", Type::getVoid()));
        TypeChecker checkerBad(table);

        Parser p2 = makeParser("v");
        auto ifBad = std::make_unique<IfStmt>(p2.parseExpression(), std::make_unique<CompoundStmt>());
        CHECK_FALSE(checkerBad.check(*ifBad));
        CHECK(checkerBad.hasErrors());
    }

    SECTION("Convenience APIs checkTypes and inferExpressionType") {
        Parser p = makeParser("42 + 8");
        auto expr = p.parseExpression();

        auto resType = inferExpressionType(*expr);
        REQUIRE(resType != nullptr);
        CHECK(resType->kind() == TypeKind::Int);

        auto diags = checkTypes(*expr);
        CHECK(diags.empty());
    }

    SECTION("clear() resets diagnostics and symbol table state") {
        TypeChecker checker;
        Parser p = makeParser("10 = 20");
        checker.check(*p.parseExpression());
        CHECK(checker.hasErrors());

        checker.clear();
        CHECK_FALSE(checker.hasErrors());
        CHECK(checker.errorCount() == 0);
        CHECK(checker.symbolTable().empty());
    }
}
