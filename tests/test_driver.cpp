// Do NOT define CATCH_CONFIG_MAIN here.
// The runner entry point is defined in tests/test_main.cpp.
#include <catch2/catch.hpp>

#include "driver.h"

#include <fstream>
#include <cstdio>

// ---------------------------------------------------------------------------
// Helper: create a temporary file and return its path.
//   The file is written with trivial content so that the pipeline's
//   file-existence check passes. Actual lexing is a stub, so no valid
//   C source is required at this stage.
// ---------------------------------------------------------------------------
static std::string makeTempFile(const std::string& content = "// stub") {
    std::string path = "cforge_test_tmp.c";
    std::ofstream f(path);
    f << content;
    return path;
}

static void removeTempFile(const std::string& path) {
    std::remove(path.c_str());
}

// ---------------------------------------------------------------------------
// DriverConfig construction
// ---------------------------------------------------------------------------
TEST_CASE("DriverConfig stores input filename", "[driver]") {
    DriverConfig cfg;
    cfg.inputFile  = "hello.c";
    cfg.outputFile = "a.out";

    REQUIRE(cfg.inputFile  == "hello.c");
    REQUIRE(cfg.outputFile == "a.out");
}

TEST_CASE("DriverConfig stores custom output filename", "[driver]") {
    DriverConfig cfg;
    cfg.inputFile  = "hello.c";
    cfg.outputFile = "hello_out";

    REQUIRE(cfg.outputFile == "hello_out");
}

// ---------------------------------------------------------------------------
// Pipeline: file-not-found
// ---------------------------------------------------------------------------
TEST_CASE("runPipeline returns FileNotFound for missing input", "[pipeline]") {
    DriverConfig cfg;
    cfg.inputFile  = "definitely_does_not_exist_XYZ.c";
    cfg.outputFile = "a.out";

    CompileResult result = runPipeline(cfg);
    REQUIRE(result == CompileResult::FileNotFound);
}

// ---------------------------------------------------------------------------
// Pipeline: success path (all stages are stubs -> Ok)
// ---------------------------------------------------------------------------
TEST_CASE("runPipeline returns Ok for existing input file", "[pipeline]") {
    std::string tmp = makeTempFile();

    DriverConfig cfg;
    cfg.inputFile  = tmp;
    cfg.outputFile = "a.out";

    CompileResult result = runPipeline(cfg);
    REQUIRE(result == CompileResult::Ok);

    removeTempFile(tmp);
}

// ---------------------------------------------------------------------------
// CompileResult enum sanity
// ---------------------------------------------------------------------------
TEST_CASE("CompileResult Ok is distinct from all error codes", "[pipeline]") {
    REQUIRE(CompileResult::Ok           != CompileResult::FileNotFound);
    REQUIRE(CompileResult::Ok           != CompileResult::LexError);
    REQUIRE(CompileResult::Ok           != CompileResult::ParseError);
    REQUIRE(CompileResult::Ok           != CompileResult::SemanticError);
    REQUIRE(CompileResult::Ok           != CompileResult::CodegenError);
}
