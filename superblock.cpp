#include "superblock.hpp"

using namespace std;

// fills Superblock struct from raw buffer
Superblock parse_superblock(char* buffer) {
    Superblock sb;

    sb.inodes_count      = read4(buffer, 0);
    sb.blocks_count      = read4(buffer, 4);
    sb.first_data_block  = read4(buffer, 20);
    sb.log_block_size    = read4(buffer, 24);
    sb.blocks_per_group  = read4(buffer, 32);
    sb.inodes_per_group  = read4(buffer, 40);
    sb.magic             = read2(buffer, 56);
    sb.state             = read2(buffer, 58);
    sb.rev_level        = read4(buffer, 76);

    if (sb.rev_level == 0) sb.first_ino = 11;   // EXT2_GOOD_OLD_FIRST_INO
    else sb.first_ino = read4(buffer, 84);

    if (sb.rev_level == 0) sb.inode_size = 128; // in revision 0 inode size field does not exist, it is always 128
    else sb.inode_size = read2(buffer, 88);     //in revision 1+ inode size is read from superblock

    // calculate actual block size from log value
    sb.block_size = 1024 << sb.log_block_size;  //min possible block is 1kB

    return sb;
}

// print what we found
void print_superblock(Superblock& sb) {
    cout << "*** EXT2 SUPERBLOCK ***" << endl;
    cout << "Magic number: 0x" << hex << sb.magic << endl;
    cout << "Total inodes: " << dec << sb.inodes_count << endl;
    cout << "Total blocks: " << sb.blocks_count << endl;
    cout << "Block size:   " << sb.block_size<< " bytes" << endl;
    cout << "Blocks/group:  " << sb.blocks_per_group << endl;
    cout << "Inodes/group:  " << sb.inodes_per_group << endl;
    cout << "Revision:      " << sb.rev_level << endl;
    cout << "Inode size:    " << sb.inode_size << " bytes" << endl;
    cout << "First inode:   " << sb.first_ino << endl;
    cout << "State:         " << sb.state << endl;
}