#include <catch2/catch.hpp>

#include "semantic/symbol_table.h"
#include "ast/var_decl.h"
#include "ast/function.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
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
// DAY 46 TESTS — Scope Management & Nested Symbol Table
// =============================================================================

// -----------------------------------------------------------------------------
// 1. pushScope() and popScope() / Nested Scope Stack
// -----------------------------------------------------------------------------
TEST_CASE("ScopeManagement: Scope stack push, pop, and depth", "[semantic][scope][stack]") {
    SymbolTable table;

    SECTION("Initial state has depth 1 (global scope, level 0)") {
        CHECK(table.scopeDepth() == 1);
        CHECK(table.currentScopeLevel() == 0);
        CHECK(table.scopeLevel() == 0);
        CHECK(table.isGlobalScope());
    }

    SECTION("pushScope increments depth and level") {
        table.pushScope();
        CHECK(table.scopeDepth() == 2);
        CHECK(table.currentScopeLevel() == 1);
        CHECK_FALSE(table.isGlobalScope());

        table.pushScope();
        CHECK(table.scopeDepth() == 3);
        CHECK(table.currentScopeLevel() == 2);

        table.pushScope();
        CHECK(table.scopeDepth() == 4);
        CHECK(table.currentScopeLevel() == 3);
    }

    SECTION("popScope decrements depth and level") {
        table.pushScope(); // depth 2
        table.pushScope(); // depth 3

        CHECK(table.popScope());
        CHECK(table.scopeDepth() == 2);
        CHECK(table.currentScopeLevel() == 1);

        CHECK(table.popScope());
        CHECK(table.scopeDepth() == 1);
        CHECK(table.currentScopeLevel() == 0);
        CHECK(table.isGlobalScope());
    }

    SECTION("Cannot pop the global scope") {
        CHECK(table.scopeDepth() == 1);
        CHECK_FALSE(table.popScope()); // Must return false and not crash
        CHECK(table.scopeDepth() == 1);
        CHECK(table.currentScopeLevel() == 0);
        CHECK(table.isGlobalScope());
    }

    SECTION("resetToGlobalScope resets any depth back to 1") {
        table.pushScope();
        table.pushScope();
        table.pushScope();
        CHECK(table.scopeDepth() == 4);

        table.resetToGlobalScope();
        CHECK(table.scopeDepth() == 1);
        CHECK(table.currentScopeLevel() == 0);
        CHECK(table.isGlobalScope());
    }

    SECTION("ScopeGuard RAII automatically manages scope entry and exit") {
        CHECK(table.scopeDepth() == 1);
        {
            ScopeGuard guard1(table);
            CHECK(table.scopeDepth() == 2);
            CHECK(table.currentScopeLevel() == 1);
            {
                ScopeGuard guard2(table);
                CHECK(table.scopeDepth() == 3);
                CHECK(table.currentScopeLevel() == 2);
            }
            CHECK(table.scopeDepth() == 2);
            CHECK(table.currentScopeLevel() == 1);
        }
        CHECK(table.scopeDepth() == 1);
        CHECK(table.currentScopeLevel() == 0);
    }
}

// -----------------------------------------------------------------------------
// 2. Identifier Lookup from Innermost to Outermost Scope
// -----------------------------------------------------------------------------
TEST_CASE("ScopeManagement: Identifier lookup from innermost to outermost", "[semantic][scope][lookup]") {
    SymbolTable table;

    // Level 0: Global declarations
    table.insert(Symbol::createVariable("g_val", Type::getInt()));
    table.insert(Symbol::createVariable("shared", Type::getInt()));

    SECTION("Global symbols visible in global scope") {
        CHECK(table.contains("g_val"));
        CHECK(table.lookup("g_val") != nullptr);
        CHECK(table.lookup("g_val")->typeKind() == TypeKind::Int);
        CHECK(table.lookup("g_val")->scopeLevel() == 0);
        CHECK(table.lookup("g_val")->isGlobal());
    }

    SECTION("Global symbols visible inside nested scopes") {
        table.pushScope(); // Level 1
        table.insert(Symbol::createVariable("l1_val", Type::getFloat()));

        // Both global and level 1 are visible
        CHECK(table.contains("g_val"));
        CHECK(table.contains("l1_val"));
        CHECK(table.lookup("g_val")->typeKind() == TypeKind::Int);
        CHECK(table.lookup("l1_val")->typeKind() == TypeKind::Float);
        CHECK(table.lookup("l1_val")->scopeLevel() == 1);
        CHECK_FALSE(table.lookup("l1_val")->isGlobal());

        table.pushScope(); // Level 2
        table.insert(Symbol::createVariable("l2_val", Type::getChar()));

        // All three levels visible
        CHECK(table.contains("g_val"));
        CHECK(table.contains("l1_val"));
        CHECK(table.contains("l2_val"));
        CHECK(table.lookup("g_val")->scopeLevel() == 0);
        CHECK(table.lookup("l1_val")->scopeLevel() == 1);
        CHECK(table.lookup("l2_val")->scopeLevel() == 2);

        table.pushScope(); // Level 3 (empty block)
        CHECK(table.contains("g_val"));
        CHECK(table.contains("l1_val"));
        CHECK(table.contains("l2_val"));
        CHECK(table.lookup("non_existent") == nullptr);
    }

    SECTION("Inner symbols NOT visible in outer scopes") {
        table.pushScope(); // Level 1
        table.insert(Symbol::createVariable("local_var", Type::getDouble()));
        CHECK(table.contains("local_var"));

        table.popScope(); // Back to Level 0
        CHECK_FALSE(table.contains("local_var"));
        CHECK(table.lookup("local_var") == nullptr);
    }
}

