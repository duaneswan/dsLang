# dsLang Standard Library Reference

## Introduction

The dsLang Standard Library provides essential functionality for OS development and applications running on dsOS. It implements core services that applications can use, such as memory management, string manipulation, and console I/O.

## Memory Management

### Memory Allocation

```c
void* malloc(unsigned long size);
```
Allocates a block of memory of the specified size.

- **Parameters**:
  - `size`: Number of bytes to allocate
- **Returns**: Pointer to the allocated memory, or NULL if the allocation fails
- **Notes**: The allocated memory is not initialized and may contain garbage values

```c
void free(void* ptr);
```
Deallocates a block of memory that was previously allocated by `malloc`.

- **Parameters**:
  - `ptr`: Pointer to the memory block to deallocate
- **Notes**: If `ptr` is NULL, no operation is performed

### Memory Manipulation

```c
void* memset(void* dest, int value, unsigned long count);
```
Fills a block of memory with the specified value.

- **Parameters**:
  - `dest`: Pointer to the memory block
  - `value`: Value to set (converted to unsigned char)
  - `count`: Number of bytes to set
- **Returns**: The `dest` pointer

```c
void* memcpy(void* dest, const void* src, unsigned long count);
```
Copies a block of memory from one location to another.

- **Parameters**:
  - `dest`: Pointer to the destination memory block
  - `src`: Pointer to the source memory block
  - `count`: Number of bytes to copy
- **Returns**: The `dest` pointer
- **Notes**: The memory blocks must not overlap

```c
void* memmove(void* dest, const void* src, unsigned long count);
```
Copies a block of memory from one location to another, properly handling overlapping regions.

- **Parameters**:
  - `dest`: Pointer to the destination memory block
  - `src`: Pointer to the source memory block
  - `count`: Number of bytes to copy
- **Returns**: The `dest` pointer
- **Notes**: Safe to use when the memory blocks might overlap

```c
int memcmp(const void* ptr1, const void* ptr2, unsigned long count);
```
Compares two blocks of memory.

- **Parameters**:
  - `ptr1`: Pointer to the first memory block
  - `ptr2`: Pointer to the second memory block
  - `count`: Number of bytes to compare
- **Returns**: 
  - 0 if the memory blocks are equal
  - <0 if the first differing byte in `ptr1` is less than the corresponding byte in `ptr2`
  - >0 if the first differing byte in `ptr1` is greater than the corresponding byte in `ptr2`

## String Handling

### String Length

```c
unsigned long strlen(const char* str);
```
Calculates the length of a null-terminated string.

- **Parameters**:
  - `str`: Null-terminated string
- **Returns**: Number of characters in the string, excluding the null terminator

### String Copying

```c
char* strcpy(char* dest, const char* src);
```
Copies a string from one location to another.

- **Parameters**:
  - `dest`: Destination buffer
  - `src`: Source string
- **Returns**: The `dest` pointer
- **Notes**: The destination buffer must be large enough to hold the copied string and its null terminator

```c
char* strncpy(char* dest, const char* src, unsigned long n);
```
Copies at most `n` characters from one string to another.

- **Parameters**:
  - `dest`: Destination buffer
  - `src`: Source string
  - `n`: Maximum number of characters to copy
- **Returns**: The `dest` pointer
- **Notes**: If `src` is shorter than `n`, the remaining characters in `dest` are filled with null characters

### String Concatenation

```c
char* strcat(char* dest, const char* src);
```
Appends one string to another.

- **Parameters**:
  - `dest`: Null-terminated destination string
  - `src`: Null-terminated source string to append
- **Returns**: The `dest` pointer
- **Notes**: The destination buffer must be large enough to hold both strings and the null terminator

```c
char* strncat(char* dest, const char* src, unsigned long n);
```
Appends at most `n` characters from one string to another.

- **Parameters**:
  - `dest`: Null-terminated destination string
  - `src`: Null-terminated source string to append
  - `n`: Maximum number of characters to append
- **Returns**: The `dest` pointer
- **Notes**: A null terminator is always appended to the result

### String Comparison

```c
int strcmp(const char* str1, const char* str2);
```
Compares two strings lexicographically.

- **Parameters**:
  - `str1`: First string
  - `str2`: Second string
- **Returns**:
  - 0 if the strings are equal
  - <0 if `str1` is lexicographically less than `str2`
  - >0 if `str1` is lexicographically greater than `str2`

```c
int strncmp(const char* str1, const char* str2, unsigned long n);
```
Compares at most `n` characters of two strings lexicographically.

- **Parameters**:
  - `str1`: First string
  - `str2`: Second string
  - `n`: Maximum number of characters to compare
- **Returns**:
  - 0 if the strings are equal (up to `n` characters)
  - <0 if `str1` is lexicographically less than `str2`
  - >0 if `str1` is lexicographically greater than `str2`

