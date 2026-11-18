#include <catch2/catch.hpp>

#include "semantic/symbol_table.h"
#include "ast/ast.h"
#include "ast/var_decl.h"
#include "ast/function.h"
#include "ast/program.h"
#include "ast/compound_stmt.h"
#include "ast/storage_class.h"
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
// DAY 45 TESTS — Symbol Table & Symbol Insertion
// =============================================================================

// -----------------------------------------------------------------------------
// 1. Symbol Data Structure Unit Tests
// -----------------------------------------------------------------------------
TEST_CASE("Symbol: Construction and attribute accessors", "[semantic][symbol]") {
    SECTION("Variable symbol construction and accessors") {
        auto intType = Type::getInt(false, false);
        SourceLocation loc{10, 5};
        Symbol sym("my_var", SymbolKind::Variable, intType, loc, StorageClass::Static);

        CHECK(sym.name() == "my_var");
        CHECK(sym.getName() == "my_var");
        CHECK(sym.kind() == SymbolKind::Variable);
        CHECK(sym.getKind() == SymbolKind::Variable);
        CHECK(sym.isVariable());
        CHECK_FALSE(sym.isFunction());

        REQUIRE(sym.type() != nullptr);
        CHECK(sym.type()->kind() == TypeKind::Int);
        CHECK(sym.typeKind() == TypeKind::Int);
        CHECK(sym.typeShared() == intType);
        CHECK_FALSE(sym.isConst());
        CHECK_FALSE(sym.isVolatile());

        CHECK(sym.storageClass() == StorageClass::Static);
        CHECK(sym.getStorageClass() == StorageClass::Static);
        CHECK(sym.isStatic());
        CHECK_FALSE(sym.isExtern());
        CHECK_FALSE(sym.isRegister());

        CHECK(sym.location().line == 10);
        CHECK(sym.location().column == 5);
        CHECK(sym.astNode() == nullptr);

        CHECK(sym.toString().find("my_var") != std::string::npos);
        CHECK(sym.toString().find("Variable") != std::string::npos);
    }

    SECTION("Qualified variable symbol (const volatile)") {
        auto cvDouble = Type::getDouble(true, true);
        Symbol sym("cv_val", SymbolKind::Variable, cvDouble, {1, 1}, StorageClass::Extern);

        CHECK(sym.isConst());
        CHECK(sym.isVolatile());
        CHECK(sym.isExtern());
        CHECK(sym.typeKind() == TypeKind::Double);
    }

    SECTION("Function symbol construction and accessors") {
        auto retType = Type::getInt();
        auto p1Type = Type::getInt();
        auto p2Type = Type::getFloat();

        Symbol fnSym = Symbol::createFunction(
            "compute",
            retType,
            {p1Type, p2Type},
            true, // isDefinition
            ParameterListKind::Typed,
            {15, 1},
            StorageClass::None
        );

        CHECK(fnSym.name() == "compute");
        CHECK(fnSym.isFunction());
        CHECK_FALSE(fnSym.isVariable());

        REQUIRE(fnSym.returnType() != nullptr);
        CHECK(fnSym.returnType()->kind() == TypeKind::Int);
        CHECK(fnSym.returnTypeKind() == TypeKind::Int);

        CHECK(fnSym.isDefinition());
        CHECK_FALSE(fnSym.isPrototype());
        CHECK_FALSE(fnSym.isDeclaration());

        CHECK(fnSym.parameterListKind() == ParameterListKind::Typed);
        CHECK_FALSE(fnSym.hasVoidParameterList());
        CHECK_FALSE(fnSym.hasUnspecifiedParameters());

        REQUIRE(fnSym.parameterCount() == 2);
        CHECK(fnSym.parameter(0) == p1Type.get());
        CHECK(fnSym.parameter(1) == p2Type.get());
        CHECK(fnSym.parameterShared(0) == p1Type);
        CHECK(fnSym.parameterShared(1) == p2Type);

        CHECK(fnSym.toString().find("compute") != std::string::npos);
        CHECK(fnSym.toString().find("Function") != std::string::npos);
        CHECK(fnSym.toString().find("[def]") != std::string::npos);
    }

    SECTION("Function prototype with void parameter list") {
        auto voidRet = Type::getVoid();
        Symbol fnSym = Symbol::createFunction(
            "reset",
            voidRet,
            {},
            false, // prototype
            ParameterListKind::Void,
            {5, 1}
        );

        CHECK(fnSym.name() == "reset");
        CHECK(fnSym.isFunction());
        CHECK(fnSym.isPrototype());
        CHECK(fnSym.isDeclaration());
        CHECK_FALSE(fnSym.isDefinition());
        CHECK(fnSym.hasVoidParameterList());
        CHECK(fnSym.parameterCount() == 0);
        CHECK(fnSym.toString().find("[proto]") != std::string::npos);
    }

    SECTION("SymbolKind string conversion") {
        CHECK(std::string(symbolKindToString(SymbolKind::Variable)) == "Variable");
        CHECK(std::string(symbolKindToString(SymbolKind::Function)) == "Function");
    }
}

