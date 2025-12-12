# Zen Development Kit (ZDK)  
Official Style Guidelines and Coding Standard  
Version 1.0.0 — December 2025  
https://github.com/z-libs

## Purpose of This Document

This is the single source of truth for all code style, naming, architecture, and contribution rules across the entire ZDK codebase. Every header, tool, and documentation file in the z-libs organization MUST conform to these rules without exception.

## 1. General Philosophy

All design choices MUST respect the following priority order.

A lower item MUST NOT override a higher item.

1. Zero-overhead performance.
2. Compile-time type safety and correctness.
3. Predictability and explicit behavior.
4. Minimalism and single-header distribution.
5. Long-term maintainability and readability.

If a choice conflicts with any of these, the higher-priority rule wins.

## 2. Language and Compiler Requirements

- The use of C language features introduced after C11 is prohibited unless the feature is explicitly guarded by a feature macro (`Z<MODULE>_EXPERIMENTAL`) or placed in the `experimental` section of the documentation.
- Compiler intrinsics MAY be used only when guarded with the same mechanism. The `extension` section is also available.
- C++ compatibility MUST be full, automatic, zero-cost.
- Permitted compilers are GCC, Clang, MSVC. Other compilers MAY be permitted but are not considered.
- Forbidden constructs are VLA, longjmp in library code.

## 3. Naming Conventions

### 3.1 Public Identifiers

All public identifiers MUST follow the exact patterns below:

- Types:                    `z<module>_<descriptive_name>` (snake_case).
- Functions:                `z<module>_<verb>_<noun>`      (snake_case).
- Global macros:            `Z_UPPER_SNAKE_CASE`.
- Module macros:            `Z<UPPER_MODULE>_<UPPER_NAME>`.
- C++ namespaces:           `z_<module>`

### 3.2 Internal Identifiers

- Internal identifiers MUST begin with `_z<module>_`.
- Private macros MUST end in `__`.
- Double-underscore sequences are otherwise forbidden, except in automatically generated symbols.

### 3.3 Short Names Option

Libraries MAY optionally expose short aliases (`vec_push`, `map_get`, etc.) when the user defines `Z<MODULE>_SHORT_NAMES`. These aliases are never enabled by default.

## 4. Formatting Rules

- Indentation: exactly 4 spaces (never tabs).
- Line width: maximum 100 columns.
- Brace style: Allman (opening brace on new line).
- Pointer asterisks bind to the variable:
    * Allowed: `int *ptr`.
    * NOT allowed: `int* ptr`or `int * ptr`.
- Space after keywords: `if (condition)`.
- No space between function name and parenthesis: `zvec_push(v, x)`.
- One space around binary operators, no space around unary operators.
- Blank line between top-level declarations.
- No trailing whitespace.
- Files MUST start and end with exactly one newline.

## 5. Header Layout Standard

Every public header MUST follow the exact structure below, in the exact order listed:

1. File header comment block (license, generator notice).
2. Include guard.
3. Bundled common definitions (`Z_COMMON_BUNDLED`), if present.
4. Public includes.
5. `extern "C"` block for C++.
6. Public macro configuration.
7. Public type definitions.
8. Public API declarations.
9. Implementation section (inline or macro-generated).
10. C++ wrappers and specializations.
11. `#endif` for include guard.

No deviations are permitted unless given an extraordinary reason.

## 6. Documentation Requirements

- All documentation MUST appear above the symbol it documents.
- Every public function MUST have a comment block above its declaration.
- Comment style: `//`-style single-line comments preferred, /* */ only for block disabling.
- Documentation MUST describe behavior on failure and allocation strategy.
- Documentation for macro expansions MUST appear on the macro definition, never inside the expansion.

## 7. Memory and Resource Management Rules

- No hidden or implicit allocations.
- All allocation goes through `Z_MALLOC` / `Z_CALLOC` / `Z_REALLOC` / `Z_FREE`.
- Library-specific overrides (for example, `VEC_MALLOC`) take precedence over global ones.
- Destructors MUST be idempotent and safe to call on zero-initialized structs.  
- RAII cleanup attributes are permitted only when guarded by `Z_HAS_CLEANUP`.

## 8. Error Handling Policy

Every public operation MUST have two API variants:

- Fast path: returns an `int` error code; asserts on logic errors; never allocates.
- Safe path: returns a `zresult<T>` or equivalent safe wrapper; never asserts.

Both variants MUST have equivalent semantics and MUST be tested equally.

## 9. Generic System Rules

- All generic containers MUST be instantiated via `DEFINE_*_TYPE` macros.
- Users declare types once before including the container header.
- Dispatch tables are built automatically by Zscanner. Manual dispatch tables or void* casting are prohibited, except for user-provided callbacks (comparators, hash functions, iterators, allocators).
- Users register types once before including container headers.
- No manual function pointer tables or `void*` casts are allowed.
- No library function MAY take ownership of user memory unless explicitly documented.

## 10. Testing and Quality Requirements

- Every public function MUST have at least one test.
- Tests are written in pure C using the built-in `assert()` or `ztest` framework.
- No warnings allowed on GCC/Clang `-Wall -Wextra -Wconversion -Werror`.

## 11. Versioning and Release Policy

Strict Semantic Versioning is enforced:

- Major: breaking changes.
- Minor: new features or new libraries (backward compatible).
- Patch: bug fixes and documentation.

## Compliance

Any contribution that violates these rules will be rejected automatically. These guidelines are not suggestions, they are the law that keeps ZDK fast, safe, predictable, and maintainable.

Adherence to this document is the definition of code quality in the z-libs organization.

— Zuhaitz  
December 2025