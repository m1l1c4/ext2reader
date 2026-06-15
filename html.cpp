#include "html.hpp"
#include "diskReader.hpp"
#include <iostream>
using namespace std;

// helper to write navigation bar
static void write_nav(ofstream& f) {
    f << "<nav>\n";
    f << "  <a href='index.html'>Home</a> | \n";
    f << "  <a href='superblock.html'>Superblock</a> | \n";
    f << "  <a href='blockgroups.html'>Block Groups</a> | \n";
    f << "  <a href='inodes.html'>Inodes</a> | \n";
    f << "  <a href='directory.html'>Directory</a>\n";
    f << "</nav><hr>\n";
}

void write_superblock_html(Superblock& sb) {
    ofstream f("output/superblock.html");

    f << "<html><body>\n";
    write_nav(f);
    f << "<h1>EXT2 Superblock</h1>\n";
    f << "<table border='1'>\n";
    f << "<tr><th>Field</th><th>Value</th></tr>\n";
    f << "<tr><td>Magic number</td><td>0x" << hex << sb.magic << "</td></tr>\n";
    f << "<tr><td>Total inodes</td><td>" << dec << sb.inodes_count << "</td></tr>\n";
    f << "<tr><td>Total blocks</td><td>" << sb.blocks_count << "</td></tr>\n";
    f << "<tr><td>Block size</td><td>" << sb.block_size << " bytes</td></tr>\n";
    f << "<tr><td>Blocks per group</td><td>" << sb.blocks_per_group << "</td></tr>\n";
    f << "<tr><td>Inodes per group</td><td>" << sb.inodes_per_group << "</td></tr>\n";
    f << "<tr><td>Revision</td><td>" << sb.rev_level << "</td></tr>\n";
    f << "<tr><td>Inode size</td><td>" << sb.inode_size << " bytes</td></tr>\n";
    f << "<tr><td>State</td><td>" << (sb.state == 1 ? "Clean" : "Errors") << "</td></tr>\n";
    f << "</table>\n";
    f << "</body></html>\n";

    cout << "Generated output/superblock.html" << endl;
}

void write_blockgroups_html(BlockGroup* bg, int num_groups) {
    ofstream f("output/blockgroups.html");

    f << "<html><body>\n";
    write_nav(f);
    f << "<h1>Block Group Descriptors</h1>\n";
    f << "<table border='1'>\n";
    f << "<tr><th>Group</th><th>Inode Table</th><th>Free Blocks</th><th>Free Inodes</th><th>Used Dirs</th></tr>\n";

    for (int i = 0; i < num_groups; i++) {
        f << "<tr>";
        f << "<td>" << i << "</td>";
        f << "<td>" << bg[i].inode_table << "</td>";
        f << "<td>" << bg[i].free_blocks_count << "</td>";
        f << "<td>" << bg[i].free_inodes_count << "</td>";
        f << "<td>" << bg[i].used_dirs_count << "</td>";
        f << "</tr>\n";
    }

    f << "</table>\n";
    f << "</body></html>\n";

    cout << "Generated output/blockgroups.html" << endl;
}

void write_inodes_html(std::ifstream& img, Superblock& sb, BlockGroup* bg, int num_groups) {
    ofstream f("output/inodes.html");

    f << "<html><body>\n";
    write_nav(f);
    f << "<h1>Inode Table</h1>\n";
    f << "<table border='1'>\n";
    f << "<tr><th>Inode</th><th>Type</th><th>Size</th><th>UID</th><th>GID</th><th>Links</th><th>First Block</th></tr>\n";

    for (int g = 0; g < num_groups; g++) {
        uint32_t inode_table_offset = bg[g].inode_table * sb.block_size;
        for (uint32_t i = 0; i < sb.inodes_per_group; i++) {
            char inode_buffer[sb.inode_size];
            read_bytes(img, inode_buffer,
                      inode_table_offset + (i * sb.inode_size),
                      sb.inode_size);
            Inode inode = parse_inode(inode_buffer);

            if (inode.links_count == 0) continue;

            f << "<tr>";
            f << "<td>" << (g * sb.inodes_per_group + i + 1) << "</td>";
            f << "<td>";
            if (inode_is_dir(inode))       f << "directory";
            else if (inode_is_file(inode)) f << "file";
            else                           f << "other";
            f << "</td>";
            f << "<td>" << inode.size << "</td>";
            f << "<td>" << inode.uid << "</td>";
            f << "<td>" << inode.gid << "</td>";
            f << "<td>" << inode.links_count << "</td>";
            f << "<td>" << inode.block[0] << "</td>";
            f << "</tr>\n";
        }
    }

    f << "</table>\n";
    f << "</body></html>\n";

    cout << "Generated output/inodes.html" << endl;
}

