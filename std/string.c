/**
 * string.c - String handling functions for dsOS
 * 
 * This file implements string manipulation functions for dsLang programs
 * running on dsOS, providing essential functionality for working with 
 * null-terminated strings.
 */

//Added this comment here to see if my automated python script actually works!

#include <stddef.h>

// Define size_t if not already defined
#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned long size_t;
#endif

/**
 * Calculate the length of a string
 * 
 * @param str Null-terminated string
 * @return Number of characters in the string, excluding the null terminator
 */
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

/**
 * Copy a string
 * 
 * @param dest Destination buffer
 * @param src Source string
 * @return The dest pointer
 */
char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++) != '\0');
    return original_dest;
}

/**
 * Copy at most n characters from a string
 * 
 * @param dest Destination buffer
 * @param src Source string
 * @param n Maximum number of characters to copy
 * @return The dest pointer
 */
char* strncpy(char* dest, const char* src, size_t n) {
    char* original_dest = dest;
    
    // Copy up to n characters
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    
    // Pad with null characters if needed
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    
    return original_dest;
}

/**
 * Concatenate two strings
 * 
 * @param dest Destination buffer (null-terminated)
 * @param src Source string to append
 * @return The dest pointer
 */
char* strcat(char* dest, const char* src) {
    char* original_dest = dest;
    
    // Find the end of the destination string
    while (*dest != '\0') {
        dest++;
    }
    
    // Copy the source string
    while ((*dest++ = *src++) != '\0');
    
    return original_dest;
}

/**
 * Concatenate at most n characters from src to dest
 * 
 * @param dest Destination buffer (null-terminated)
 * @param src Source string to append
 * @param n Maximum number of characters to append
 * @return The dest pointer
 */
char* strncat(char* dest, const char* src, size_t n) {
    char* original_dest = dest;
    
    // Find the end of the destination string
    while (*dest != '\0') {
        dest++;
    }
    
    // Copy at most n characters from the source
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    
    // Add null terminator
    dest[i] = '\0';
    
    return original_dest;
}

/**
 * Compare two strings lexicographically
 * 
 * @param str1 First string
 * @param str2 Second string
 * @return 0 if equal, <0 if str1 < str2, >0 if str1 > str2
 */
int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return (unsigned char)*str1 - (unsigned char)*str2;
}

/**
 * Compare at most n characters of two strings lexicographically
 * 
 * @param str1 First string
 * @param str2 Second string
 * @param n Maximum number of characters to compare
 * @return 0 if equal (up to n characters), <0 if str1 < str2, >0 if str1 > str2
 */
int strncmp(const char* str1, const char* str2, size_t n) {
    if (n == 0) {
        return 0;
    }
    
    while (--n && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    
    return (unsigned char)*str1 - (unsigned char)*str2;
}

/**
 * Find the first occurrence of a character in a string
 * 
 * @param str String to search
 * @param ch Character to find
 * @return Pointer to the first occurrence of ch in str, or NULL if not found
 */
char* strchr(const char* str, int ch) {
    while (*str != '\0' && *str != (char)ch) {
        str++;
    }
    
    return (*str == (char)ch) ? (char*)str : NULL;
}

/**
 * Find the last occurrence of a character in a string
 * 
 * @param str String to search
 * @param ch Character to find
 * @return Pointer to the last occurrence of ch in str, or NULL if not found
 */
char* strrchr(const char* str, int ch) {
    const char* last = NULL;
    
    while (*str != '\0') {
        if (*str == (char)ch) {
            last = str;
        }
        str++;
    }
    
    // Also check null terminator if ch is '\0'
    if ((char)ch == '\0') {
        return (char*)str;
    }
    
    return (char*)last;
}

/**
 * Convert string to integer
 * 
 * @param str String to convert
 * @return Integer value
 */
int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    
    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }
    
    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // Convert digits
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return sign * result;
}

/**
 * Convert integer to string
 * 
 * @param value Integer to convert
 * @param str Buffer to store the string
 * @return Pointer to the string
 */
char* itoa(int value, char* str) {
    char* original_str = str;
    int is_negative = 0;
    
    // Handle negative numbers
    if (value < 0) {
        is_negative = 1;
        value = -value;
    }
    
    // Handle special case of 0
    if (value == 0) {
        *str++ = '0';
        *str = '\0';
        return original_str;
    }
    
    // Convert digits (in reverse order)
    while (value > 0) {
        *str++ = '0' + (value % 10);
        value /= 10;
    }
    
    // Add negative sign if needed
    if (is_negative) {
        *str++ = '-';
    }
    
    // Null terminate
    *str = '\0';
    
    // Reverse the string
    str--;  // Point to the last character
    char* start = original_str;
    char temp;
    while (start < str) {
        temp = *start;
        *start++ = *str;
        *str-- = temp;
    }
    
    return original_str;
}

/**
 * Find the first occurrence of a substring in a string
 * 
 * @param haystack String to search in
 * @param needle Substring to find
 * @return Pointer to the first occurrence of needle in haystack, or NULL if not found
 */
char* strstr(const char* haystack, const char* needle) {
    // Empty needle is always found at the beginning
    if (*needle == '\0') {
        return (char*)haystack;
    }
    
    while (*haystack) {
        // Check if needle matches starting from current position
        const char* h = haystack;
        const char* n = needle;
        
        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }
        
        // If we've reached the end of needle, it's a match
        if (*n == '\0') {
            return (char*)haystack;
        }
        
        haystack++;
    }
    
    return NULL;
}
