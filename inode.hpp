#ifndef INODE_HPP
#define INODE_HPP

#include <cstdint>
#include "diskReader.hpp"


struct Inode {
    uint16_t mode;          // file type and permissions
    uint32_t size;          // file size in bytes
    uint16_t uid;           // user id
    uint16_t gid;           // group id
    uint16_t links_count;   // hard links count
    uint32_t block[15];     // block pointers (12 direct + 3 indirect)
    
};

// file type masks
#define EXT2_S_IFREG  0x8000  // regular file
#define EXT2_S_IFDIR  0x4000  // directory
#define EXT2_S_IFLNK  0xA000  // symbolic link

Inode parse_inode(char* buffer);
void print_inode(Inode& inode, int index);
bool inode_is_file(Inode& inode);
bool inode_is_dir(Inode& inode);

#endif