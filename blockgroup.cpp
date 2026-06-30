#include "blockgroup.hpp"
#include <iostream>

using namespace std;

BlockGroup parse_blockgroup(char* buffer) {
    BlockGroup bg;
    bg.block_bitmap      = read4(buffer, 0);
    bg.inode_bitmap      = read4(buffer, 4);
    bg.inode_table       = read4(buffer, 8);
    bg.free_blocks_count = read2(buffer, 12);
    bg.free_inodes_count = read2(buffer, 14);
    bg.used_dirs_count   = read2(buffer, 16);

    return bg;
}

void print_blockgroup(BlockGroup& bg, int index) {
    cout << "*** BLOCK GROUP " << index << " ***" << endl;
    cout << "Block bitmap:       " << bg.block_bitmap << endl;
    cout << "Inode bitmap:       " << bg.inode_bitmap << endl;
    cout << "Inode table:        " << bg.inode_table << endl;
    cout << "Free blocks:        " << bg.free_blocks_count << endl;
    cout << "Free inodes:        " << bg.free_inodes_count << endl;
    cout << "Used dirs:          " << bg.used_dirs_count << endl;
}