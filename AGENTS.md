# BTreeFy Project Context

BTreeFy is a lightweight Behavior Tree library designed for embedded systems, with specific support for Zephyr RTOS and POSIX environments. It focuses on static memory allocation and efficient tree traversal.

## Core Technologies
- **Language:** C (Core library, C11 standard), Python (Python 3.x for code generation/parsing)
- **Python Dependencies:** `typing-extensions` (required by `btf_groot_parser.py`)
- **Formatting Tools:** `clang-format` (required by `format-files.sh`)
- **RTOS & Embedded Tools:** Zephyr RTOS, Zephyr SDK, and `west` (for Zephyr-specific runners and examples)
- **Build System:** CMake (Min version 3.10)
- **Testing:** Unity (C unit testing framework, fetched automatically from GitHub during CMake test configuration)
- **External Tools:** Groot (for Behavior Tree modeling)

## Guiding Principles
- **Embedded First:** Avoid dynamic memory allocation (`malloc`/`free`). All tree structures and data must be statically allocated.
- **Portability:** Core logic is platform-agnostic; hardware/RTOS interactions are isolated in "runners".
- **Naming Convention:** All public symbols, functions, and types must be prefixed with `btf_`.

## Architecture Overview
- **LCRS Representation:** Trees use the Left-Child Right-Sibling pattern to minimize memory footprint.
- **Code Generation:** Behavior trees modeled in Groot (.xml) are converted to C arrays using `scripts/btf_groot_parser.py`.
- **Blackboard/Data:** Trees support a generic data pointer (`void *data`) for state sharing across nodes.

## Project Structure
- `include/btreefy/`: Public API and data structures.
- `src/`: Core implementation.
- `src/runners/`: Platform-specific tree runners (POSIX, Zephyr).
- `scripts/`: Python scripts for parsing and code generation.
- `tests/`: Unit tests using the Unity framework.
- `models/`: Behavior tree source files (.xml, .btproj).
- `examples/`: Example applications demonstrating library usage.
  - `examples/pc/`: POSIX-based example demonstrating door operator controller behavior.
  - `examples/zephyr-app/`: Zephyr RTOS-based workspace showing integration with Zephyr build system.

## Development Workflows
- **Building & Testing:** Use the provided `./compile-execute.sh` script:
  - `./compile-execute.sh`: Build the library.
  - `./compile-execute.sh --with-tests`: Generate test data, build, and run unit tests. Requires an internet connection during configuration to fetch the Unity framework.
- **Generating Tree Data:** Use `scripts/btf_groot_parser.py` manually if needed to generate C arrays from XML models.
- **Formatting:** Adhere to `.clang-format` using the `format-files.sh` script.

## Context & Artifact Management
- **Context Maintenance:** When any aspect of the project defined in this context file changes, this file (`AGENTS.md`) must be updated accordingly to keep agents and developers aligned.
- **Artifact Organization:** All artifacts generated during development sessions must be created inside the `.gemini/` folder. Within `.gemini/`, files must be organized by category:
  - Plans must be saved in `.gemini/plans/`
  - Tasks must be saved in `.gemini/tasks/`
  - Other session-related artifacts should follow a similar structure under their respective subdirectories (e.g., `.gemini/scratch/`, `.gemini/logs/`).

