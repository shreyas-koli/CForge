#pragma once

#include "ast/ast_node.h"

// =============================================================================
// Statement — Abstract Base for Statement AST Nodes
// =============================================================================
//
// Category node for code constructs that execute action (e.g. declarations,
// control flow, function definitions).
//
class Statement : public ASTNode {
public:
    explicit Statement(SourceLocation loc = {}) : ASTNode(loc) {}
    ~Statement() override = default;
};
