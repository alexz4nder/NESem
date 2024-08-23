//
// Created by aco on 18.08.24.
//

#include <cstdlib>
#include <SDL2/SDL.h>
#include "NES_PPU.h"
#include "NES_CPU.h"
#ifndef NESEMU_NES_APU_H
#define NESEMU_NES_APU_H


class NES_APU {
public:
    NES_PPU *ppu;
    NES_CPU *cpu;
    NES_APU(NES_PPU *nes_ppu,NES_CPU *nes_cpu);
    SDL_AudioSpec au_spec;

    void exec();
    int16_t generate_sample();
    void start_clocking();

    union status_reg{

        u_int8_t reg;
        struct status_reg_data{
            u_int8_t P1:1;
            u_int8_t P2:1;
            u_int8_t T:1;
            u_int8_t N:1;
            u_int8_t D:1;
        }data;

    }status_reg;

    bool frame_counter_mode;

//PULSE CHANNELS
    union pulse_reg_1{
        u_int8_t reg;
        struct pulse_1_data{
            u_int8_t V:4;
            u_int8_t C:1;
            u_int8_t L:1;
            u_int8_t D:2;
        }data;
    };

    u_int8_t p1_envelope;
    u_int8_t p2_envelope;

    pulse_reg_1 p1_reg1;
    pulse_reg_1 p2_reg1;

    int16_t p1_timer_value;
    int16_t p2_timer_value;

    int16_t p1_timer;
    int16_t p2_timer;

    u_int8_t p1_timer_duty[4];
    u_int8_t p2_timer_duty[4];

    u_int16_t p1_lenght_counter;
    u_int16_t p2_lenght_counter;

    //TRIANGLE

    bool triangle_C;
    u_int8_t triangle_linear_counter_reload_value;

    int16_t triangle_timer_value;
    int16_t triangle_timer;
    u_int8_t triangle_sequencer_index;

    bool reload_flag;

    int16_t triangle_lenght_counter;

    //NOISE
    union noise_reg{
        u_int8_t reg;
        struct noise_data{
            u_int8_t V:4;
            u_int8_t C:1;
            u_int8_t L:1;
        }data;
    }noise_reg;

    int16_t noise_timer;
    int16_t noise_timer_value;
    bool noise_mode_flag;
    u_int16_t noise_lenght;

    u_int16_t noise_shift_reg;


    //LENGHT COUNTER LOOKUP TABLE
    u_int16_t lenght_counter_table[0x20];
    u_int16_t triangle_sequencer_table[32]={15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    int16_t noise_sequencer_timer_table[32]={4,8,16,32,64,96,128,160,202,254,380,508,762,1016,2034,4068};
};


#endif //NESEMU_NES_APU_H
