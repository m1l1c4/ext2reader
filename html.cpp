#include "html.hpp"
#include "diskReader.hpp"
#include "inode.hpp"
#include <iostream>
using namespace std;

// helper to write navigation bar
static void write_nav(ofstream& f) {
    f << "<nav>\n";
    f << "  <a href='index.html'>Pocetna</a> | \n";
    f << "  <a href='superblock.html'>Superblok</a> | \n";
    f << "  <a href='blockgroups.html'>Blok Grupe</a> | \n";
    f << "  <a href='inodes.html'>Inode</a> | \n";
    f << "  <a href='directory.html'>Imenici</a>\n";
    f << "</nav><hr>\n";
}

void write_superblock_html(Superblock& sb) {
    ofstream f("output/superblock.html");

    f << "<html><body>\n";
    write_nav(f);
    f << "<h1>EXT2 Superblok</h1>\n";
    f << "<table border='1'>\n";
    f << "<tr><th>Field</th><th>Value</th></tr>\n";
    f << "<tr><td>Magicni broj</td><td>0x" << hex << sb.magic << "</td></tr>\n";
    f << "<tr><td>Ukupan broj inoda</td><td>" << dec << sb.inodes_count << "</td></tr>\n";
    f << "<tr><td>Ukupan broj blokova</td><td>" << sb.blocks_count << "</td></tr>\n";
    f << "<tr><td>Velicina bloka</td><td>" << sb.block_size << " bytes</td></tr>\n";
    f << "<tr><td>Broj blokova u grupi</td><td>" << sb.blocks_per_group << "</td></tr>\n";
    f << "<tr><td>Broj inoda u grupi</td><td>" << sb.inodes_per_group << "</td></tr>\n";
    f << "<tr><td>Revizija</td><td>" << sb.rev_level << "</td></tr>\n";
    f << "<tr><td>Velicina inode</td><td>" << sb.inode_size << " bytes</td></tr>\n";
    f << "<tr><td>Status</td><td>" << (sb.state == 1 ? "Clean" : "Errors") << "</td></tr>\n";
    f << "</table>\n";
    f << "</body></html>\n";

    cout << "Generated output/superblock.html" << endl;
}

void write_blockgroups_html(BlockGroup* bg, int num_groups) {
    ofstream f("output/blockgroups.html");

    f << "<html><body>\n";
    write_nav(f);
    f << "<h1>Deskriptori blok grupe</h1>\n";
    f << "<table border='1'>\n";
    f << "<tr><th>Grupa</th><th>Bitmapa blokova</th><th>Bitmap inoda</th><th>Tabela inoda</th><th>Slobodni blokovi</th><th>Slobodne inode</th><th>Iskorisceni imenici</th></tr>\n";

    for (int i = 0; i < num_groups; i++) {
        f << "<tr id='group" << i << "'>";
        f << "<td>" << i << "</td>";
        f << "<td>" << bg[i].block_bitmap << "</td>";
        f << "<td>" << bg[i].inode_bitmap << "</td>";
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
    f << "<h1>Tabela inoda</h1>\n";
    f << "<table border='1'>\n";
    f << "<tr><th>Inoda</th><th>Tip</th><th>Velicina</th><th>UID</th><th>GID</th><th>Veze</th><th>Prvi blok</th><th>Blok grupa</th><th>Simbolicka veza - putanja</th></tr>\n";

    for (int g = 0; g < num_groups; g++) {
        uint32_t inode_table_offset = bg[g].inode_table * sb.block_size;
        for (uint32_t i = 0; i < sb.inodes_per_group; i++) {
            char inode_buffer[sb.inode_size];
            read_bytes(img, inode_buffer,
                      inode_table_offset + (i * sb.inode_size),
                      sb.inode_size);
            Inode inode = parse_inode(inode_buffer);

            if (inode.links_count == 0) continue;

            int inode_num = g * sb.inodes_per_group + i + 1;

            // reserved inode check (skip root which is always inode 2)
            if ((uint32_t)inode_num < sb.first_ino && inode_num != 2) {
                f << "<tr>";
                f << "<td>" << inode_num << "</td>";
                f << "<td colspan='8' style='text-align:center;'>RESERVED</td>";
                f << "</tr>\n";
                continue;
            }

            f << "<tr>";
            f << "<td>" << inode_num << "</td>";
            f << "<td>";
            if (inode_is_dir(inode))           f << "imenik";
            else if (inode_is_symlink(inode))  f << "simbolicka veza";
            else if (inode_is_file(inode))     f << "datoteka";
            else                                f << "ostalo";
            f << "</td>";
            f << "<td>" << inode.size << "</td>";
            f << "<td>" << inode.uid << "</td>";
            f << "<td>" << inode.gid << "</td>";
            f << "<td>" << inode.links_count << "</td>";

            f << "<td>";
            if (inode_is_symlink(inode) && is_fast_symlink(inode)) {
                f << "N/A (unutar inode)";
            } else {
                f << inode.block[0];
            }
            f << "</td>";

            f << "<td>" << g << "</td>";

            f << "<td>";
            if (inode_is_symlink(inode)) {
                f << read_symlink_target(img, inode, sb);
                f << (is_fast_symlink(inode) ? " (brza)" : " (spora)");
            }
            f << "</td>";

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
                case EXT2_FT_REG_FILE: f << "datoteka"; break;
                case EXT2_FT_DIR:      f << "imenik"; break;
                case EXT2_FT_SYMLINK:  f << "simbolicka veza"; break;
                default:               f << "drugo"; break;
            }
            f << "</td></tr>\n";

            if (entry.file_type == EXT2_FT_DIR &&
                entry.name != "." &&
                entry.name != "..") {

                uint32_t inode_index  = entry.inode - 1;
                uint32_t group_index  = inode_index / sb.inodes_per_group;
                uint32_t local_index  = inode_index % sb.inodes_per_group;

                uint32_t sub_inode_offset = bg[group_index].inode_table * sb.block_size +
                            local_index * sb.inode_size;


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
    f << "<h1>Struktura direktorijuma</h1>\n";
    f << "<table border='1'>\n";
    f << "<tr><th>Naziv</th><th>Inoda</th><th>Tip</th></tr>\n";

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
    f << "<h1>Vizualizacija EXT2 fajlsistema</h1>\n";
    f << "<p>Izaberite sekciju iz navigacionog panela iznad naslova.</p>\n";
    f << "</body></html>\n";
    cout << "Generated output/index.html" << endl;

    write_superblock_html(sb);
    write_blockgroups_html(bg, num_groups);
    write_inodes_html(img, sb, bg, num_groups);
    write_directory_html(img, sb, bg);
}