// -----------------------------------------------------------------------------
// 2. Symbol Creation from AST Nodes
// -----------------------------------------------------------------------------
TEST_CASE("Symbol: Creation from VarDecl and Function AST nodes", "[semantic][symbol][ast]") {
    SECTION("From VarDecl AST node") {
        Parser parser = makeParser("const int total = 100;");
        auto varDecl = parser.parseVarDecl();
        REQUIRE(varDecl != nullptr);

        Symbol sym = Symbol::fromVarDecl(*varDecl);
        CHECK(sym.name() == "total");
        CHECK(sym.isVariable());
        REQUIRE(sym.type() != nullptr);
        CHECK(sym.type()->kind() == TypeKind::Int);
        CHECK(sym.isConst());
        CHECK(sym.astNode() == varDecl.get());
        CHECK(sym.getASTNode() == varDecl.get());
    }

    SECTION("From Function AST node (prototype)") {
        Parser parser = makeParser("int add(int a, int b);");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);

        Symbol sym = Symbol::fromFunction(*fn);
        CHECK(sym.name() == "add");
        CHECK(sym.isFunction());
        CHECK(sym.returnTypeKind() == TypeKind::Int);
        CHECK(sym.isPrototype());
        CHECK(sym.astNode() == fn.get());
        REQUIRE(sym.parameterCount() == 2);
        CHECK(sym.parameter(0)->kind() == TypeKind::Int);
        CHECK(sym.parameter(1)->kind() == TypeKind::Int);
        CHECK(sym.parameterName(0) == "a");
        CHECK(sym.parameterName(1) == "b");
    }

    SECTION("From Function AST node (definition)") {
        Parser parser = makeParser("void run(void) { }");
        auto fn = parser.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);

        Symbol sym = Symbol::fromFunction(*fn);
        CHECK(sym.name() == "run");
        CHECK(sym.isFunction());
        CHECK(sym.returnTypeKind() == TypeKind::Void);
        CHECK(sym.isDefinition());
        CHECK(sym.hasVoidParameterList());
        CHECK(sym.parameterCount() == 0);
        CHECK(sym.astNode() == fn.get());
    }
}

