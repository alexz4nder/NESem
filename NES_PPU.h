//
// Created by aco on 09.07.24.
//

#ifndef NESEMU_NES_PPU_H
#define NESEMU_NES_PPU_H
#include <SDL2/SDL.h>
#include "NES_CARTRIDGE.h"

class NES_CPU;

class NES_PPU {
    SDL_Window *window;
    SDL_Renderer *renderer;

public:
    bool safe_to_render;
    u_int16_t scanline;
    u_int16_t dot;
    NES_CPU *cpu;

    NES_CARTRIDGE *cartridge;
    u_int8_t VRAM[0x1000];
    union OAM_memory{
        u_int8_t memory;
        struct OAM_atribbutes{
            u_int8_t pallet:2;
            u_int8_t unused:3;
            u_int8_t priority:1;
            u_int8_t flip_horizontal:1;
            u_int8_t flip_vertical:1;
        }data;
    }OAM[256];

    u_int8_t OAM_que[8];
    u_int8_t OAM_num;
    u_int8_t OAM_adr;

    u_int8_t read_buffer;

    union PPUCTRL_reg{
        u_int8_t reg;

        struct ppuctrl_data{
            u_int8_t base_nametable_x:1;
            u_int8_t base_nametable_y :1;
            u_int8_t adr_increment:1;
            u_int8_t sprite_pattern_table :1;
            u_int8_t background_pattern_table :1;
            u_int8_t sprite_size :1;
            u_int8_t ms_select : 1;
            u_int8_t vblank_nmi : 1;

        }data;

    }PPUCTRL;

    union STATUS_reg{
        u_int8_t reg;

        struct ppu_status_data{
            u_int8_t unused:6;
            u_int8_t S:1;
            u_int8_t vblank :1;
        }data;
    }PPUSTATUS;



    u_int8_t PPUSCROLL;
    u_int8_t PPUADR;
    u_int8_t PPUDATA;

    bool read_latch;
    u_int16_t adr;



    u_int16_t scrol_x;
    u_int16_t scrol_y;

    u_int16_t scrol_x_internal;
    u_int16_t scrol_y_internal;

    u_int8_t base_nametable_x_internal;
    u_int8_t base_nametable_y_internal;

    SDL_Color color[0x40];

    NES_PPU(SDL_Renderer *sdl_renderer,SDL_Window *sdl_window);
    void LoadCartridge(NES_CARTRIDGE *cart);
    void exec();
    u_int8_t pallets[0x20];

    u_int8_t read(u_int16_t adr);


};


#endif //NESEMU_NES_PPU_H
