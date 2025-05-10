/**
 * io.c - Console I/O functions for dsOS
 * 
 * This file provides basic console input/output functionality for dsLang
 * programs running on dsOS. It includes functions for displaying characters
 * and strings to the screen, reading keyboard input, and formatted output.
 */

#include <stddef.h>
#include <stdarg.h>

// Define size_t if not already defined
#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned long size_t;
#endif

// External functions from other modules
extern size_t strlen(const char* str);
extern char* itoa(int value, char* str);

// VGA text mode constants
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// Default text color (white on black)
#define VGA_DEFAULT_COLOR 0x07

// Current cursor position
static int cursor_x = 0;
static int cursor_y = 0;

// VGA text mode buffer pointer
static unsigned short* vga_buffer = (unsigned short*)VGA_MEMORY;

/**
 * Set the cursor position
 * 
 * @param x X coordinate (0-based, 0 is leftmost)
 * @param y Y coordinate (0-based, 0 is topmost)
 */
static void set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
    
    // The cursor position is x + y * width
    unsigned short position = y * VGA_WIDTH + x;
    
    // Send the high byte
    outb(0x3D4, 0x0E);
    outb(0x3D5, (position >> 8) & 0xFF);
    
    // Send the low byte
    outb(0x3D4, 0x0F);
    outb(0x3D5, position & 0xFF);
}

/**
 * Clear the screen
 */
void clear_screen() {
    // Fill the entire screen with blank characters
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            // The character is stored in the lower byte
            // The attribute (color) is stored in the upper byte
            vga_buffer[y * VGA_WIDTH + x] = ' ' | (VGA_DEFAULT_COLOR << 8);
        }
    }
    
    // Reset the cursor position
    set_cursor(0, 0);
}

/**
 * Scroll the screen up one line
 */
static void scroll() {
    // Move each line up one
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    // Clear the last line
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = ' ' | (VGA_DEFAULT_COLOR << 8);
    }
}

/**
 * Write a character to the screen at the current cursor position
 * 
 * @param c Character to write
 */
void putchar(char c) {
    // Handle special characters
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        // Tab is 8 spaces
        cursor_x = (cursor_x + 8) & ~7;
    } else if (c == '\b') {
        // Backspace
        if (cursor_x > 0) {
            cursor_x--;
            // Clear the character
            vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = ' ' | (VGA_DEFAULT_COLOR << 8);
        }
    } else {
        // Regular character
        vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = c | (VGA_DEFAULT_COLOR << 8);
        cursor_x++;
    }
    
    // Wrap to the next line if needed
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    // Scroll if needed
    if (cursor_y >= VGA_HEIGHT) {
        scroll();
        cursor_y = VGA_HEIGHT - 1;
    }
    
    // Update the hardware cursor
    set_cursor(cursor_x, cursor_y);
}

/**
 * Write a string to the screen
 * 
 * @param str Null-terminated string to write
 */
void puts(const char* str) {
    while (*str) {
        putchar(*str);
        str++;
    }
}

/**
 * Port I/O functions
 * These are required for console I/O and are typically implemented in assembly.
 * For simplicity, we're using GCC's inline assembly feature.
 */

/**
 * Output a byte to an I/O port
 * 
 * @param port Port address
 * @param value Byte value to output
 */