// -----------------------------------------------------------------------------
// 3. SymbolTable Basic Operations & Queries
// -----------------------------------------------------------------------------
TEST_CASE("SymbolTable: Basic container operations", "[semantic][symbol_table][basic]") {
    SymbolTable table;

    SECTION("Initially empty") {
        CHECK(table.empty());
        CHECK(table.size() == 0);
        CHECK_FALSE(table.contains("x"));
        CHECK_FALSE(table.has("x"));
        CHECK(table.lookup("x") == nullptr);
        CHECK(table.find("x") == nullptr);
        CHECK(table.get("x") == nullptr);
        CHECK(table.order().empty());
        CHECK(table.getAllSymbols().empty());
    }

    SECTION("Insert single variable symbol") {
        Symbol sym = Symbol::createVariable("count", Type::getInt());
        bool inserted = table.insert(sym);

        CHECK(inserted);
        CHECK_FALSE(table.empty());
        CHECK(table.size() == 1);
        CHECK(table.contains("count"));
        CHECK(table.has("count"));

        const Symbol* found = table.lookup("count");
        REQUIRE(found != nullptr);
        CHECK(found->name() == "count");
        CHECK(found->isVariable());
        CHECK(found->typeKind() == TypeKind::Int);
    }

    SECTION("Duplicate insertion fails and preserves original") {
        Symbol sym1 = Symbol::createVariable("val", Type::getInt());
        Symbol sym2 = Symbol::createVariable("val", Type::getFloat());

        CHECK(table.insert(sym1));
        CHECK_FALSE(table.insert(sym2)); // duplicate rejected

        CHECK(table.size() == 1);
        const Symbol* found = table.lookup("val");
        REQUIRE(found != nullptr);
        CHECK(found->typeKind() == TypeKind::Int); // original preserved
    }

    SECTION("insertOrAssign updates existing symbol") {
        Symbol sym1 = Symbol::createVariable("val", Type::getInt());
        Symbol sym2 = Symbol::createVariable("val", Type::getFloat());

        table.insert(sym1);
        CHECK(table.lookup("val")->typeKind() == TypeKind::Int);

        table.insertOrAssign(sym2);
        CHECK(table.size() == 1);
        CHECK(table.lookup("val")->typeKind() == TypeKind::Float);
    }

    SECTION("Remove existing symbol") {
        table.insert(Symbol::createVariable("a", Type::getInt()));
        table.insert(Symbol::createVariable("b", Type::getInt()));
        CHECK(table.size() == 2);

        CHECK(table.remove("a"));
        CHECK(table.size() == 1);
        CHECK_FALSE(table.contains("a"));
        CHECK(table.contains("b"));

        CHECK_FALSE(table.remove("a")); // already removed
        CHECK_FALSE(table.remove("non_existent"));
    }

    SECTION("Clear symbol table") {
        table.insert(Symbol::createVariable("x", Type::getInt()));
        table.insert(Symbol::createVariable("y", Type::getFloat()));
        CHECK(table.size() == 2);

        table.clear();
        CHECK(table.empty());
        CHECK(table.size() == 0);
        CHECK(table.lookup("x") == nullptr);
        CHECK(table.order().empty());
    }

    SECTION("Insertion order preservation") {
        table.insert(Symbol::createVariable("first", Type::getInt()));
        table.insert(Symbol::createVariable("second", Type::getFloat()));
        table.insert(Symbol::createVariable("third", Type::getChar()));

        const auto& order = table.order();
        REQUIRE(order.size() == 3);
        CHECK(order[0] == "first");
        CHECK(order[1] == "second");
        CHECK(order[2] == "third");

        auto allSymbols = table.getAllSymbols();
        REQUIRE(allSymbols.size() == 3);
        CHECK(allSymbols[0]->name() == "first");
        CHECK(allSymbols[1]->name() == "second");
        CHECK(allSymbols[2]->name() == "third");
    }

    SECTION("Non-const lookup allows mutating symbol metadata") {
        table.insert(Symbol::createVariable("flag", Type::getInt()));
        Symbol* sym = table.lookup("flag");
        REQUIRE(sym != nullptr);
        sym->setStorageClass(StorageClass::Extern);

        const Symbol* constSym = table.lookup("flag");
        CHECK(constSym->storageClass() == StorageClass::Extern);
    }
}

