#include "driver.h"

#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// printUsage()
//   Prints usage instructions to stderr and returns EXIT_FAILURE so the
//   caller can propagate it directly without duplicating the exit code.
// ---------------------------------------------------------------------------
static int printUsage(const char* programName) {
    std::cerr << "Usage:\n"
              << "  " << programName << " <input.c>\n"
              << "  " << programName << " <input.c> -o <output>\n";
    return 1;
}

// ---------------------------------------------------------------------------
// main()
//   Responsibilities on Day 3:
//     1. Parse command-line arguments into a DriverConfig.
//     2. Validate required arguments are present.
//     3. Invoke the compilation pipeline.
//     4. Map the pipeline result to a POSIX exit code (0 = success, 1 = error).
//
//   argc/argv:
//     argc  - count of arguments including the program name at argv[0].
//     argv  - array of C-string pointers:
//               argv[0] = program path  (e.g. "cforge" or "./build/cforge")
//               argv[1] = first argument (expected: input filename)
//               argv[2] = optional flag  (expected: "-o")
//               argv[3] = optional value (expected: output filename)
// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    // --- Argument count guard -----------------------------------------------
    // argc == 1 means only the program name was supplied; no source file.
    if (argc < 2) {
        return printUsage(argv[0]);
    }

    // --- Populate DriverConfig ----------------------------------------------
    DriverConfig config;
    config.inputFile  = argv[1];
    config.outputFile = "a.out"; // sensible default

    // Walk the remaining arguments looking for known flags.
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-o") {
            // -o must be followed by an output filename.
            if (i + 1 >= argc) {
                std::cerr << "cforge: error: -o requires an argument\n";
                return 1;
            }
            config.outputFile = argv[++i]; // consume the next token
        } else {
            std::cerr << "cforge: error: unknown argument: " << arg << "\n";
            return printUsage(argv[0]);
        }
    }

    // --- Run the compilation pipeline ---------------------------------------
    CompileResult result = runPipeline(config);

    // --- Map CompileResult -> exit code -------------------------------------
    switch (result) {
        case CompileResult::Ok:
            return 0;
        case CompileResult::FileNotFound:
            // Error message already printed by runPipeline().
            return 1;
        case CompileResult::LexError:
            std::cerr << "cforge: fatal: lexer error\n";
            return 1;
        case CompileResult::ParseError:
            std::cerr << "cforge: fatal: parser error\n";
            return 1;
        case CompileResult::SemanticError:
            std::cerr << "cforge: fatal: semantic error\n";
            return 1;
        case CompileResult::CodegenError:
            std::cerr << "cforge: fatal: code generation error\n";
            return 1;
    }

    return 1; // unreachable but silences compiler warnings
}