inline void outb(unsigned short port, unsigned char value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

/**
 * Input a byte from an I/O port
 * 
 * @param port Port address
 * @return Byte value read from the port
 */
inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/**
 * Output a word (16 bits) to an I/O port
 * 
 * @param port Port address
 * @param value Word value to output
 */
inline void outw(unsigned short port, unsigned short value) {
    asm volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

/**
 * Input a word (16 bits) from an I/O port
 * 
 * @param port Port address
 * @return Word value read from the port
 */
inline unsigned short inw(unsigned short port) {
    unsigned short ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/**
 * Output a long (32 bits) to an I/O port
 * 
 * @param port Port address
 * @param value Long value to output
 */
inline void outl(unsigned short port, unsigned int value) {
    asm volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

/**
 * Input a long (32 bits) from an I/O port
 * 
 * @param port Port address
 * @return Long value read from the port
 */
inline unsigned int inl(unsigned short port) {
    unsigned int ret;
    asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/**
 * Read a character from the keyboard
 * 
 * @return Character read
 */
int getchar() {
    // Wait for keyboard data
    while ((inb(0x64) & 1) == 0);
    
    // Read the scan code
    unsigned char scan_code = inb(0x60);
    
    // TODO: Implement a proper scan code to ASCII conversion
    // This is a very simplified version that only handles some keys
    if (scan_code < 0x80) { // Key press (not release)
        // Simple mapping for common keys (not a complete table)
        static const char scan_to_ascii[] = {
            0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
            '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
            0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
            0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
            '*', 0, ' ', 0
        };
        
        if (scan_code < sizeof(scan_to_ascii)) {
            return scan_to_ascii[scan_code];
        }
    }
    
    // Return 0 for unhandled keys
    return 0;
}

/**
 * Print formatted output (simplified version of printf)
 * 
 * @param format Format string
 * @param ... Variable arguments
 * @return Number of characters printed
 */
int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    int chars_printed = 0;
    char buffer[32]; // For number conversions
    
    while (*format) {
        if (*format == '%') {
            format++; // Skip %
            
            // Handle format specifiers
            switch (*format) {
                case 'd':
                case 'i': {
                    // Signed decimal integer
                    int value = va_arg(args, int);
                    itoa(value, buffer);
                    puts(buffer);
                    chars_printed += strlen(buffer);
                    break;
                }
                case 'u': {
                    // Unsigned decimal integer
                    unsigned int value = va_arg(args, unsigned int);
                    // Use itoa for simplicity, but ensure value is positive
                    itoa((int)value, buffer);
                    puts(buffer);
                    chars_printed += strlen(buffer);
                    break;
                }
                case 'x': {
                    // Hexadecimal (lowercase)
                    unsigned int value = va_arg(args, unsigned int);
                    char* p = buffer;
                    
                    // Add prefix
                    *p++ = '0';
                    *p++ = 'x';
                    
                    // Convert to hex
                    int started = 0;
                    for (int i = 28; i >= 0; i -= 4) {
                        int digit = (value >> i) & 0xF;
                        if (digit != 0 || started || i == 0) {
                            started = 1;
                            *p++ = "0123456789abcdef"[digit];
                        }
                    }
                    
                    // Null terminate
                    *p = '\0';
                    
                    puts(buffer);
                    chars_printed += strlen(buffer);
                    break;
                }
                case 'X': {
                    // Hexadecimal (uppercase)
                    unsigned int value = va_arg(args, unsigned int);
                    char* p = buffer;
                    
                    // Add prefix
                    *p++ = '0';
                    *p++ = 'X';
                    
                    // Convert to hex
                    int started = 0;
                    for (int i = 28; i >= 0; i -= 4) {
                        int digit = (value >> i) & 0xF;
                        if (digit != 0 || started || i == 0) {
                            started = 1;
                            *p++ = "0123456789ABCDEF"[digit];
                        }
                    }
                    
                    // Null terminate
                    *p = '\0';
                    
                    puts(buffer);
                    chars_printed += strlen(buffer);
                    break;
                }
                case 'c': {
                    // Character
                    char c = (char)va_arg(args, int);
                    putchar(c);
                    chars_printed++;
                    break;
                }
                case 's': {
                    // String
                    char* str = va_arg(args, char*);
                    if (str == NULL) {
                        str = "(null)";
                    }
                    puts(str);
                    chars_printed += strlen(str);
                    break;
                }
                case '%': {
                    // Literal %
                    putchar('%');
                    chars_printed++;
                    break;
                }
                default: {
                    // Unrecognized format specifier, print it as-is
                    putchar('%');
                    putchar(*format);
                    chars_printed += 2;
                    break;
                }
            }
        } else {
            // Regular character
            putchar(*format);
            chars_printed++;
        }
        
        format++;
    }
    
    va_end(args);
    return chars_printed;
}