// -----------------------------------------------------------------------------
// 3. Inner-Scope Shadowing of Outer Declarations
// -----------------------------------------------------------------------------
TEST_CASE("ScopeManagement: Inner-scope shadowing of outer declarations", "[semantic][scope][shadowing]") {
    SymbolTable table;

    // Scope 0: Global int x = 10;
    table.insert(Symbol::createVariable("x", Type::getInt()));
    table.insert(Symbol::createVariable("y", Type::getInt()));

    REQUIRE(table.lookup("x") != nullptr);
    CHECK(table.lookup("x")->typeKind() == TypeKind::Int);
    CHECK(table.lookup("x")->scopeLevel() == 0);

    SECTION("Single-level shadowing") {
        table.pushScope(); // Scope 1
        // Shadow x with float x
        bool inserted = table.insert(Symbol::createVariable("x", Type::getFloat()));
        CHECK(inserted); // Allowed because it's in a different scope

        const Symbol* sym = table.lookup("x");
        REQUIRE(sym != nullptr);
        CHECK(sym->typeKind() == TypeKind::Float); // Inner x shadows outer x
        CHECK(sym->scopeLevel() == 1);

        // y was not shadowed, so global y is still visible
        const Symbol* symY = table.lookup("y");
        REQUIRE(symY != nullptr);
        CHECK(symY->typeKind() == TypeKind::Int);
        CHECK(symY->scopeLevel() == 0);

        // Scope-specific lookups can still reach global x
        CHECK(table.lookupCurrentScope("x")->typeKind() == TypeKind::Float);
        CHECK(table.lookupGlobal("x")->typeKind() == TypeKind::Int);
    }

    SECTION("Multi-level shadowing across 3 scopes (int -> float -> char)") {
        table.pushScope(); // Scope 1
        table.insert(Symbol::createVariable("x", Type::getFloat()));
        CHECK(table.lookup("x")->typeKind() == TypeKind::Float);
        CHECK(table.lookup("x")->scopeLevel() == 1);

        table.pushScope(); // Scope 2
        table.insert(Symbol::createVariable("x", Type::getChar()));
        CHECK(table.lookup("x")->typeKind() == TypeKind::Char);
        CHECK(table.lookup("x")->scopeLevel() == 2);

        // Unshadow step by step via popScope
        table.popScope(); // Back to Scope 1
        CHECK(table.lookup("x")->typeKind() == TypeKind::Float);
        CHECK(table.lookup("x")->scopeLevel() == 1);

        table.popScope(); // Back to Scope 0
        CHECK(table.lookup("x")->typeKind() == TypeKind::Int);
        CHECK(table.lookup("x")->scopeLevel() == 0);
    }

    SECTION("Shadowing with different symbol kinds (Variable shadows Function)") {
        table.insert(Symbol::createFunction("compute", Type::getInt(), {}, false, ParameterListKind::Void));
        CHECK(table.lookup("compute")->isFunction());

        table.pushScope(); // Inner scope
        table.insert(Symbol::createVariable("compute", Type::getDouble()));

        const Symbol* innerSym = table.lookup("compute");
        REQUIRE(innerSym != nullptr);
        CHECK(innerSym->isVariable());
        CHECK(innerSym->typeKind() == TypeKind::Double);
        CHECK(innerSym->scopeLevel() == 1);

        table.popScope();
        const Symbol* outerSym = table.lookup("compute");
        REQUIRE(outerSym != nullptr);
        CHECK(outerSym->isFunction());
        CHECK(outerSym->scopeLevel() == 0);
    }
}

// -----------------------------------------------------------------------------
// 4. Duplicate Declaration Rejection within the Same Scope
// -----------------------------------------------------------------------------
TEST_CASE("ScopeManagement: Duplicate declaration rejection within the same scope", "[semantic][scope][duplicate]") {
    SymbolTable table;

    SECTION("Reject duplicates in global scope") {
        CHECK(table.insert(Symbol::createVariable("a", Type::getInt())));
        CHECK_FALSE(table.insert(Symbol::createVariable("a", Type::getInt()))); // Duplicate
        CHECK_FALSE(table.insert(Symbol::createVariable("a", Type::getFloat()))); // Different type, same name
        CHECK_FALSE(table.insert(Symbol::createFunction("a", Type::getVoid(), {}, false, ParameterListKind::Void)));

        CHECK(table.size() == 1);
        CHECK(table.lookup("a")->typeKind() == TypeKind::Int);
    }

    SECTION("Reject duplicates in nested block scope") {
        table.pushScope(); // Scope 1

        CHECK(table.insert(Symbol::createVariable("local", Type::getInt())));
        CHECK_FALSE(table.insert(Symbol::createVariable("local", Type::getInt()))); // Duplicate
        CHECK_FALSE(table.insert(Symbol::createVariable("local", Type::getDouble())));

        CHECK(table.currentScopeSize() == 1);
        CHECK(table.lookupCurrentScope("local")->typeKind() == TypeKind::Int);
    }

    SECTION("Outer declaration does NOT prevent declaration in new scope, but duplicate in new scope is rejected") {
        table.insert(Symbol::createVariable("x", Type::getInt())); // Scope 0

        table.pushScope(); // Scope 1
        CHECK(table.insert(Symbol::createVariable("x", Type::getFloat()))); // Allowed (shadowing)
        CHECK_FALSE(table.insert(Symbol::createVariable("x", Type::getChar()))); // Rejected (duplicate in scope 1)

        CHECK(table.lookup("x")->typeKind() == TypeKind::Float);
    }
}

