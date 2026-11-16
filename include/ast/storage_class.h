#pragma once

#include <string>

// =============================================================================
// StorageClass — C Storage-Class Specifiers (Day 42)
// =============================================================================
//
// In C17 (ISO/IEC 9899:2018 §6.7.1):
// Storage-class specifiers specify storage duration, linkage, and scope.
// They belong to declaration metadata, NOT the Type representation.
//
enum class StorageClass {
    None,     // Default / unspecified storage class
    Static,   // 'static' — internal linkage / static storage duration
    Extern,   // 'extern' — external linkage
    Register, // 'register' — automatic storage duration, register allocation hint
};

inline const char* storageClassToString(StorageClass sc) {
    switch (sc) {
        case StorageClass::None:     return "none";
        case StorageClass::Static:   return "static";
        case StorageClass::Extern:   return "extern";
        case StorageClass::Register: return "register";
        default:                     return "unknown";
    }
}
