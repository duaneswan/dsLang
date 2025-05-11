/**
 * memory.c - Memory management functions for dsOS
 * 
 * This file implements the core memory management functionality for dsLang programs
 * running on dsOS, including simple heap allocation, memory copying, and
 * memory manipulation functions.
 */

// Added comment to check to see if my automated python script is working correctly

#include <stddef.h>

// Define size_t if not already defined
#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned long size_t;
#endif

// Simple heap implementation
// -------------------------------

// Heap size (1MB for now)
#define HEAP_SIZE (1024 * 1024)

// Heap memory area (placed by the linker)
static unsigned char heap[HEAP_SIZE] __attribute__((section(".heap")));

// Current position in the heap
static size_t heap_position = 0;

// Memory block header structure
typedef struct {
    size_t size;      // Size of the block (not including header)
    unsigned char used;   // 1 if block is used, 0 if free
} BlockHeader;

#define HEADER_SIZE sizeof(BlockHeader)

/**
 * Allocate memory from the heap
 * 
 * This is a very simple first-fit allocator. It searches for a free block
 * of sufficient size. If found, it marks the block as used and returns it.
 * If not found, it allocates from the end of the heap.
 * 
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL if allocation failed
 */
void* malloc(size_t size) {
    // Align size to 8 bytes for better memory alignment
    size = (size + 7) & ~7;
    
    if (size == 0) {
        return NULL;
    }
    
    // Search for a suitable free block
    size_t pos = 0;
    while (pos < heap_position) {
        BlockHeader* header = (BlockHeader*)(heap + pos);
        
        if (!header->used && header->size >= size) {
            // Found a suitable free block
            header->used = 1;
            
            // Split the block if it's significantly larger than requested
            if (header->size >= size + HEADER_SIZE + 16) {
                size_t remaining = header->size - size - HEADER_SIZE;
                header->size = size;
                
                // Create a new free block header after this block
                BlockHeader* next = (BlockHeader*)(heap + pos + HEADER_SIZE + size);
                next->size = remaining;
                next->used = 0;
            }
            
            return heap + pos + HEADER_SIZE;
        }
        
        // Move to the next block
        pos += HEADER_SIZE + header->size;
    }
    
    // No suitable free block found, allocate at end of heap
    if (heap_position + HEADER_SIZE + size > HEAP_SIZE) {
        // Out of memory
        return NULL;
    }
    
    BlockHeader* header = (BlockHeader*)(heap + heap_position);
    header->size = size;
    header->used = 1;
    
    void* ptr = heap + heap_position + HEADER_SIZE;
    heap_position += HEADER_SIZE + size;
    
    return ptr;
}

/**
 * Free previously allocated memory
 * 
 * @param ptr Pointer to memory to free
 */
void free(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    
    // Check if ptr is within our heap
    if (ptr < (void*)heap || ptr >= (void*)(heap + HEAP_SIZE)) {
        return;
    }
    
    // Get the block header
    BlockHeader* header = (BlockHeader*)((unsigned char*)ptr - HEADER_SIZE);
    
    // Mark the block as free
    header->used = 0;
    
    // Simple coalescing of adjacent free blocks
    // (This is a basic implementation and could be improved)
    size_t pos = 0;
    while (pos < heap_position) {
        BlockHeader* current = (BlockHeader*)(heap + pos);
        
        // If this is a free block, check if the next block is also free
        if (!current->used) {
            size_t next_pos = pos + HEADER_SIZE + current->size;
            if (next_pos < heap_position) {
                BlockHeader* next = (BlockHeader*)(heap + next_pos);
                if (!next->used) {
                    // Coalesce the blocks
                    current->size += HEADER_SIZE + next->size;
                    continue;  // Check again for more coalescing
                }
            }
        }
        
        // Move to the next block
        pos += HEADER_SIZE + current->size;
    }
}

// Memory utility functions
// -------------------------------

/**
 * Fill a block of memory with a specified value
 * 
 * @param dest Pointer to the memory block
 * @param value Value to fill with (converted to unsigned char)
 * @param count Number of bytes to fill
 * @return The dest pointer
 */
void* memset(void* dest, int value, size_t count) {
    unsigned char* d = (unsigned char*)dest;
    unsigned char v = (unsigned char)value;
    
    for (size_t i = 0; i < count; i++) {
        d[i] = v;
    }
    
    return dest;
}

/**
 * Copy a block of memory from one location to another
 * (The blocks must not overlap)
 * 
 * @param dest Destination memory block
 * @param src Source memory block
 * @param count Number of bytes to copy
 * @return The dest pointer
 */
void* memcpy(void* dest, const void* src, size_t count) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    for (size_t i = 0; i < count; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

/**
 * Copy a block of memory from one location to another,
 * properly handling overlapping regions
 * 
 * @param dest Destination memory block
 * @param src Source memory block
 * @param count Number of bytes to copy
 * @return The dest pointer
 */
void* memmove(void* dest, const void* src, size_t count) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    // If the destination is before the source, copy forward
    if (d < s) {
        for (size_t i = 0; i < count; i++) {
            d[i] = s[i];
        }
    } 
    // If the destination is after the source, copy backward
    else if (d > s) {
        for (size_t i = count; i > 0; i--) {
            d[i-1] = s[i-1];
        }
    }
    
    return dest;
}

/**
 * Compare two blocks of memory
 * 
 * @param ptr1 First memory block
 * @param ptr2 Second memory block
 * @param count Number of bytes to compare
 * @return 0 if blocks are equal, <0 if ptr1 < ptr2, >0 if ptr1 > ptr2
 */
int memcmp(const void* ptr1, const void* ptr2, size_t count) {
    const unsigned char* p1 = (const unsigned char*)ptr1;
    const unsigned char* p2 = (const unsigned char*)ptr2;
    
    for (size_t i = 0; i < count; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    
    return 0;
}