static void write_dir_recursive(ofstream& f, std::ifstream& img,
                                  Inode& dir_inode, BlockGroup* bg,
                                  Superblock& sb, int depth) {
    char dir_buffer[sb.block_size];
    uint32_t block_offset = dir_inode.block[0] * sb.block_size;
    read_bytes(img, dir_buffer, block_offset, sb.block_size);

    uint32_t offset = 0;
    while (offset < sb.block_size) {
        DirEntry entry = parse_direntry(dir_buffer + offset);
        if (entry.rec_len == 0) break;
        if (entry.inode != 0) {
            // indent with spaces
            for (int i = 0; i < depth; i++) f << "&nbsp;&nbsp;";

            f << "<tr><td>";
            for (int i = 0; i < depth; i++) f << "&nbsp;&nbsp;";
            f << entry.name << "</td>";
            f << "<td>" << entry.inode << "</td>";
            f << "<td>";
            switch (entry.file_type) {
                case EXT2_FT_REG_FILE: f << "file"; break;
                case EXT2_FT_DIR:      f << "directory"; break;
                case EXT2_FT_SYMLINK:  f << "symlink"; break;
                default:               f << "unknown"; break;
            }
            f << "</td></tr>\n";

            if (entry.file_type == EXT2_FT_DIR &&
                entry.name != "." &&
                entry.name != "..") {
                uint32_t sub_inode_offset = bg[0].inode_table * sb.block_size +
                                           (entry.inode - 1) * sb.inode_size;
                char sub_inode_buffer[sb.inode_size];
                read_bytes(img, sub_inode_buffer, sub_inode_offset, sb.inode_size);
                Inode sub_inode = parse_inode(sub_inode_buffer);
                write_dir_recursive(f, img, sub_inode, bg, sb, depth + 1);
            }
        }
        offset += entry.rec_len;
    }
}

void write_directory_html(std::ifstream& img, Superblock& sb, BlockGroup* bg) {
    ofstream f("output/directory.html");

    f << "<html><body>\n";
    write_nav(f);
    f << "<h1>Directory Structure</h1>\n";
    f << "<table border='1'>\n";
    f << "<tr><th>Name</th><th>Inode</th><th>Type</th></tr>\n";

    // start from root inode
    uint32_t root_inode_offset = bg[0].inode_table * sb.block_size + (2 - 1) * sb.inode_size;
    char root_inode_buffer[sb.inode_size];
    read_bytes(img, root_inode_buffer, root_inode_offset, sb.inode_size);
    Inode root_inode = parse_inode(root_inode_buffer);

    write_dir_recursive(f, img, root_inode, bg, sb, 0);

    f << "</table>\n";
    f << "</body></html>\n";

    cout << "Generated output/directory.html" << endl;
}

void generate_html(std::ifstream& img, Superblock& sb, BlockGroup* bg, int num_groups) {
    // generate index.html
    ofstream f("output/index.html");
    f << "<html><body>\n";
    write_nav(f);
    f << "<h1>EXT2 Filesystem Visualizer</h1>\n";
    f << "<p>Izaberite sekciju iz navigacionog panela iznad naslova.</p>\n";
    f << "</body></html>\n";
    cout << "Generated output/index.html" << endl;

    write_superblock_html(sb);
    write_blockgroups_html(bg, num_groups);
    write_inodes_html(img, sb, bg, num_groups);
    write_directory_html(img, sb, bg);
}