#ifndef BLOCKGROUP_HPP
#define BLOCKGROUP_HPP

#include <cstdint>
#include "diskReader.hpp"

struct BlockGroup {
    uint32_t inode_table;       // block where inode table starts ← most important!
    uint16_t free_blocks_count; // free blocks in this group
    uint16_t free_inodes_count; // free inodes in this group
    uint16_t used_dirs_count;   // number of directories in this group
};

BlockGroup parse_blockgroup(char* buffer);
void print_blockgroup(BlockGroup& bg, int index);

#endif