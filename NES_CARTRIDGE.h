//
// Created by aco on 02.07.24.
//
#include <stdlib.h>

using namespace std;

#ifndef NESEMU_NES_CARTRIDGE_H
#define NESEMU_NES_CARTRIDGE_H

//#include "NES_PPU.h"
class NES_PPU;


class NES_CARTRIDGE_MEMORY{
public:
    char mem[16*1024];

    char read(unsigned short int adr);
};

class NES_CARTRIDGE_CHR_MEMORY{
public:
    char mem[8*1024];

    char read(u_int16_t adr);
};

class NES_CARTRIDGE {
public:
    char header[16];

    NES_PPU *ppu;
    //u_int8_t ppuVRAM[0x800];

    unsigned char PRG_SIZE;
    unsigned char CHR_SIZE;
    unsigned char mapper;
    bool mirroring;

    NES_CARTRIDGE_MEMORY *memory[10];
    NES_CARTRIDGE_CHR_MEMORY *CHR[10];

    NES_CARTRIDGE(char *file_path);


    char read(u_int16_t adr);
    char ppu_read(u_int16_t adr);
    void ppu_write(u_int8_t X,u_int16_t adr);
};


#endif //NESEMU_NES_CARTRIDGE_H
