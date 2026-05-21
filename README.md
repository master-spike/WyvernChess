# WyvernChess

WyvernChess is a C++20 chess engine built with CMake.

## Requirements

- CMake 3.10 or newer
- A C++20-capable compiler

## Build

Configure and build the project from the repository root:

```sh
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
```

The executable is generated at:

```sh
build/wyvernchess
```

`compile_commands.json` is generated in the `build/` directory for editor and
tooling integration.

## Clean rebuild

If the build directory was generated from a different source path or you need a
clean configuration:

```sh
rm -rf build
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
```
