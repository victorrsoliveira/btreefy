if ! command -v clang-format 2>&1 >/dev/null
then
    echo "clang-format could not be found"
    exit 1
fi

find . -iname '*.h' -o -iname '*.c' | xargs clang-format -i 