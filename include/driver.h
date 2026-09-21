#pragma once

#include <string>

// ---------------------------------------------------------------------------
// DriverConfig
//   Holds all settings parsed from the command line before compilation begins.
//   Populated by main() and passed through to the pipeline.
// ---------------------------------------------------------------------------
struct DriverConfig {
    std::string inputFile;   // Path to the .c source file (required)
    std::string outputFile;  // Path to the output file (optional; defaults to "a.out")
};

// ---------------------------------------------------------------------------
// CompileResult
//   Unified return type propagated from every pipeline stage back to main().
// ---------------------------------------------------------------------------
enum class CompileResult {
    Ok,           // Stage completed successfully
    FileNotFound, // Input source file could not be opened/located
    LexError,     // Reserved for Day 4+ (Lexer failures)
    ParseError,   // Reserved for Day 5+ (Parser failures)
    SemanticError,// Reserved for Day 6+ (Semantic analysis failures)
    CodegenError, // Reserved for Day 7+ (Code generation failures)
};

// ---------------------------------------------------------------------------
// runPipeline()
//   Entry point for the compilation pipeline.
//   Stages executed (all stubs for now):
//     1. Lexer          -> tokens
//     2. Parser         -> AST
//     3. Semantic       -> Typed AST
//     4. Code generation -> object/assembly
//   Returns CompileResult::Ok on success, or a specific error code on failure.
// ---------------------------------------------------------------------------
CompileResult runPipeline(const DriverConfig& config);