### String Searching

```c
char* strchr(const char* str, int ch);
```
Finds the first occurrence of a character in a string.

- **Parameters**:
  - `str`: String to search
  - `ch`: Character to find (converted to `char`)
- **Returns**: Pointer to the first occurrence of the character in the string, or NULL if not found

```c
char* strrchr(const char* str, int ch);
```
Finds the last occurrence of a character in a string.

- **Parameters**:
  - `str`: String to search
  - `ch`: Character to find (converted to `char`)
- **Returns**: Pointer to the last occurrence of the character in the string, or NULL if not found

```c
char* strstr(const char* haystack, const char* needle);
```
Finds the first occurrence of a substring in a string.

- **Parameters**:
  - `haystack`: String to search in
  - `needle`: Substring to find
- **Returns**: Pointer to the first occurrence of the substring in the string, or NULL if not found

### String Conversion

```c
int atoi(const char* str);
```
Converts a string to an integer.

- **Parameters**:
  - `str`: String to convert
- **Returns**: Integer value

```c
char* itoa(int value, char* str);
```
Converts an integer to a string.

- **Parameters**:
  - `value`: Integer to convert
  - `str`: Buffer to store the string
- **Returns**: The `str` pointer

## Console I/O

### Output Functions

```c
void putchar(char c);
```
Writes a character to the console.

- **Parameters**:
  - `c`: Character to write

```c
void puts(const char* str);
```
Writes a string to the console.

- **Parameters**:
  - `str`: Null-terminated string to write

```c
int printf(const char* format, ...);
```
Prints formatted output to the console.

- **Parameters**:
  - `format`: Format string
  - `...`: Variable arguments
- **Returns**: Number of characters printed
- **Supported Format Specifiers**:
  - `%d`, `%i`: Signed decimal integer
  - `%u`: Unsigned decimal integer
  - `%x`: Hexadecimal integer (lowercase)
  - `%X`: Hexadecimal integer (uppercase)
  - `%c`: Character
  - `%s`: String
  - `%%`: Literal `%`

```c
void clear_screen();
```
Clears the console screen and resets the cursor position.

### Input Functions

```c
int getchar();
```
Reads a character from the keyboard.

- **Returns**: Character read, or 0 for unhandled keys

## Hardware Access

### Port I/O Functions

```c
void outb(unsigned short port, unsigned char value);
```
Outputs a byte to an I/O port.

- **Parameters**:
  - `port`: Port address
  - `value`: Byte value to output

```c
unsigned char inb(unsigned short port);
```
Inputs a byte from an I/O port.

- **Parameters**:
  - `port`: Port address
- **Returns**: Byte value read from the port

```c
void outw(unsigned short port, unsigned short value);
```
Outputs a word (16 bits) to an I/O port.

- **Parameters**:
  - `port`: Port address
  - `value`: Word value to output

```c
unsigned short inw(unsigned short port);
```
Inputs a word (16 bits) from an I/O port.

- **Parameters**:
  - `port`: Port address
- **Returns**: Word value read from the port

```c
void outl(unsigned short port, unsigned int value);
```
Outputs a long (32 bits) to an I/O port.

- **Parameters**:
  - `port`: Port address
  - `value`: Long value to output

```c
unsigned int inl(unsigned short port);
```
Inputs a long (32 bits) from an I/O port.

- **Parameters**:
  - `port`: Port address
- **Returns**: Long value read from the port

## Implementation Details

### Memory Allocator

The `malloc` function uses a simple first-fit allocator with a fixed-size heap. The heap size is defined as 1MB by default but can be adjusted based on system requirements.

The allocator maintains a linked list of memory block headers, each containing:
- Size of the block
- Whether the block is used or free

When `malloc` is called, it searches for a suitable free block. If found, it marks the block as used and returns it. If no suitable block is found, it allocates from the end of the heap.

When `free` is called, it marks the block as free and attempts to coalesce adjacent free blocks to reduce fragmentation.

### Console Output

Console output is implemented using the VGA text mode memory-mapped buffer at address `0xB8000`. Each character in the buffer consists of:
- An ASCII character (8 bits)
- An attribute byte (8 bits) that defines the foreground and background colors

The console supports basic control characters such as:
- `\n`: Newline
- `\r`: Carriage return
- `\t`: Tab
- `\b`: Backspace

When the screen is full, the console scrolls up, moving each line up one position and clearing the bottom line.

### Keyboard Input

Keyboard input is implemented by reading from the keyboard controller ports. When a key is pressed, the keyboard controller generates an interrupt, and the `getchar` function reads the scan code from the keyboard data port.

The scan code is then converted to an ASCII character using a simple lookup table. Note that this is a simplified implementation and doesn't handle all keys or key combinations.