// -----------------------------------------------------------------------------
// 4. Populating SymbolTable with VarDecl AST Nodes
// -----------------------------------------------------------------------------
TEST_CASE("SymbolTable: Populating VarDecl AST nodes", "[semantic][symbol_table][vardecl]") {
    SymbolTable table;

    SECTION("Primitive variable declarations") {
        Parser p1 = makeParser("int x;");
        auto d1 = p1.parseVarDecl();
        REQUIRE(d1 != nullptr);
        CHECK(table.insert(*d1));

        Parser p2 = makeParser("char ch = 'A';");
        auto d2 = p2.parseVarDecl();
        REQUIRE(d2 != nullptr);
        CHECK(table.insert(*d2));

        Parser p3 = makeParser("float f = 3.14;");
        auto d3 = p3.parseVarDecl();
        REQUIRE(d3 != nullptr);
        CHECK(table.insert(*d3));

        Parser p4 = makeParser("double d = 2.718;");
        auto d4 = p4.parseVarDecl();
        REQUIRE(d4 != nullptr);
        CHECK(table.insert(*d4));

        CHECK(table.size() == 4);

        const Symbol* sX = table.lookup("x");
        REQUIRE(sX != nullptr);
        CHECK(sX->name() == "x");
        CHECK(sX->isVariable());
        CHECK(sX->typeKind() == TypeKind::Int);
        CHECK(sX->astNode() == d1.get());

        const Symbol* sCh = table.lookup("ch");
        REQUIRE(sCh != nullptr);
        CHECK(sCh->typeKind() == TypeKind::Char);

        const Symbol* sF = table.lookup("f");
        REQUIRE(sF != nullptr);
        CHECK(sF->typeKind() == TypeKind::Float);

        const Symbol* sD = table.lookup("d");
        REQUIRE(sD != nullptr);
        CHECK(sD->typeKind() == TypeKind::Double);
    }

    SECTION("Variable declarations with storage classes and qualifiers") {
        Parser p1 = makeParser("static const int MAX_LIMIT = 100;");
        auto d1 = p1.parseVarDecl();
        REQUIRE(d1 != nullptr);
        CHECK(table.insert(*d1));

        Parser p2 = makeParser("extern volatile char status_byte;");
        auto d2 = p2.parseVarDecl();
        REQUIRE(d2 != nullptr);
        CHECK(table.insert(*d2));

        const Symbol* sMax = table.lookup("MAX_LIMIT");
        REQUIRE(sMax != nullptr);
        CHECK(sMax->isStatic());
        CHECK(sMax->isConst());
        CHECK(sMax->typeKind() == TypeKind::Int);

        const Symbol* sStatus = table.lookup("status_byte");
        REQUIRE(sStatus != nullptr);
        CHECK(sStatus->isExtern());
        CHECK(sStatus->isVolatile());
        CHECK(sStatus->typeKind() == TypeKind::Char);
    }
}

// -----------------------------------------------------------------------------
// 5. Populating SymbolTable with Function AST Nodes
// -----------------------------------------------------------------------------
TEST_CASE("SymbolTable: Populating Function AST nodes", "[semantic][symbol_table][function]") {
    SymbolTable table;

    SECTION("Function prototypes with various parameter types") {
        Parser p1 = makeParser("int add(int a, int b);");
        auto fn1 = p1.parseFunctionDeclaration();
        REQUIRE(fn1 != nullptr);
        CHECK(table.insert(*fn1));

        Parser p2 = makeParser("float scale(char tag, double factor);");
        auto fn2 = p2.parseFunctionDeclaration();
        REQUIRE(fn2 != nullptr);
        CHECK(table.insert(*fn2));

        Parser p3 = makeParser("void log_event(void);");
        auto fn3 = p3.parseFunctionDeclaration();
        REQUIRE(fn3 != nullptr);
        CHECK(table.insert(*fn3));

        CHECK(table.size() == 3);

        const Symbol* sAdd = table.lookup("add");
        REQUIRE(sAdd != nullptr);
        CHECK(sAdd->isFunction());
        CHECK(sAdd->returnTypeKind() == TypeKind::Int);
        CHECK(sAdd->parameterCount() == 2);
        CHECK(sAdd->parameter(0)->kind() == TypeKind::Int);
        CHECK(sAdd->parameter(1)->kind() == TypeKind::Int);
        CHECK(sAdd->isPrototype());

        const Symbol* sScale = table.lookup("scale");
        REQUIRE(sScale != nullptr);
        CHECK(sScale->returnTypeKind() == TypeKind::Float);
        CHECK(sScale->parameterCount() == 2);
        CHECK(sScale->parameter(0)->kind() == TypeKind::Char);
        CHECK(sScale->parameter(1)->kind() == TypeKind::Double);

        const Symbol* sLog = table.lookup("log_event");
        REQUIRE(sLog != nullptr);
        CHECK(sLog->returnTypeKind() == TypeKind::Void);
        CHECK(sLog->hasVoidParameterList());
        CHECK(sLog->parameterCount() == 0);
    }

    SECTION("Function definition with body") {
        Parser p = makeParser("int square(int x) { return x * x; }");
        auto fn = p.parseFunctionDeclaration();
        REQUIRE(fn != nullptr);
        CHECK(table.insert(*fn));

        const Symbol* sSquare = table.lookup("square");
        REQUIRE(sSquare != nullptr);
        CHECK(sSquare->isFunction());
        CHECK(sSquare->isDefinition());
        CHECK_FALSE(sSquare->isPrototype());
        CHECK(sSquare->parameterCount() == 1);
        CHECK(sSquare->parameter(0)->kind() == TypeKind::Int);
        CHECK(sSquare->parameterName(0) == "x");
        CHECK(sSquare->astNode() == fn.get());
    }

    SECTION("Function with static and extern storage classes") {
        Parser p1 = makeParser("static int helper(void);");
        auto fn1 = p1.parseFunctionDeclaration();
        REQUIRE(fn1 != nullptr);
        CHECK(table.insert(*fn1));

        Parser p2 = makeParser("extern void remote_call(int code);");
        auto fn2 = p2.parseFunctionDeclaration();
        REQUIRE(fn2 != nullptr);
        CHECK(table.insert(*fn2));

        const Symbol* sHelper = table.lookup("helper");
        REQUIRE(sHelper != nullptr);
        CHECK(sHelper->isStatic());

        const Symbol* sRemote = table.lookup("remote_call");
        REQUIRE(sRemote != nullptr);
        CHECK(sRemote->isExtern());
    }
}

