#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include <cstdint>
#include <string>
#include <fstream>
#include "inode.hpp"
#include "superblock.hpp"
#include "blockgroup.hpp"

#define EXT2_FT_UNKNOWN  0
#define EXT2_FT_REG_FILE 1
#define EXT2_FT_DIR      2
#define EXT2_FT_SYMLINK  7

struct DirEntry {
    uint32_t    inode;      // inode number
    uint16_t    rec_len;    // total size of this entry
    uint8_t     name_len;   // length of filename
    uint8_t     file_type;  // file type
    std::string name;       // filename
};

void read_directory(std::ifstream& img, Inode& dir_inode,
                    BlockGroup* bg, Superblock& sb,
                    int depth);

DirEntry parse_direntry(char* buffer);
void print_direntry(DirEntry& entry);

#endif