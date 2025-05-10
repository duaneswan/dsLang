<p align="center">
  <img src="https://cdn.dtechmediacreations.com/controller.php?system=dsLang.png" alt="dsLang Logo" />
</p>

# dsLang

dsLang is a systems programming language designed specifically for building dsOS (a custom x86-64 operating system) and applications that run on it. With C/Objective-C-inspired syntax, dsLang provides a minimal yet powerful toolset for low-level OS development.

## Features

- C-like syntax with minimal Objective-C style message passing
- Static typing with fundamental data types suitable for systems programming
- Purpose-built for OS development on x86-64 architecture
- Cross-compilation from macOS to dsOS (x86-64, BIOS-based non-UEFI)
- Complete compiler with LLVM backend
- Comprehensive standard library for OS-level programming
- No external dependencies for compiled programs

## Project Structure

- `/compiler` - Source code for the dsLang compiler
- `/std` - Standard library implementation
- `/docs` - Language specification and documentation
- `/examples` - Example programs written in dsLang
- `/tests` - Test suite for the compiler and language features
- `/build` - Build artifacts (created during compilation)

## Quick Start

1. Build the compiler and standard library:
   ```
   make
   ```

2. Compile an example program:
   ```
   make examples
   ```

3. Run the example OS kernel in QEMU:
   ```
   make run
   ```

See the [documentation](/docs/) for detailed information on dsLang syntax, standard library, and usage.

## Requirements

- macOS development environment
- LLVM (installed via Homebrew: `brew install llvm`)
- x86_64-elf cross-compilation tools (installed via Homebrew: `brew install x86_64-elf-gcc`)
- QEMU (for testing the OS, installed via Homebrew: `brew install qemu`)

## License

This project is distributed under the MIT License. See LICENSE for more information.

