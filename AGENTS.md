# BTreeFy Project Context

BTreeFy is a lightweight Behavior Tree library designed for embedded systems, with specific support for Zephyr RTOS and POSIX environments. It focuses on static memory allocation and efficient tree traversal.

## Core Technologies
- **Language:** C (Core library, C11 standard), Python (Code generation/Parser)
- **RTOS:** Zephyr RTOS
- **Build System:** CMake (Min version 3.10)
- **Testing:** Unity (C unit testing framework)
- **External Tools:** Groot (for Behavior Tree modeling)

## Guiding Principles
- **Embedded First:** Avoid dynamic memory allocation (`malloc`/`free`). All tree structures and data must be statically allocated.
- **Portability:** Core logic is platform-agnostic.
- **Naming Convention:** All public symbols, functions, and types must be prefixed with `btf_`.

## Architecture Overview
- **LCRS Representation:** Trees use the Left-Child Right-Sibling pattern to minimize memory footprint.
- **Code Generation:** Behavior trees modeled in Groot (.xml) are converted to C arrays using `scripts/btf_groot_parser.py`.
- **Blackboard/Data:** Trees support a generic data pointer (`void *data`) for state sharing across nodes.

## Project Structure
- `include/btreefy/`: Public API and data structures.
- `src/`: Core implementation.
- `scripts/`: Python scripts for parsing and code generation.
- `tests/`: Unit tests using the Unity framework.
- `models/`: Behavior tree source files (.xml, .btproj).

## Development Workflows
- **Building & Testing:** Use the provided `./compile-execute.sh` script:
  - `./compile-execute.sh`: Build the library.
  - `./compile-execute.sh --with-tests`: Generate test data, build, and run unit tests.
- **Generating Tree Data:** Use `scripts/btf_groot_parser.py` manually if needed to generate C arrays from XML models.
- **Formatting:** Adhere to `.clang-format` using the `format-files.sh` script.
