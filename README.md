# PDF Ripper - Core

## What is this?

**PDF Ripper - Core** is a C++ library focused on robust PDF parsing and handling.

The long-term goal of this project is to build a solid, low-level, open-source foundation 
for PDF tooling that is reliable, extensible, and actually pleasant to use, providing a
clean and extensible API that represents pieces of PDF documents as easy-to-use objects.

---

## Current status

Right now in a nutshell, this project is the world’s most convoluted PDF page counter.

> It does that pretty well, though. It managed to count up to 3!

It's not really a library aswell, since there is a literal `main()` function in the codebase 
that accounts for the test base of the library, but we will get there.

We do have some nice features though, such as:

- Reading PDF files and subranges
- Parsing document header, trailer, catalog, and pages metadata
- Full fleshed AI slopped lexer/parser for PDF syntax (works well though)
- Resolving some types of indirect references
- Basic compression/decompression support

So yes, it counts pages… but with *architecture*.

---

## Why this exists

PDF are everywhere, and proper tooling for specific, low level operations is really scarce
and hardly ever free (thank you Adobe). This project aims to fill that gap.

There are already many PDF libraries out there, many really good ones, but I am somewhat
familiar with PDFs, and wanted to learn C++, so that was a match.

---

## TODO

- Add unit tests
- Add workflow tests
- Add actual writing of files (this will be complicated probably) through a `writer` interface
- The `parser` class is a monster that should be split into separate classes and work as an
  orchestrator of a class that manages parser implementations and a class that builds objects.
  (something like that, im not sure yet)
- Refactor the error obejcts, they should not be a glorified integer, but a proper class with
  detailed information about the error, while still being lightweight and easy to use. I don't
  really want to use exceptions
- Add proper CMake support to actually build this as a library and not just a testbed with a 
  `main()` function
- Add support for more features as needed, since there are way too many of them
- Add proper documentation and examples
- Add integrated AI assistant
- Add crypto miner
