#include <catch2/catch.hpp>

#include "ast/ast.h"
#include <memory>
#include <string>

// Concrete test-only Expression for testing abstract category and ownership
class TestExpression : public Expression {
public:
    explicit TestExpression(std::string val, bool* destroyedFlag = nullptr, SourceLocation loc = {})
        : Expression(loc), m_val(std::move(val)), m_destroyedFlag(destroyedFlag) {}

    ~TestExpression() override {
        if (m_destroyedFlag) {
            *m_destroyedFlag = true;
        }
    }

    std::string toString() const override { return "TestExpr(" + m_val + ")"; }
    const std::string& val() const { return m_val; }

private:
    std::string m_val;
    bool* m_destroyedFlag{nullptr};
};

// Concrete test-only Statement for testing abstract category and ownership
class TestStatement : public Statement {
public:
    explicit TestStatement(std::string name, bool* destroyedFlag = nullptr, SourceLocation loc = {})
        : Statement(loc), m_name(std::move(name)), m_destroyedFlag(destroyedFlag) {}

    ~TestStatement() override {
        if (m_destroyedFlag) {
            *m_destroyedFlag = true;
        }
    }

    std::string toString() const override { return "TestStmt(" + m_name + ")"; }
    const std::string& name() const { return m_name; }

private:
    std::string m_name;
    bool* m_destroyedFlag{nullptr};
};

// -----------------------------------------------------------------------------
// 1. Program Construction
// -----------------------------------------------------------------------------
TEST_CASE("Program construction and initial state", "[ast][program]") {
    Program program(SourceLocation{1, 1});
    CHECK(program.location().line == 1);
    CHECK(program.location().column == 1);
    CHECK(program.statementCount() == 0);
    CHECK(program.statements().empty());
    CHECK(program.toString() == "Program (0 statements)");
}

// -----------------------------------------------------------------------------
// 2. Function Construction
// -----------------------------------------------------------------------------
TEST_CASE("Function construction and property accessors", "[ast][function]") {
    Function fn("main", "int", SourceLocation{5, 1});
    CHECK(fn.name() == "main");
    CHECK(fn.returnType() == "int");
    CHECK(fn.location().line == 5);
    CHECK(fn.location().column == 1);
    CHECK(fn.parameters().empty());
    CHECK(fn.body().empty());
    CHECK(fn.toString() == "Function: int main()");

    fn.setName("foo");
    fn.setReturnType("void");
    CHECK(fn.name() == "foo");
    CHECK(fn.returnType() == "void");
}

// -----------------------------------------------------------------------------
// 3. VariableDeclaration Construction
// -----------------------------------------------------------------------------
TEST_CASE("VariableDeclaration construction with and without initializer", "[ast][variable]") {
    // Without initializer
    VariableDeclaration var1("x", "int", nullptr, SourceLocation{10, 5});
    CHECK(var1.name() == "x");
    CHECK(var1.type() == "int");
    CHECK_FALSE(var1.hasInitializer());
    CHECK(var1.initializer() == nullptr);
    CHECK(var1.location().line == 10);
    CHECK(var1.location().column == 5);
    CHECK(var1.toString() == "VariableDeclaration: int x");

    // With initializer
    auto init = std::make_unique<TestExpression>("10");
    VariableDeclaration var2("y", "float", std::move(init), SourceLocation{12, 3});
    CHECK(var2.name() == "y");
    CHECK(var2.type() == "float");
    CHECK(var2.hasInitializer());
    REQUIRE(var2.initializer() != nullptr);
    CHECK(var2.initializer()->toString() == "TestExpr(10)");
    CHECK(var2.toString() == "VariableDeclaration: float y = TestExpr(10)");
}

