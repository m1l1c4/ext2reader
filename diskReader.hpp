#ifndef DISKREADER_HPP
#define DISKREADER_HPP

#include <cstdint>
#include <fstream>
#include <iostream>
#include <cstring>

bool open_image(const char* path, std::ifstream& img);
void read_bytes(std::ifstream& img, char* buffer, int offset, int size);

uint32_t read4(char* buf, int index);
uint16_t read2(char* buf, int index);

#endif