#include "inode.hpp"
#include <iostream>

using namespace std;

Inode parse_inode(char* buffer) {
    Inode inode;

    inode.mode        = read2(buffer, 0);
    inode.uid         = read2(buffer, 2);
    inode.size        = read4(buffer, 4);
    inode.gid         = read2(buffer, 24);
    inode.links_count = read2(buffer, 26);

    // read all 15 block pointers
    for (int i = 0; i < 15; i++) {
        inode.block[i] = read4(buffer, 40 + (i * 4));
    }

    return inode;
}

void print_inode(Inode& inode, int index) {
    // skip empty inodes
    if (inode.links_count == 0) return;

    cout << "--- Inode " << index << " ---" << endl;
    cout << "Type:         ";
    if (inode_is_dir(inode))       cout << "directory" << endl;
    else if (inode_is_file(inode)) cout << "regular file" << endl;
    else                           cout << "other" << endl;

    cout << "Size:         " << inode.size << " bytes" << endl;
    //cout << "UID:          " << inode.uid << endl;
    //cout << "GID:          " << inode.gid << endl;
    cout << "Links count:  " << inode.links_count << endl;
    cout << "First block:  " << inode.block[0] << endl;
}

bool inode_is_file(Inode& inode) {
    return (inode.mode & 0xF000) == EXT2_S_IFREG;
}

bool inode_is_dir(Inode& inode) {
    return (inode.mode & 0xF000) == EXT2_S_IFDIR;
}