// -----------------------------------------------------------------------------
// 4. Expression Category
// -----------------------------------------------------------------------------
TEST_CASE("Expression category polymorphism", "[ast][expression]") {
    std::unique_ptr<ASTNode> node = std::make_unique<TestExpression>("42", nullptr, SourceLocation{2, 4});
    CHECK(node->location().line == 2);
    CHECK(node->location().column == 4);
    CHECK(node->toString() == "TestExpr(42)");

    // Cast to Expression category
    Expression* expr = dynamic_cast<Expression*>(node.get());
    REQUIRE(expr != nullptr);
    CHECK(expr->toString() == "TestExpr(42)");
}

// -----------------------------------------------------------------------------
// 5. Statement Category
// -----------------------------------------------------------------------------
TEST_CASE("Statement category polymorphism", "[ast][statement]") {
    std::unique_ptr<ASTNode> node = std::make_unique<TestStatement>("stmt1", nullptr, SourceLocation{3, 8});
    CHECK(node->location().line == 3);
    CHECK(node->location().column == 8);

    // Cast to Statement category
    Statement* stmt = dynamic_cast<Statement*>(node.get());
    REQUIRE(stmt != nullptr);
    CHECK(stmt->toString() == "TestStmt(stmt1)");
}

// -----------------------------------------------------------------------------
// 6. Parent-Child Ownership & Destruction
// -----------------------------------------------------------------------------
TEST_CASE("Safe destruction of child AST nodes via unique_ptr", "[ast][ownership]") {
    bool exprDestroyed = false;
    {
        auto expr = std::make_unique<TestExpression>("val", &exprDestroyed);
        VariableDeclaration decl("z", "double", std::move(expr));
        CHECK_FALSE(exprDestroyed);
    }
    // When decl goes out of scope, its initializer must be destroyed
    CHECK(exprDestroyed);
}

// -----------------------------------------------------------------------------
// 7. Program Owning Multiple Children
// -----------------------------------------------------------------------------
TEST_CASE("Program owning multiple child statements", "[ast][program]") {
    bool stmt1Destroyed = false;
    bool stmt2Destroyed = false;

    {
        Program program;
        program.addStatement(std::make_unique<TestStatement>("s1", &stmt1Destroyed));
        program.addStatement(std::make_unique<TestStatement>("s2", &stmt2Destroyed));

        CHECK(program.statementCount() == 2);
        CHECK(program.statements()[0]->toString() == "TestStmt(s1)");
        CHECK(program.statements()[1]->toString() == "TestStmt(s2)");
        CHECK_FALSE(stmt1Destroyed);
        CHECK_FALSE(stmt2Destroyed);
    }

    CHECK(stmt1Destroyed);
    CHECK(stmt2Destroyed);
}

// -----------------------------------------------------------------------------
// 8. Safe Destruction of Complete Tree
// -----------------------------------------------------------------------------
TEST_CASE("Hierarchical tree destruction", "[ast][ownership]") {
    bool paramDestroyed = false;
    bool bodyStmtDestroyed = false;

    {
        Program prog;
        auto fn = std::make_unique<Function>("main", "int");
        fn->addParameter(std::make_unique<VariableDeclaration>("argc", "int", nullptr, SourceLocation{}));

        // Custom param for flag tracking
        auto param = std::make_unique<VariableDeclaration>("argv", "char**");
        param->setInitializer(std::make_unique<TestExpression>("null", &paramDestroyed));
        fn->addParameter(std::move(param));

        fn->addBodyStatement(std::make_unique<TestStatement>("return_0", &bodyStmtDestroyed));

        prog.addStatement(std::move(fn));
        CHECK(prog.statementCount() == 1);
        CHECK_FALSE(paramDestroyed);
        CHECK_FALSE(bodyStmtDestroyed);
    }

    CHECK(paramDestroyed);
    CHECK(bodyStmtDestroyed);
}

