#include "diskReader.hpp"

using namespace std;


bool open_image(const char* path, ifstream& img) {
    img.open(path, ios::binary);
    if (!img) {
        cerr << "Ne mogu da otvorim: " << path << endl;
        return false;
    }
    return true;
}

void read_bytes(ifstream& img, char* buffer, int offset, int size) {
    img.seekg(offset);  //moves the file read position to the "offset" (given byte position)
    img.read(buffer, size);
}

// read 4 and read 2 bytes from buffer at given index (little-endian)
uint32_t read4(char* buf, int index) {
    uint32_t value;
    memcpy(&value, buf + index, 4);
    return value;
}

uint16_t read2(char* buf, int index) {
    uint16_t value;
    memcpy(&value, buf + index, 2);
    return value;
}
