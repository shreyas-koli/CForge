#pragma once

#include "ast/ast_node.h"

#include <cstddef>
#include <string>
#include <vector>

// =============================================================================
// DiagnosticSeverity — Severity Level of a Diagnostic (Day 52)
// =============================================================================
//
// Identifies whether a diagnostic represents a fatal compilation error,
// a non-fatal warning, or an informational note.
//
enum class DiagnosticSeverity {
    Error,
    Warning,
    Note,
};

const char* diagnosticSeverityToString(DiagnosticSeverity severity) noexcept;

// =============================================================================
// Diagnostic — Shared Semantic Analysis Error & Warning Representation (Day 52)
// =============================================================================
//
// Shared diagnostic representation across all Phase 6 semantic checkers:
//   - RedeclarationChecker (Day 47)
//   - UndeclaredChecker   (Day 48)
//   - TypeChecker         (Day 49)
//   - CallChecker         (Day 50)
//   - ReturnChecker       (Day 51)
//
// Fields:
//   identifier          — identifier name involved (if applicable)
//   message             — human-readable diagnostic explanation
//   location            — primary source location (filename, line, column)
//   previousLocation    — previous declaration location (for redeclarations)
//   hasPreviousLocation — true if previousLocation is valid
//   severity            — Error, Warning, or Note
//
// Standard format produced by toString() / format():
//   "filename:line:column: severity: message"
//   e.g. "main.c:5:5: error: undeclared identifier 'counter'"
//
// Graceful fallback formatting:
//   - Line > 0, no filename:   "line:column: severity: message"
//   - Line == 0, with filename: "filename: severity: message"
//   - Line == 0, no filename:   "severity: message"
//   - With previous location:   appends " (previously declared at ...)"
//
struct Diagnostic {
    std::string identifier;
    std::string message;
    SourceLocation location{};
    SourceLocation previousLocation{};
    bool hasPreviousLocation{false};
    DiagnosticSeverity severity{DiagnosticSeverity::Error};

    Diagnostic() = default;

    // Full constructor (preserves Day 47 constructor signature)
    Diagnostic(std::string id,
               std::string msg,
               SourceLocation loc = {},
               SourceLocation prevLoc = {},
               bool hasPrev = false,
               DiagnosticSeverity sev = DiagnosticSeverity::Error)
        : identifier(std::move(id))
        , message(std::move(msg))
        , location(loc)
        , previousLocation(prevLoc)
        , hasPreviousLocation(hasPrev)
        , severity(sev)
    {}

    // Message + location constructor
    Diagnostic(std::string msg,
               SourceLocation loc,
               DiagnosticSeverity sev = DiagnosticSeverity::Error)
        : identifier("")
        , message(std::move(msg))
        , location(loc)
        , previousLocation()
        , hasPreviousLocation(false)
        , severity(sev)
    {}

    // Message + location + identifier constructor
    Diagnostic(std::string msg,
               SourceLocation loc,
               std::string id,
               DiagnosticSeverity sev = DiagnosticSeverity::Error)
        : identifier(std::move(id))
        , message(std::move(msg))
        , location(loc)
        , previousLocation()
        , hasPreviousLocation(false)
        , severity(sev)
    {}

    // Static helper factories
    static Diagnostic makeError(std::string msg, SourceLocation loc = {}, std::string id = "") {
        return Diagnostic(std::move(id), std::move(msg), loc, {}, false, DiagnosticSeverity::Error);
    }

    static Diagnostic makeWarning(std::string msg, SourceLocation loc = {}, std::string id = "") {
        return Diagnostic(std::move(id), std::move(msg), loc, {}, false, DiagnosticSeverity::Warning);
    }

    static Diagnostic makeNote(std::string msg, SourceLocation loc = {}, std::string id = "") {
        return Diagnostic(std::move(id), std::move(msg), loc, {}, false, DiagnosticSeverity::Note);
    }

    // Accessors
    const std::string& getIdentifier() const noexcept { return identifier; }
    const std::string& getMessage() const noexcept { return message; }
    SourceLocation getLocation() const noexcept { return location; }
    SourceLocation getPreviousLocation() const noexcept { return previousLocation; }
    DiagnosticSeverity getSeverity() const noexcept { return severity; }

    const std::string& filename() const noexcept { return location.filename; }
    int line() const noexcept { return location.line; }
    int column() const noexcept { return location.column; }

    bool isError() const noexcept { return severity == DiagnosticSeverity::Error; }
    bool isWarning() const noexcept { return severity == DiagnosticSeverity::Warning; }
    bool isNote() const noexcept { return severity == DiagnosticSeverity::Note; }

    // Formatting methods
    std::string toString() const;
    std::string format() const { return toString(); }
};
