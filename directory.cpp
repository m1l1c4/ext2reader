#include "directory.hpp"
#include "diskReader.hpp"
#include <iostream>
using namespace std;

DirEntry parse_direntry(char* buffer) {
    DirEntry entry;

    entry.inode     = read4(buffer, 0);
    entry.rec_len   = read2(buffer, 4);
    entry.name_len  = (uint8_t)buffer[6];
    entry.file_type = (uint8_t)buffer[7];

    // read filename manually byte by byte
    entry.name = "";
    for (int i = 0; i < entry.name_len; i++) {
        entry.name += buffer[8 + i];
    }

    return entry;
}

void print_direntry(DirEntry& entry) {
    if (entry.inode == 0) return; // skip empty entries

    cout << "Inode: " << entry.inode;
    cout << "  Name: " << entry.name;
    cout << "  Type: ";
    switch (entry.file_type) {
        case EXT2_FT_REG_FILE: cout << "file"; break;
        case EXT2_FT_DIR:      cout << "directory"; break;
        case EXT2_FT_SYMLINK:  cout << "symlink"; break;
        default:               cout << "unknown"; break;
    }
    cout << endl;
}

void read_directory(std::ifstream& img, Inode& dir_inode, 
                    BlockGroup* bg, Superblock& sb, 
                    int depth) {
    // read directory data block
    char dir_buffer[sb.block_size];
    uint32_t block_offset = dir_inode.block[0] * sb.block_size;
    read_bytes(img, dir_buffer, block_offset, sb.block_size);

    // walk through directory entries
    uint32_t offset = 0;

    

                            
    while (offset < sb.block_size) {
        DirEntry entry = parse_direntry(dir_buffer + offset);



        if (entry.name_len == 0) break;
        if (entry.inode != 0) {
            // print indentation based on depth
            for (int i = 0; i < depth; i++) cout << "  ";
            print_direntry(entry);

            // if this is a subdirectory (and not . or ..)
            // recursively read it too
            if (entry.file_type == EXT2_FT_DIR && 
                entry.name != "." && 
                entry.name != "..") {

                // read the subdirectory inode
                uint32_t sub_inode_offset = bg[0].inode_table * sb.block_size + 
                                           (entry.inode - 1) * sb.inode_size;
                char sub_inode_buffer[sb.inode_size];
                read_bytes(img, sub_inode_buffer, sub_inode_offset, sb.inode_size);
                Inode sub_inode = parse_inode(sub_inode_buffer);

                // recurse into subdirectory
                read_directory(img, sub_inode, bg, sb, depth + 1);
            }
        }
        offset += entry.rec_len;

    }
}