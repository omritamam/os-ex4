#pragma once

#include "MemoryConstants.h"

/*
 * Initialize the virtual memory.
 */
void VMinitialize();

/* Reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value){
    auto offest = virtualAddress % PAGE_SIZE;
    virtualAddress >> OFFSET_WIDTH;
    auto p2 =
    int* addr1 = 0;
    PMread(0 + 5, &addr1); // first translation
    PMread(addr1 * PAGE_SIZE + 1, &addr2); // second translation
    PMwrite(addr2 * PAGE_SIZE + 6, value);
}

/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite(uint64_t virtualAddress, word_t value);
