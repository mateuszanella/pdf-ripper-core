# Using pdf-core

## Dependencies

pdf-core depends on:
- [io-core](https://github.com/ripper-org/io-core) — I/O abstractions
- [zlib-ng](https://github.com/zlib-ng/zlib-ng) — compression support

Both are fetched automatically when building with CMake.

## Building and installing

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cmake --install build --prefix /your/install/prefix
```

## Integrating with CMake

### Installed package

```cmake
find_package(pdf_ripper_core REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE pdf_ripper_core::pdf_ripper_core)
```

### FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    pdf_ripper_core
    GIT_REPOSITORY https://github.com/ripper-org/pdf-core
    GIT_TAG        main
)

FetchContent_MakeAvailable(pdf_ripper_core)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE pdf_ripper_core::pdf_ripper_core)
```

io-core and zlib-ng are pulled in automatically as transitive dependencies.

## CMake options

| Option                                   | Default | Description                             |
| ---------------------------------------- | ------- | --------------------------------------- |
| `PDF_RIPPER_CORE_ENABLE_TESTS`           | auto    | Enable test suite                       |
| `PDF_RIPPER_CORE_IO_CORE_GIT_REPOSITORY` | GitHub  | Override io-core git repository         |
| `PDF_RIPPER_CORE_IO_CORE_GIT_TAG`        | `main`  | Override io-core git tag                |

## API overview

pdf-core exposes a clean, object-oriented API for working with PDF documents:

- **Document** — top-level representation of a PDF file
- **Parser** — reads and parses PDF syntax into in-memory objects
- **Serializer** — writes in-memory objects back to valid PDF output
- **Object system** — indirect objects, streams, references, and value types
- **Cross-reference table** — management of object locations
- **Trailer** — document metadata and cross-reference root
- **Catalog / Pages tree** — document structure navigation
