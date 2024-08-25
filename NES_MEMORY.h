//
// Created by aco on 26.06.24.
//
#include "NES_CARTRIDGE.h"
#ifndef NESEMU_NES_MEMORY_H
#define NESEMU_NES_MEMORY_H
#include "NES_PPU.h"
class NES_APU;
class NES_CPU;
class NES_MEMORY {
public:
    char RAM[0x800];
    NES_PPU *ppu;
    NES_CPU *cpu;
    NES_APU *apu;

    NES_CARTRIDGE *Cartridge;

    NES_MEMORY();

    void LoadCartridge(NES_CARTRIDGE *new_cartridge);

    struct controller{
        u_int8_t reg;
    } Controller;

    void capture_controller_state();

    //READ
    char read(unsigned short int adr);
    //WRITE
    void write(char X,unsigned short int adr);


    void dump_ram(char *file_name);

};


#endif //NESEMU_NES_MEMORY_H