// -----------------------------------------------------------------------------
// 6. Populating SymbolTable from Program Translation Unit
// -----------------------------------------------------------------------------
TEST_CASE("SymbolTable: Populating from Program AST (Translation Unit)", "[semantic][symbol_table][program]") {
    SECTION("Program with multiple functions") {
        const std::string src =
            "int add(int a, int b);\n"
            "float calculate(int count, float factor);\n"
            "int main(void) { return 0; }\n";

        Parser parser = makeParser(src);
        auto prog = parser.parseProgram();
        REQUIRE(prog != nullptr);
        REQUIRE(prog->statementCount() == 3);

        SymbolTable table;
        table.populate(*prog);

        CHECK(table.size() == 3);
        CHECK(table.contains("add"));
        CHECK(table.contains("calculate"));
        CHECK(table.contains("main"));

        const Symbol* sAdd = table.lookup("add");
        REQUIRE(sAdd != nullptr);
        CHECK(sAdd->isPrototype());
        CHECK(sAdd->returnTypeKind() == TypeKind::Int);

        const Symbol* sCalc = table.lookup("calculate");
        REQUIRE(sCalc != nullptr);
        CHECK(sCalc->isPrototype());
        CHECK(sCalc->returnTypeKind() == TypeKind::Float);
        CHECK(sCalc->parameterCount() == 2);

        const Symbol* sMain = table.lookup("main");
        REQUIRE(sMain != nullptr);
        CHECK(sMain->isDefinition());
        CHECK(sMain->returnTypeKind() == TypeKind::Int);
        CHECK(sMain->hasVoidParameterList());
    }

    SECTION("Program mixing global variables and functions") {
        auto prog = std::make_unique<Program>();

        // Add global variable: int g_counter = 0;
        Parser pVar = makeParser("int g_counter = 0;");
        auto var = pVar.parseVarDecl();
        REQUIRE(var != nullptr);
        prog->addStatement(std::move(var));

        // Add function prototype: void set_counter(int val);
        Parser pFn1 = makeParser("void set_counter(int val);");
        auto fn1 = pFn1.parseFunctionDeclaration();
        REQUIRE(fn1 != nullptr);
        prog->addStatement(std::move(fn1));

        // Add function definition: int get_counter(void) { return 0; }
        Parser pFn2 = makeParser("int get_counter(void) { return 0; }");
        auto fn2 = pFn2.parseFunctionDeclaration();
        REQUIRE(fn2 != nullptr);
        prog->addStatement(std::move(fn2));

        SymbolTable table;
        table.populate(prog.get());

        CHECK(table.size() == 3);
        REQUIRE(table.contains("g_counter"));
        REQUIRE(table.contains("set_counter"));
        REQUIRE(table.contains("get_counter"));

        CHECK(table.lookup("g_counter")->isVariable());
        CHECK(table.lookup("g_counter")->typeKind() == TypeKind::Int);

        CHECK(table.lookup("set_counter")->isFunction());
        CHECK(table.lookup("set_counter")->isPrototype());
        CHECK(table.lookup("set_counter")->returnTypeKind() == TypeKind::Void);

        CHECK(table.lookup("get_counter")->isFunction());
        CHECK(table.lookup("get_counter")->isDefinition());
        CHECK(table.lookup("get_counter")->returnTypeKind() == TypeKind::Int);

        // Verify order
        const auto& order = table.order();
        CHECK(order[0] == "g_counter");
        CHECK(order[1] == "set_counter");
        CHECK(order[2] == "get_counter");
    }
}