// -----------------------------------------------------------------------------
// 9. Function Owned by Program
// -----------------------------------------------------------------------------
TEST_CASE("Function node owned by Program", "[ast][function]") {
    Program program;
    auto fn = std::make_unique<Function>("add", "int", SourceLocation{1, 1});
    fn->addParameter(std::make_unique<VariableDeclaration>("a", "int"));
    fn->addParameter(std::make_unique<VariableDeclaration>("b", "int"));
    fn->addBodyStatement(std::make_unique<VariableDeclaration>("res", "int"));

    program.addStatement(std::move(fn));
    REQUIRE(program.statementCount() == 1);

    Statement* stmt = program.statements()[0].get();
    Function* fnPtr = dynamic_cast<Function*>(stmt);
    REQUIRE(fnPtr != nullptr);

    CHECK(fnPtr->name() == "add");
    CHECK(fnPtr->returnType() == "int");
    CHECK(fnPtr->parameters().size() == 2);
    CHECK(fnPtr->parameters()[0]->name() == "a");
    CHECK(fnPtr->parameters()[1]->name() == "b");
    CHECK(fnPtr->body().size() == 1);
    CHECK(fnPtr->body()[0]->toString() == "VariableDeclaration: int res");
}

// -----------------------------------------------------------------------------
// 10. VariableDeclaration Ownership & Initializer Replacement
// -----------------------------------------------------------------------------
TEST_CASE("VariableDeclaration initializer ownership transfer", "[ast][variable]") {
    bool init1Destroyed = false;
    bool init2Destroyed = false;

    VariableDeclaration decl("count", "int", std::make_unique<TestExpression>("1", &init1Destroyed));
    CHECK(decl.hasInitializer());

    decl.setInitializer(std::make_unique<TestExpression>("2", &init2Destroyed));
    CHECK(init1Destroyed); // First initializer destroyed upon replacement
    CHECK_FALSE(init2Destroyed);

    auto taken = decl.takeInitializer();
    CHECK_FALSE(decl.hasInitializer());
    CHECK(taken->toString() == "TestExpr(2)");
    CHECK_FALSE(init2Destroyed);
}

// -----------------------------------------------------------------------------
// 11. Source Location Tracking across AST Nodes
// -----------------------------------------------------------------------------
TEST_CASE("SourceLocation stored accurately across AST nodes", "[ast][location]") {
    SourceLocation loc{42, 17};
    VariableDeclaration var("val", "int", nullptr, loc);
    CHECK(var.location().line == 42);
    CHECK(var.location().column == 17);

    SourceLocation fnLoc{100, 1};
    Function fn("calc", "double", fnLoc);
    CHECK(fn.location().line == 100);
    CHECK(fn.location().column == 1);
}

// =============================================================================
// DAY 19 TESTS - AST Printer
// =============================================================================

// -----------------------------------------------------------------------------
// 12. Identifiers and Literals
// -----------------------------------------------------------------------------
TEST_CASE("ASTPrinter: identifier and literal expressions", "[ast][printer]") {
    ASTPrinter printer;

    IdentifierExpression id("myVar");
    CHECK(printer.print(&id) == "IdentifierExpression: myVar\n");

    LiteralExpression intLit(LiteralKind::Integer, "42");
    CHECK(printer.print(&intLit) == "LiteralExpression (Integer): 42\n");

    LiteralExpression floatLit(LiteralKind::Float, "3.14");
    CHECK(printer.print(&floatLit) == "LiteralExpression (Float): 3.14\n");

    LiteralExpression charLit(LiteralKind::Char, "'a'");
    CHECK(printer.print(&charLit) == "LiteralExpression (Char): 'a'\n");

    LiteralExpression strLit(LiteralKind::String, "\"hello\"");
    CHECK(printer.print(&strLit) == "LiteralExpression (String): \"hello\"\n");
}

