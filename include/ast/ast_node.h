#pragma once

#include <cstddef>
#include <memory>
#include <string>

// =============================================================================
// SourceLocation — AST Node Position
// =============================================================================
//
// Simple 1-based source location tracking matching the Lexer's convention.
//
struct SourceLocation {
    int line{0};    // 1-based line number (0 = unknown)
    int column{0};  // 1-based column number (0 = unknown)
};

// =============================================================================
// ASTNode — Base Class for All AST Nodes
// =============================================================================
//
// Root of the AST class hierarchy.
// Provides polymorphic destruction, source location tracking, and base interface.
//
class ASTNode {
public:
    explicit ASTNode(SourceLocation loc = {}) : m_location(loc) {}
    virtual ~ASTNode() = default;

    // Prevent copying to enforce single-ownership tree structure
    ASTNode(const ASTNode&) = delete;
    ASTNode& operator=(const ASTNode&) = delete;

    // Allow moving
    ASTNode(ASTNode&&) noexcept = default;
    ASTNode& operator=(ASTNode&&) noexcept = default;

    SourceLocation location() const { return m_location; }
    void setLocation(SourceLocation loc) { m_location = loc; }

    // Human-readable node description for debugging/testing
    virtual std::string toString() const = 0;

private:
    SourceLocation m_location;
};
