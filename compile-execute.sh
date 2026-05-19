#!/bin/bash

# Function to print help
print_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "BTreeFy Build and Test Script"
    echo ""
    echo "Options:"
    echo "  --with-tests    Build the library and run unit tests"
    echo "  -h, --help      Display this help message"
    echo ""
}

# Default: Build without tests
TESTS_OPTION="OFF"

# Argument parsing
for arg in "$@"; do
    case $arg in
        --with-tests)
            TESTS_OPTION="ON"
            ;;
        -h|--help)
            print_help
            exit 0
            ;;
        *)
            echo "Unknown argument: $arg"
            print_help
            exit 1
            ;;
    esac
done

# Clean and configure
rm -rf ./build

if [ "$TESTS_OPTION" == "ON" ]; then
    # Check for python3
    if ! command -v python3 &> /dev/null; then
        echo "Error: python3 is not installed."
        exit 1
    fi

    # Check for required python packages
    if ! python3 -c "import typing_extensions" &> /dev/null; then
        echo "Error: Python package 'typing_extensions' is not installed."
        echo "Please install it with: pip install typing-extensions"
        exit 1
    fi

    echo "Generating test tree data..."
    python3 scripts/btf_groot_parser.py -m models/test_trees.xml -tn test_tree_1 --source-output-dir tests --include-output-dir tests
fi

cmake . -B build -DBTREEFY_BUILD_TESTS=$TESTS_OPTION

# Build
cd build
if make; then
    echo "========================================"
    echo "Build Successful!"
    echo "========================================"

    if [ "$TESTS_OPTION" == "ON" ]; then
        echo "Running Tests..."
        ctest --output-on-failure
    fi
else
    echo "========================================"
    echo "Build Failed!"
    echo "========================================"
    exit 1
fi
cd ..