// -----------------------------------------------------------------------------
// 7. Global Declarations Only: Isolation from Local Block Scopes
// -----------------------------------------------------------------------------
TEST_CASE("SymbolTable: Global declarations only isolation (Day 45 scope restriction)", "[semantic][symbol_table][global_only]") {
    SECTION("Local variables inside function bodies are NOT added to global symbol table") {
        const std::string src =
            "int compute(int x) {\n"
            "    int local_a = 10;\n"
            "    const double local_b = 2.5;\n"
            "    {\n"
            "        char nested_c = 'Z';\n"
            "    }\n"
            "    return local_a;\n"
            "}\n";

        Parser parser = makeParser(src);
        auto prog = parser.parseProgram();
        REQUIRE(prog != nullptr);

        SymbolTable table;
        table.populate(*prog);

        // Global symbol table MUST contain only the function 'compute'
        CHECK(table.size() == 1);
        CHECK(table.contains("compute"));

        // MUST NOT contain local variables (Day 46 owns local scope management)
        CHECK_FALSE(table.contains("local_a"));
        CHECK_FALSE(table.contains("local_b"));
        CHECK_FALSE(table.contains("nested_c"));
        CHECK_FALSE(table.contains("x")); // Function parameter is also in function/local scope
    }

    SECTION("Realistic multi-construct function does not leak local variables") {
        const std::string src =
            "int calculate(int count, float factor) {\n"
            "    const int base = 10;\n"
            "    {\n"
            "        int base = 20;\n"
            "        int temporary = base + count;\n"
            "    }\n"
            "    float result = count * factor;\n"
            "    return count + base;\n"
            "}\n"
            "int main(void) {\n"
            "    int count = 5;\n"
            "    int result = calculate(count, 2.5);\n"
            "    return 0;\n"
            "}\n";

        Parser parser = makeParser(src);
        auto prog = parser.parseProgram();
        REQUIRE(prog != nullptr);

        SymbolTable table;
        table.populate(*prog);

        CHECK(table.size() == 2);
        CHECK(table.contains("calculate"));
        CHECK(table.contains("main"));

        CHECK_FALSE(table.contains("base"));
        CHECK_FALSE(table.contains("temporary"));
        CHECK_FALSE(table.contains("result"));
        CHECK_FALSE(table.contains("count"));
        CHECK_FALSE(table.contains("factor"));
    }
}

// -----------------------------------------------------------------------------
// 8. Function Prototype Preceding Definition
// -----------------------------------------------------------------------------
TEST_CASE("SymbolTable: Prototype preceding definition in translation unit", "[semantic][symbol_table][proto_def]") {
    const std::string src =
        "int process(int code);\n"
        "int process(int code) { return code + 1; }\n";

    Parser parser = makeParser(src);
    auto prog = parser.parseProgram();
    REQUIRE(prog != nullptr);
    REQUIRE(prog->statementCount() == 2);

    SymbolTable table;
    table.populate(*prog);

    CHECK(table.size() == 1); // Not duplicated
    const Symbol* sym = table.lookup("process");
    REQUIRE(sym != nullptr);
    CHECK(sym->isFunction());
    CHECK(sym->isDefinition()); // Updated to definition
    CHECK(sym->returnTypeKind() == TypeKind::Int);
    CHECK(sym->parameterCount() == 1);
    CHECK(sym->astNode() == prog->statements()[1].get()); // Points to definition AST node
}

// -----------------------------------------------------------------------------
// 9. ToString Formatting
// -----------------------------------------------------------------------------
TEST_CASE("SymbolTable: Formatted string dump", "[semantic][symbol_table][dump]") {
    SymbolTable table;
    table.insert(Symbol::createVariable("x", Type::getInt()));
    table.insert(Symbol::createFunction("foo", Type::getVoid(), {}, false, ParameterListKind::Void));

    std::string dump = table.toString();
    CHECK(dump.find("SymbolTable(2 symbols)") != std::string::npos);
    CHECK(dump.find("Symbol(x: Variable") != std::string::npos);
    CHECK(dump.find("Symbol(foo: Function") != std::string::npos);
}
