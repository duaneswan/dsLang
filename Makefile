# Makefile for dsLang Compiler and dsOS
#
# This Makefile builds the dsLang compiler, standard library, and example kernel.
# It is designed to run on macOS and produce a cross-compiled OS kernel binary.

# Directories
COMPILER_DIR = compiler
STD_DIR = std
EXAMPLES_DIR = examples
BUILD_DIR = build

# Tools and flags
CXX = clang++
CC = clang
AS = i386-elf-as
LD = i386-elf-ld

# Use llvm-config to get LLVM flags if available
LLVM_CONFIG = llvm-config
LLVM_CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags 2>/dev/null || echo "-I/usr/local/opt/llvm/include -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS")
LLVM_LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags 2>/dev/null || echo "-L/usr/local/opt/llvm/lib")
LLVM_LIBS = $(shell $(LLVM_CONFIG) --libs core analysis executionengine mcjit interpreter native bitwriter 2>/dev/null || echo "-lLLVM")

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -g -O0 $(LLVM_CXXFLAGS)
CFLAGS = -std=c11 -Wall -Wextra -g -O0
LDFLAGS = $(LLVM_LDFLAGS) $(LLVM_LIBS)

# dsLang compiler source files
COMPILER_SOURCES = $(wildcard $(COMPILER_DIR)/*.cpp)
COMPILER_OBJECTS = $(patsubst $(COMPILER_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(COMPILER_SOURCES))
COMPILER_TARGET = $(BUILD_DIR)/dscc

# Standard library source files
STD_SOURCES = $(wildcard $(STD_DIR)/*.c)
STD_OBJECTS = $(patsubst $(STD_DIR)/%.c,$(BUILD_DIR)/std/%.o,$(STD_SOURCES))
STD_LIB = $(BUILD_DIR)/libds.a

# Example kernel
KERNEL_SOURCE = $(EXAMPLES_DIR)/kernel.ds
KERNEL_OBJECT = $(BUILD_DIR)/kernel.o
BOOT_SOURCE = $(COMPILER_DIR)/boot.s
BOOT_OBJECT = $(BUILD_DIR)/boot.o
KERNEL_BINARY = $(BUILD_DIR)/dsOS-kernel.bin
KERNEL_SYMBOLS = $(BUILD_DIR)/dsOS-kernel.sym

# Default target
all: directories $(COMPILER_TARGET) $(STD_LIB) $(KERNEL_BINARY)

# Create needed directories
directories:
	mkdir -p $(BUILD_DIR)/std $(EXAMPLES_DIR)

# Compile the dsLang compiler
$(COMPILER_TARGET): $(COMPILER_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compile the compiler source files
$(BUILD_DIR)/%.o: $(COMPILER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Compile the standard library
$(STD_LIB): $(STD_OBJECTS)
	ar rcs $@ $^

# Compile the standard library source files
$(BUILD_DIR)/std/%.o: $(STD_DIR)/%.c
	$(CC) $(CFLAGS) -ffreestanding -c -o $@ $<

# Build the example kernel
$(KERNEL_BINARY): $(BOOT_OBJECT) $(KERNEL_OBJECT) $(STD_LIB)
	$(LD) -T $(COMPILER_DIR)/linker.ld -o $@ $^ --oformat=elf32-i386
	i386-elf-objcopy --only-keep-debug $@ $(KERNEL_SYMBOLS)
	i386-elf-objcopy -O binary $@ $@.bin

# Compile the kernel source (once we have the compiler)
$(KERNEL_OBJECT): $(KERNEL_SOURCE) $(COMPILER_TARGET)
	$(COMPILER_TARGET) -o $@ $<

# Assemble the boot source
$(BOOT_OBJECT): $(BOOT_SOURCE)
	$(AS) -o $@ $<

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Create an example kernel file if it doesn't exist
example:
	@if [ ! -f $(KERNEL_SOURCE) ]; then \
		mkdir -p $(EXAMPLES_DIR); \
		echo '/* kernel.ds - Example dsOS kernel */\n\n/* This is the main entry point for the dsOS kernel */\nint kernel_main() {\n    clear_screen();\n    puts("Welcome to dsOS!");\n    puts("This kernel was written in dsLang.");\n    return 0;\n}' > $(KERNEL_SOURCE); \
		echo "Created example kernel at $(KERNEL_SOURCE)"; \
	else \
		echo "Example kernel already exists at $(KERNEL_SOURCE)"; \
	fi

# Run the kernel in QEMU
run: $(KERNEL_BINARY)
	qemu-system-i386 -kernel $(KERNEL_BINARY).bin

# Phony targets
.PHONY: all clean example run directories

# Dependencies
# If we had header file dependencies, we'd include them here
