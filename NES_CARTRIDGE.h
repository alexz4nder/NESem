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

class NES_CARTRIDGE_RAM{
public:
    char mem[16*1024];

    char read(unsigned short int adr);
    char write(u_int8_t X,u_int16_t adr);
};


class NES_CARTRIDGE_CHR_MEMORY{
public:
    char mem[8*1024];

    char read(u_int16_t adr);
};

typedef struct mmc1{
    u_int8_t shift_reg;
    u_int8_t control_reg;
    u_int8_t chr0_reg;
    u_int8_t chr1_reg;
    u_int8_t prg_reg;
    u_int8_t shift_counter;
    NES_CARTRIDGE_RAM *mmc1_ram;
}Mmc1;
typedef struct NROM{
    NES_CARTRIDGE_RAM *nrom_ram;
}NROM;

class NES_CARTRIDGE {
public:
    char header[16];
    bool iNES2;

    NES_PPU *ppu;
    //u_int8_t ppuVRAM[0x800];

    unsigned char PRG_SIZE;
    unsigned char CHR_SIZE;
    unsigned char mapper;
    void *mapper_data;
    u_int8_t mirroring;
    bool mirroring_changable;

    NES_CARTRIDGE_MEMORY *memory[16];
    NES_CARTRIDGE_CHR_MEMORY *CHR[16];

    NES_CARTRIDGE(char *file_path);


    char read(u_int16_t adr);
    void write(u_int8_t  X,u_int16_t adr);
    char ppu_read(u_int16_t adr);
    void ppu_write(u_int8_t X,u_int16_t adr);
};


#endif //NESEMU_NES_CARTRIDGE_H