// -----------------------------------------------------------------------------
// 13. Binary Precedence Trees
// -----------------------------------------------------------------------------
TEST_CASE("ASTPrinter: binary precedence 'a + b * c'", "[ast][printer]") {
    // Models AST for: a + (b * c)
    auto bTimesC = std::make_unique<BinaryExpr>(
        "*",
        std::make_unique<IdentifierExpression>("b"),
        std::make_unique<IdentifierExpression>("c")
    );
    auto aPlus = std::make_unique<BinaryExpr>(
        "+",
        std::make_unique<IdentifierExpression>("a"),
        std::move(bTimesC)
    );

    ASTPrinter printer;
    std::string expected =
        "BinaryExpr: +\n"
        "  Left:\n"
        "    IdentifierExpression: a\n"
        "  Right:\n"
        "    BinaryExpr: *\n"
        "      Left:\n"
        "        IdentifierExpression: b\n"
        "      Right:\n"
        "        IdentifierExpression: c\n";

    CHECK(printer.print(aPlus.get()) == expected);
}

// -----------------------------------------------------------------------------
// 14. Unary Expressions
// -----------------------------------------------------------------------------
TEST_CASE("ASTPrinter: unary expressions '-x' and chained '!-x'", "[ast][printer]") {
    ASTPrinter printer;

    UnaryExpr negX("-", std::make_unique<IdentifierExpression>("x"));
    std::string expectedNegX =
        "UnaryExpr: -\n"
        "  Operand:\n"
        "    IdentifierExpression: x\n";
    CHECK(printer.print(&negX) == expectedNegX);

    UnaryExpr notNegX("!", std::make_unique<UnaryExpr>("-", std::make_unique<IdentifierExpression>("x")));
    std::string expectedNotNegX =
        "UnaryExpr: !\n"
        "  Operand:\n"
        "    UnaryExpr: -\n"
        "      Operand:\n"
        "        IdentifierExpression: x\n";
    CHECK(printer.print(&notNegX) == expectedNotNegX);
}

// -----------------------------------------------------------------------------
// 15. Assignment Expressions
// -----------------------------------------------------------------------------
TEST_CASE("ASTPrinter: assignment expressions 'a = b = c' and compound 'x += 5'", "[ast][printer]") {
    ASTPrinter printer;

    // Right-associative: a = (b = c)
    auto bAssignC = std::make_unique<AssignExpr>(
        "=",
        std::make_unique<IdentifierExpression>("b"),
        std::make_unique<IdentifierExpression>("c")
    );
    auto aAssign = std::make_unique<AssignExpr>(
        "=",
        std::make_unique<IdentifierExpression>("a"),
        std::move(bAssignC)
    );

    std::string expectedChained =
        "AssignExpr: =\n"
        "  Left:\n"
        "    IdentifierExpression: a\n"
        "  Right:\n"
        "    AssignExpr: =\n"
        "      Left:\n"
        "        IdentifierExpression: b\n"
        "      Right:\n"
        "        IdentifierExpression: c\n";
    CHECK(printer.print(aAssign.get()) == expectedChained);

    AssignExpr compound(
        "+=",
        std::make_unique<IdentifierExpression>("x"),
        std::make_unique<LiteralExpression>(LiteralKind::Integer, "5")
    );
    std::string expectedCompound =
        "AssignExpr: +=\n"
        "  Left:\n"
        "    IdentifierExpression: x\n"
        "  Right:\n"
        "    LiteralExpression (Integer): 5\n";
    CHECK(printer.print(&compound) == expectedCompound);
}

// -----------------------------------------------------------------------------
// 16. Variable Declarations
// -----------------------------------------------------------------------------
TEST_CASE("ASTPrinter: variable declarations with and without initializer", "[ast][printer]") {
    ASTPrinter printer;

    VariableDeclaration uninit("count", "int");
    CHECK(printer.print(&uninit) == "VariableDeclaration: int count\n");

    VariableDeclaration init("x", "float", std::make_unique<LiteralExpression>(LiteralKind::Float, "3.14"));
    std::string expectedInit =
        "VariableDeclaration: float x\n"
        "  Initializer:\n"
        "    LiteralExpression (Float): 3.14\n";
    CHECK(printer.print(&init) == expectedInit);
}

