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
