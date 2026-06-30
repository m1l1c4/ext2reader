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
    for (int i = 0; i < 15; i++) inode.block[i] = read4(buffer, 40 + (i * 4));

    return inode;
}

void print_inode(Inode& inode, int index, ifstream& img, Superblock& sb) {
    // skip empty inodes
    if (inode.links_count == 0) return;

    // check if this is a reserved inode (except root which is always inode 2)
    if ((uint32_t)index < sb.first_ino && index != 2) {
        cout << "--- Inode " << index << " RESERVED ---" << endl;
        return;
    }

    cout << "--- Inode " << index << " ---" << endl;
    cout << "Type:         ";
    if (inode_is_dir(inode))       cout << "directory" << endl;
    else if (inode_is_symlink(inode))  cout << "symlink" << endl;
    else if (inode_is_file(inode)) cout << "regular file" << endl;
    else                           cout << "other" << endl;

    cout << "Size:         " << inode.size << " bytes" << endl;
    //cout << "UID:          " << inode.uid << endl;
    //cout << "GID:          " << inode.gid << endl;
    cout << "Links count:  " << inode.links_count << endl;
    cout << "First block:  " << inode.block[0] << endl;
    cout << "First block:  ";
    if (inode_is_symlink(inode) && is_fast_symlink(inode)) {
        cout << "N/A (stored in inode)" << endl;
    } else {
        cout << inode.block[0] << endl;
    }
    if (inode_is_symlink(inode)) {
        string target = read_symlink_target(img, inode, sb);
        cout << "Symlink target: " << target 
             << (is_fast_symlink(inode) ? " (fast)" : " (slow)") << endl;
    }
}

bool inode_is_file(Inode& inode) {
    return (inode.mode & 0xF000) == EXT2_S_IFREG;
}

bool inode_is_dir(Inode& inode) {
    return (inode.mode & 0xF000) == EXT2_S_IFDIR;
}
bool inode_is_symlink(Inode& inode) {
    return (inode.mode & 0xF000) == EXT2_S_IFLNK;
}

bool is_fast_symlink(Inode& inode) {
    return inode.size <= 60;
}

string read_symlink_target(std::ifstream& img, Inode& inode, Superblock& sb) {
    string target = "";

    if (is_fast_symlink(inode)) {
        char* raw = (char*)inode.block;
        for (uint32_t i = 0; i < inode.size; i++) {
            target += raw[i];
        }
    } else {
        char buffer[sb.block_size];
        read_bytes(img, buffer, inode.block[0] * sb.block_size, sb.block_size);
        for (uint32_t i = 0; i < inode.size; i++) {
            target += buffer[i];
        }
    }

    return target;
}

