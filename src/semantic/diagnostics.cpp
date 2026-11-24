#include "semantic/diagnostics.h"

const char* diagnosticSeverityToString(DiagnosticSeverity severity) noexcept {
    switch (severity) {
        case DiagnosticSeverity::Error:
            return "error";
        case DiagnosticSeverity::Warning:
            return "warning";
        case DiagnosticSeverity::Note:
            return "note";
    }
    return "error";
}

std::string Diagnostic::toString() const {
    std::string s;
    if (location.hasFilename()) {
        s += location.filename;
        if (location.hasLocation()) {
            s += ":" + std::to_string(location.line) + ":" + std::to_string(location.column);
        }
        s += ": ";
    } else if (location.hasLocation()) {
        s += std::to_string(location.line) + ":" + std::to_string(location.column) + ": ";
    }

    s += diagnosticSeverityToString(severity);
    s += ": ";
    s += message;

    if (hasPreviousLocation) {
        if (previousLocation.hasFilename() && previousLocation.hasLocation()) {
            s += " (previously declared at " + previousLocation.filename + ":" +
                 std::to_string(previousLocation.line) + ":" +
                 std::to_string(previousLocation.column) + ")";
        } else if (previousLocation.hasFilename()) {
            s += " (previously declared at " + previousLocation.filename + ")";
        } else if (previousLocation.hasLocation()) {
            s += " (previously declared at " + std::to_string(previousLocation.line) +
                 ":" + std::to_string(previousLocation.column) + ")";
        }
    }

    return s;
}