// -----------------------------------------------------------------------------
// 17. Function Prototypes and Definitions
// -----------------------------------------------------------------------------
TEST_CASE("ASTPrinter: function prototype and definition", "[ast][printer]") {
    ASTPrinter printer;

    // Prototype: int calculate(int x);
    Function proto("calculate", "int", SourceLocation{}, false);
    proto.addParameter(std::make_unique<VariableDeclaration>("x", "int"));

    std::string expectedProto =
        "Function (prototype): int calculate\n"
        "  Parameters:\n"
        "    VariableDeclaration: int x\n";
    CHECK(printer.print(&proto) == expectedProto);

    // Definition: int add(int a, int b) { ... }
    Function def("add", "int", SourceLocation{}, true);
    def.addParameter(std::make_unique<VariableDeclaration>("a", "int"));
    def.addParameter(std::make_unique<VariableDeclaration>("b", "int"));

    std::string expectedDef =
        "Function (definition): int add\n"
        "  Parameters:\n"
        "    VariableDeclaration: int a\n"
        "    VariableDeclaration: int b\n"
        "  Body:\n"
        "    [body not parsed]\n";
    CHECK(printer.print(&def) == expectedDef);

    // Definition with empty parameters: void greet(void) { }
    Function greet("greet", "void", SourceLocation{}, true);
    std::string expectedGreet =
        "Function (definition): void greet\n"
        "  Parameters: (none)\n"
        "  Body:\n"
        "    [body not parsed]\n";
    CHECK(printer.print(&greet) == expectedGreet);
}

// -----------------------------------------------------------------------------
// 18. Program with Multiple Functions
// -----------------------------------------------------------------------------
TEST_CASE("ASTPrinter: program with multiple declarations", "[ast][printer]") {
    Program prog;

    auto proto = std::make_unique<Function>("calculate", "int", SourceLocation{}, false);
    proto->addParameter(std::make_unique<VariableDeclaration>("x", "int"));
    prog.addStatement(std::move(proto));

    auto add = std::make_unique<Function>("add", "int", SourceLocation{}, true);
    add->addParameter(std::make_unique<VariableDeclaration>("a", "int"));
    add->addParameter(std::make_unique<VariableDeclaration>("b", "int"));
    prog.addStatement(std::move(add));

    auto greet = std::make_unique<Function>("greet", "void", SourceLocation{}, true);
    prog.addStatement(std::move(greet));

    std::string expected =
        "Program\n"
        "  Function (prototype): int calculate\n"
        "    Parameters:\n"
        "      VariableDeclaration: int x\n"
        "  Function (definition): int add\n"
        "    Parameters:\n"
        "      VariableDeclaration: int a\n"
        "      VariableDeclaration: int b\n"
        "    Body:\n"
        "      [body not parsed]\n"
        "  Function (definition): void greet\n"
        "    Parameters: (none)\n"
        "    Body:\n"
        "      [body not parsed]\n";

    ASTPrinter printer;
    CHECK(printer.print(&prog) == expected);
}

// -----------------------------------------------------------------------------
// 19. Defensive Null Pointer Handling and Custom Indentation
// -----------------------------------------------------------------------------
TEST_CASE("ASTPrinter: defensive null handling and custom indentation", "[ast][printer]") {
    ASTPrinter printer(2);

    CHECK(printer.print(nullptr) == "[null]\n");

    BinaryExpr binWithNull("+", std::make_unique<IdentifierExpression>("a"), nullptr);
    std::string expectedBinNull =
        "BinaryExpr: +\n"
        "  Left:\n"
        "    IdentifierExpression: a\n"
        "  Right:\n"
        "    [null]\n";
    CHECK(printer.print(&binWithNull) == expectedBinNull);

    // Custom 4-space indentation
    ASTPrinter printer4(4);
    UnaryExpr un("-", std::make_unique<IdentifierExpression>("x"));
    std::string expected4 =
        "UnaryExpr: -\n"
        "    Operand:\n"
        "        IdentifierExpression: x\n";
    CHECK(printer4.print(&un) == expected4);
}
