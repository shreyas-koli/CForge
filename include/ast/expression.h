#pragma once

#include "ast/ast_node.h"

// =============================================================================
// Expression — Abstract Base for Expression AST Nodes
// =============================================================================
//
// Category node for code constructs that evaluate to a value (e.g. literals,
// binary operations, function calls).
//
class Expression : public ASTNode {
public:
    explicit Expression(SourceLocation loc = {}) : ASTNode(loc) {}
    ~Expression() override = default;
};
