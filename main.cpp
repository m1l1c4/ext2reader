#include <iostream>
#include "superblock.hpp"
#include "diskReader.hpp"
#include "blockgroup.hpp"
#include "inode.hpp"
#include "directory.hpp"
#include "html.hpp"


using namespace std;

int main() {
    // DISK IMAGE - open ext2 filesystem image for reading
    ifstream img;

    if (!open_image("myfs.img", img)) {
        return 1;
    }
    //***************************************************** */
    // SUPERBLOCK - read the information about the configuration of the filesystem
    char buffer[SUPERBLOCK_SIZE];
    read_bytes(img, buffer, SUPERBLOCK_OFFSET, SUPERBLOCK_SIZE);
   
    Superblock sb = parse_superblock(buffer);
    if (sb.magic != 0xEF53) {
        cerr << "Nije ext2!" << endl;
        return 1;
    }

    print_superblock(sb);
    /***************************************************** */
    // BLOCKGROPUS - read block group descriptor

    // total blocks / blocks per group, rounded up - so we do not lose any blocks in the last group by integer division
    int num_groups = (sb.blocks_count + sb.blocks_per_group - 1) / sb.blocks_per_group;
    cout << "\nNumber of block groups: " << num_groups << endl;

    // block group descriptor table starts at block after superblock
    //which is 3rd block (block 2) for 1KiB, 2nd block (block 1) for 2KiB and larger
    uint32_t bgdt_offset = (sb.first_data_block + 1) * sb.block_size;

    // each descriptor is 32 bytes
    //"For each block group in the file system, such a group_desc is created. 
    //Each represent a single block group within the file system and the information within any one of them is pertinent only to the group it is describing. 
    //Every block group descriptor table contains all the information about all the block groups. "
    int descriptor_size = 32;
    BlockGroup bg[num_groups];

    // read and print each block group descriptor
    for (int i = 0; i < num_groups; i++) {
        char bg_buffer[descriptor_size];        //create 32 byte buffer for each block group descriptor
        read_bytes(img, bg_buffer, bgdt_offset + (i * descriptor_size), descriptor_size); //Read 32 bytes from disk at bgdt_offset + (i × 32) — each descriptor is right after the previous one
        bg[i] = parse_blockgroup(bg_buffer);    //Parse buffer into bg[i] struct
        print_blockgroup(bg[i], i);
    }
    /************************************************************* */
    //INODES
     // loop through all block groups and read their inode tables
    for (int j = 0; j < num_groups; j++) {
        uint32_t inode_table_offset = bg[j].inode_table * sb.block_size;

        for (uint32_t i = 0; i < sb.inodes_per_group; i++) {
        char inode_buffer[sb.inode_size];
        read_bytes(img, inode_buffer,
                   inode_table_offset + (i * sb.inode_size),
                   sb.inode_size);

        Inode inode = parse_inode(inode_buffer);
        // global inode number = group * inodes_per_group + local inode + 1
        print_inode(inode, j * sb.inodes_per_group + i + 1);
    }}
    /********************************************************** */
    //DIRECTORIES
    cout << "\n*** DIRECTORY ENTRIES ***" << endl;

    // read root inode
    uint32_t root_inode_offset = bg[0].inode_table * sb.block_size + (2 - 1) * sb.inode_size;
    char root_inode_buffer[sb.inode_size];
    read_bytes(img, root_inode_buffer, root_inode_offset, sb.inode_size);
    Inode root_inode = parse_inode(root_inode_buffer);

    // recursively read all directories
    cout << "--- Root directory / ---" << endl;
    read_directory(img, root_inode, bg, sb, 0);
    
    //********************************************************** */
    //HTML GENERATION
    cout << "\n*** HTML GENERATION ***" << endl;
    generate_html(img, sb, bg, num_groups);
    //********************************************************** */


    img.close();
    return 0;
}