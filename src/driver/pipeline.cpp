#include "driver.h"

#include <iostream>
#include <fstream>

// ---------------------------------------------------------------------------
// Stage stubs
//   Each function accepts only what it needs. Returns CompileResult so that
//   errors can be returned upward without exceptions or global state.
//   All stages are no-ops at this point; they will be filled in on their
//   designated implementation days.
// ---------------------------------------------------------------------------

// Day 4: Will tokenize the source file into a Token stream.
static CompileResult runLexer(const DriverConfig& config) {
    (void)config; // suppress unused-parameter warning
    return CompileResult::Ok;
}

// Day 5: Will consume the Token stream and produce an AST.
static CompileResult runParser(const DriverConfig& config) {
    (void)config;
    return CompileResult::Ok;
}

// Day 6: Will walk the AST, resolve types, and produce a Typed AST.
static CompileResult runSemantic(const DriverConfig& config) {
    (void)config;
    return CompileResult::Ok;
}

// Day 7+: Will lower the Typed AST to LLVM IR and generate object code.
static CompileResult runCodegen(const DriverConfig& config) {
    (void)config;
    return CompileResult::Ok;
}

// ---------------------------------------------------------------------------
// runPipeline()
//   Orchestrates the four compiler stages in order.
//   Validates that the input file exists before starting any stage,
//   then runs each stage and short-circuits on the first failure.
// ---------------------------------------------------------------------------
CompileResult runPipeline(const DriverConfig& config) {
    // --- File existence check -----------------------------------------------
    {
        std::ifstream probe(config.inputFile);
        if (!probe.good()) {
            std::cerr << "cforge: error: file not found: "
                      << config.inputFile << "\n";
            return CompileResult::FileNotFound;
        }
    }

    // --- Stage 1: Lexer ------------------------------------------------------
    CompileResult r;

    r = runLexer(config);
    if (r != CompileResult::Ok) return r;

    // --- Stage 2: Parser -----------------------------------------------------
    r = runParser(config);
    if (r != CompileResult::Ok) return r;

    // --- Stage 3: Semantic Analysis ------------------------------------------
    r = runSemantic(config);
    if (r != CompileResult::Ok) return r;

    // --- Stage 4: Code Generation --------------------------------------------
    r = runCodegen(config);
    if (r != CompileResult::Ok) return r;

    return CompileResult::Ok;
}
