#include "ast/ast_node.h"

// ASTNode base class implementation.
// Virtual destructor default definition in header.

std::string SourceLocation::toString() const {
    if (hasFilename() && hasLocation()) {
        return filename + ":" + std::to_string(line) + ":" + std::to_string(column);
    }
    if (hasFilename() && !hasLocation()) {
        return filename;
    }
    if (!hasFilename() && hasLocation()) {
        return std::to_string(line) + ":" + std::to_string(column);
    }
    return "";
}