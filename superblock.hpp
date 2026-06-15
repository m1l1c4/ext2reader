#ifndef SUPERBLOCK_HPP
#define SUPERBLOCK_HPP

#include <cstdint>
#include <fstream>
#include <iostream>
#include <cstring>
#include "diskReader.hpp"



// Superblock is always at offset 1024 bytes
#define SUPERBLOCK_OFFSET 1024
#define SUPERBLOCK_SIZE   1024

/*
IZVORI
ext2 specification:https://www.nongnu.org/ext2-doc/ext2.html
kernel source: https://github.com/torvalds/linux/blob/master/fs/ext2/ext2.h

OSDev wiki https://wiki.osdev.org/Ext2
wiki ext2 https://en.wikipedia.org/wiki/Ext2
*/

// ext2 superblock structure

struct Superblock {
    uint32_t inodes_count;      // total number of inodes
    uint32_t blocks_count;      // total number of blocks
    uint32_t first_data_block;  // ← add this back!
    uint32_t log_block_size;    // block size (as power of 2)
    uint32_t blocks_per_group;  // blocks per group
    uint32_t inodes_per_group;  // inodes per group
    uint16_t magic;             // magic number (0xEF53)
    uint16_t state;             // filesystem state
    uint32_t rev_level;         // revision level    
    uint16_t inode_size;        // size of each inode structure
    uint32_t block_size;        // calculated, not from disk
};


Superblock parse_superblock(char* buffer);
void print_superblock(Superblock& sb);


#endif