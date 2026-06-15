#ifndef HTML_HPP
#define HTML_HPP

#include <fstream>
#include <string>
#include "superblock.hpp"
#include "blockgroup.hpp"
#include "inode.hpp"
#include "directory.hpp"

void generate_html(std::ifstream& img, Superblock& sb, BlockGroup* bg, int num_groups);
void write_superblock_html(Superblock& sb);
void write_blockgroups_html(BlockGroup* bg, int num_groups);
void write_inodes_html(std::ifstream& img, Superblock& sb, BlockGroup* bg, int num_groups);
void write_directory_html(std::ifstream& img, Superblock& sb, BlockGroup* bg);

#endif