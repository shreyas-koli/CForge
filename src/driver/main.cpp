#include "driver.h"
#include "ast/printer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>

// ---------------------------------------------------------------------------
// printUsage()
//   Prints usage instructions to stderr and returns EXIT_FAILURE so the
//   caller can propagate it directly without duplicating the exit code.
// ---------------------------------------------------------------------------
static int printUsage(const char* programName) {
    std::cerr << "Usage:\n"
              << "  " << programName << " <input.c>\n"
              << "  " << programName << " <input.c> -o <output>\n"
              << "  " << programName << " --dump-tokens <input.c>\n"
              << "  " << programName << " --dump-ast <input.c>\n";
    return 1;
}

static int dumpTokens(const std::string& inputFile) {
    std::ifstream input(inputFile);
    if (!input.good()) {
        std::cerr << "cforge: error: file not found: " << inputFile << "\n";
        return 1;
    }

    std::string source((std::istreambuf_iterator<char>(input)),
                       std::istreambuf_iterator<char>());
    Lexer lexer(std::move(source));

    for (const Token& token : lexer.tokenize()) {
        std::cout << std::left << std::setw(12) << tokenTypeName(token.type)
                  << " " << std::quoted(token.lexeme)
                  << " " << token.line << ":" << token.column << "\n";
    }

    return 0;
}

static int dumpAst(const std::string& inputFile) {
    std::ifstream input(inputFile);
    if (!input.good()) {
        std::cerr << "cforge: error: file not found: " << inputFile << "\n";
        return 1;
    }

    std::string source((std::istreambuf_iterator<char>(input)),
                       std::istreambuf_iterator<char>());

    Lexer lexer(std::move(source));
    std::vector<Token> tokens = lexer.tokenize();

    // Check for lexical errors
    for (const Token& tok : tokens) {
        if (isLexerError(tok)) {
            std::cerr << "cforge: error: " << tok.lexeme << " at " << tok.line << ":" << tok.column << "\n";
            return 1;
        }
    }

    // Attempt to parse as translation unit (Program)
    Parser programParser(tokens);
    auto program = programParser.parseProgram();
    if (program && programParser.isAtEnd() && !programParser.hasError()) {
        ASTPrinter printer;
        std::cout << printer.print(program.get());
        return 0;
    }

    // If parseProgram did not succeed or consume all tokens, attempt parseExpression
    Parser exprParser(tokens);
    auto expr = exprParser.parseExpression();
    if (expr && exprParser.isAtEnd() && !exprParser.hasError()) {
        ASTPrinter printer;
        std::cout << printer.print(expr.get());
        return 0;
    }

    // Output relevant error
    bool isLikelyDeclaration = !tokens.empty() && tokens[0].type == TokenType::Keyword;
    if (isLikelyDeclaration && programParser.hasError()) {
        std::cerr << "cforge: error: " << programParser.errorMessage() << "\n";
    } else if (exprParser.hasError()) {
        std::cerr << "cforge: error: " << exprParser.errorMessage() << "\n";
    } else if (programParser.hasError()) {
        std::cerr << "cforge: error: " << programParser.errorMessage() << "\n";
    } else if (!programParser.isAtEnd()) {
        std::cerr << "cforge: error: unexpected trailing tokens: '"
                  << programParser.peek().lexeme << "'\n";
    } else {
        std::cerr << "cforge: error: failed to parse input\n";
    }

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

    if (std::string(argv[1]) == "--dump-tokens") {
        if (argc != 3) {
            return printUsage(argv[0]);
        }
        return dumpTokens(argv[2]);
    }

    if (std::string(argv[1]) == "--dump-ast") {
        if (argc != 3) {
            return printUsage(argv[0]);
        }
        return dumpAst(argv[2]);
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
