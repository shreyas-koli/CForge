# CForge

CForge is an educational C compiler built from scratch in C++.

## Project Structure

```
CForge/
├── docs/       # Architecture & project documentation
├── include/    # Public CForge headers
├── src/        # Compiler implementation stages
│   ├── lexer/   # Lexical analysis
│   ├── parser/  # Syntax parsing
│   ├── ast/     # Abstract Syntax Tree nodes
│   ├── semantic/# Type checking and semantic analysis
│   ├── types/   # Type system representation
│   ├── ir/      # LLVM IR generation
│   ├── codegen/ # Code generation
│   └── driver/  # CLI driver entry point (main)
├── tests/      # Unit & integration tests
├── examples/   # Sample C source programs
└── build/      # Build output directory
```

## Building CForge

### Prerequisites
- C++17 compatible compiler (`g++`, `clang++`, or MSVC)
- CMake 3.20 or higher
- Ninja or Make

### Configure and Build
```bash
cmake -B build -G Ninja
cmake --build build
```

### Running Tests
```bash
ctest --test-dir build --output-on-failure
```