// -----------------------------------------------------------------------------
// 5. Removal of Visibility when a Scope is Popped
// -----------------------------------------------------------------------------
TEST_CASE("ScopeManagement: Visibility removed when scope is popped", "[semantic][scope][pop_visibility]") {
    SymbolTable table;

    table.insert(Symbol::createVariable("global_keep", Type::getInt()));
    CHECK(table.size() == 1);

    table.pushScope(); // Scope 1
    table.insert(Symbol::createVariable("temp1", Type::getFloat()));
    table.insert(Symbol::createVariable("temp2", Type::getChar()));
    CHECK(table.size() == 3);
    CHECK(table.currentScopeSize() == 2);

    table.pushScope(); // Scope 2
    table.insert(Symbol::createVariable("temp3", Type::getDouble()));
    CHECK(table.size() == 4);
    CHECK(table.currentScopeSize() == 1);

    // Pop Scope 2
    CHECK(table.popScope());
    CHECK(table.size() == 3);
    CHECK_FALSE(table.contains("temp3"));
    CHECK(table.lookup("temp3") == nullptr);
    // temp1, temp2, global_keep still visible
    CHECK(table.contains("temp1"));
    CHECK(table.contains("temp2"));
    CHECK(table.contains("global_keep"));

    // Pop Scope 1
    CHECK(table.popScope());
    CHECK(table.size() == 1);
    CHECK_FALSE(table.contains("temp1"));
    CHECK_FALSE(table.contains("temp2"));
    CHECK(table.lookup("temp1") == nullptr);
    CHECK(table.lookup("temp2") == nullptr);
    // Only global_keep remains
    CHECK(table.contains("global_keep"));
    CHECK(table.lookup("global_keep") != nullptr);
}

// -----------------------------------------------------------------------------
// 6. AST Integration: Populating from AST Nodes across Scopes
// -----------------------------------------------------------------------------
TEST_CASE("ScopeManagement: AST VarDecl insertion with scopes", "[semantic][scope][ast]") {
    SymbolTable table;

    // Global variable parsed via VarDecl
    Parser p1 = makeParser("int count = 0;");
    auto d1 = p1.parseVarDecl();
    REQUIRE(d1 != nullptr);
    CHECK(table.insert(*d1));

    CHECK(table.lookup("count")->scopeLevel() == 0);
    CHECK(table.lookup("count")->astNode() == d1.get());

    // Enter local block scope
    table.pushScope();
    Parser p2 = makeParser("float count = 1.5;"); // Shadow count
    auto d2 = p2.parseVarDecl();
    REQUIRE(d2 != nullptr);
    CHECK(table.insert(*d2));

    CHECK(table.lookup("count")->scopeLevel() == 1);
    CHECK(table.lookup("count")->typeKind() == TypeKind::Float);
    CHECK(table.lookup("count")->astNode() == d2.get());

    table.popScope();
    CHECK(table.lookup("count")->scopeLevel() == 0);
    CHECK(table.lookup("count")->typeKind() == TypeKind::Int);
    CHECK(table.lookup("count")->astNode() == d1.get());
}

// -----------------------------------------------------------------------------
// 7. getAllVisibleSymbols() with Shadowing
// -----------------------------------------------------------------------------
TEST_CASE("ScopeManagement: getAllVisibleSymbols deduplicates shadowed symbols", "[semantic][scope][visible]") {
    SymbolTable table;

    table.insert(Symbol::createVariable("a", Type::getInt()));
    table.insert(Symbol::createVariable("b", Type::getInt()));

    table.pushScope();
    table.insert(Symbol::createVariable("b", Type::getFloat())); // shadows b
    table.insert(Symbol::createVariable("c", Type::getChar()));

    auto visible = table.getAllVisibleSymbols();
    CHECK(visible.size() == 3); // a, b, c (not two b's)

    // Verify b is the inner float b
    const Symbol* symB = nullptr;
    for (const auto* s : visible) {
        if (s->name() == "b") symB = s;
    }
    REQUIRE(symB != nullptr);
    CHECK(symB->typeKind() == TypeKind::Float);
    CHECK(symB->scopeLevel() == 1);
}
