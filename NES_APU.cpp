//
// Created by aco on 18.08.24.
//

#include "NES_APU.h"
#include "NES_CPU.h"
#include "NES_PPU.h"
#include <SDL2/SDL.h>


void apu_callback(void *usr_dat,Uint8 *stream,int len){
    NES_APU *apu=(NES_APU*)usr_dat;

    u_int16_t *str=(u_int16_t *)stream;
    len/=2;
    str[0]= apu->generate_sample();
    for (int i = 0; i < 20; ++i) {
        apu->cpu->exec();
        apu->ppu->exec();
        apu->ppu->exec();
        apu->ppu->exec();
        apu->cpu->exec();
        apu->ppu->exec();
        apu->ppu->exec();
        apu->ppu->exec();
        apu->exec();
    }

}

NES_APU::NES_APU(NES_PPU *nes_ppu, NES_CPU *nes_cpu) {
    au_spec.userdata=(void *)this;
    au_spec.freq=44100;
    au_spec.samples=1;
    au_spec.format=AUDIO_U16;
    au_spec.channels=1;
    au_spec.callback=apu_callback;

    ppu=nes_ppu;
    cpu=nes_cpu;

    noise_shift_reg=1;

    p1_timer_duty[0]=0b1;
    p1_timer_duty[1]=0b11;
    p1_timer_duty[2]=0b1111;
    p1_timer_duty[3]=0b11;

    p2_timer_duty[0]=0b1;
    p2_timer_duty[1]=0b11;
    p2_timer_duty[2]=0b1111;
    p2_timer_duty[3]=0b11;

    lenght_counter_table[0x0]=10;
    lenght_counter_table[0x1]=254;
    lenght_counter_table[0x2]=20;
    lenght_counter_table[0x3]=2;
    lenght_counter_table[0x4]=40;
    lenght_counter_table[0x5]=4;
    lenght_counter_table[0x6]=80;
    lenght_counter_table[0x7]=6;
    lenght_counter_table[0x8]=160;
    lenght_counter_table[0x9]=8;
    lenght_counter_table[0xA]=60;
    lenght_counter_table[0xB]=10;
    lenght_counter_table[0xC]=14;
    lenght_counter_table[0xD]=12;
    lenght_counter_table[0xE]=26;
    lenght_counter_table[0xF]=14;

    lenght_counter_table[0x10]=12;
    lenght_counter_table[0x11]=16;
    lenght_counter_table[0x12]=24;
    lenght_counter_table[0x13]=18;
    lenght_counter_table[0x14]=48;
    lenght_counter_table[0x15]=20;
    lenght_counter_table[0x16]=96;
    lenght_counter_table[0x17]=22;
    lenght_counter_table[0x18]=192;
    lenght_counter_table[0x19]=24;
    lenght_counter_table[0x1A]=72;
    lenght_counter_table[0x1B]=26;
    lenght_counter_table[0x1C]=16;
    lenght_counter_table[0x1D]=28;
    lenght_counter_table[0x1E]=32;
    lenght_counter_table[0x1F]=30;

}

void NES_APU::start_clocking() {
    SDL_AudioSpec obt;
    SDL_OpenAudio(&au_spec,&obt);
    SDL_PauseAudio(0);

}

int16_t NES_APU::generate_sample() {
    u_int16_t SAMPLE=0;
    u_int16_t P1_SAMPLE=0;
    u_int16_t P2_SAMPLE=0;
    u_int16_t TRIANGLE_SAMPLE=0;
    u_int16_t NOISE_SAMPLE=0;


    //PULSE1
    if (status_reg.data.P1 && p1_timer_value>=8){

        P1_SAMPLE= 0b1 & p1_timer_duty[p1_reg1.data.D];

        if (p1_lenght_counter==0){
            P1_SAMPLE=0;
        }

        u_int16_t vol;
        if (p1_reg1.data.C){
            vol=p1_reg1.data.V;
        } else{
            vol=p1_envelope;
        }
        vol=p1_reg1.data.V;


        P1_SAMPLE*=vol;

    }

//PULSE2
    if (status_reg.data.P2){

        P2_SAMPLE= 0b1 & p2_timer_duty[p2_reg1.data.D];

        //P2_SAMPLE*=5000;
        if (p2_lenght_counter==0){
            P2_SAMPLE=0;
        }

        u_int16_t vol;
        if (p2_reg1.data.C){
            vol=p2_reg1.data.V;
        } else{
            vol=p2_envelope;
        }
        vol=p2_reg1.data.V;

        P2_SAMPLE*=vol;
    }
    //TRIANGLE
    if (status_reg.data.T){

        TRIANGLE_SAMPLE=triangle_sequencer_table[triangle_sequencer_index];
    }


    //NOISE
    if (status_reg.data.N && noise_lenght>0){
        if (noise_shift_reg & 0b1){
            NOISE_SAMPLE=1;
        }

        NOISE_SAMPLE*=noise_reg.data.V;
    }

    SAMPLE+=(P1_SAMPLE+P2_SAMPLE)*75+TRIANGLE_SAMPLE*85+NOISE_SAMPLE*50;
    //SAMPLE=NOISE_SAMPLE*50;

    //printf("%hd |%hhx\n",SAMPLE,status_reg.reg);
    //printf("%hx\n",NOISE_SAMPLE);
    return SAMPLE;
}

void NES_APU::exec() {

    //P1 update
    p1_timer-=1;
    if (p1_timer<=0){
        p1_timer=p1_timer_value;

        u_int8_t p1_dut_last_bit= p1_timer_duty[p1_reg1.data.D] & 0b1;
        p1_timer_duty[p1_reg1.data.D]>>=1;
        p1_timer_duty[p1_reg1.data.D] &= 0b01111111;
        if (p1_dut_last_bit){
            p1_timer_duty[p1_reg1.data.D]|=0b10000000;
        }

    }


    if (p1_lenght_counter!=0 && !p1_reg1.data.L && ppu->dot<6)
    {
        if(frame_counter_mode ){
                if (ppu->scanline==52 ||  ppu->scanline==209) {
                    p1_lenght_counter -= 1;
                }
        }
        else{
            if (ppu->scanline==65 ||ppu->scanline==196){
                p1_lenght_counter-=1;
            }
        }

    }

    if (!p1_reg1.data.C && ppu->dot<6){
        if(frame_counter_mode ){
            if (ppu->scanline==0 ||ppu->scanline==52 ||ppu->scanline==105||  ppu->scanline==209) {
                p1_envelope-=p1_reg1.data.V;
            }
        }
        else{
            if (ppu->scanline==0 ||ppu->scanline==65 || ppu->scanline==131 ||ppu->scanline==196){
                p1_envelope-=p1_reg1.data.V;
            }
        }
        if (p1_envelope<=0){
            p1_envelope=15;
        }
    }
    //P2 UPDATE

    p2_timer-=1;
    if (p2_timer<=0){
        p2_timer=p2_timer_value;

        u_int8_t p2_dut_last_bit= p2_timer_duty[p2_reg1.data.D] & 0b1;
        p2_timer_duty[p2_reg1.data.D]>>=1;
        p2_timer_duty[p2_reg1.data.D] &= 0b01111111;
        if (p2_dut_last_bit){
            p2_timer_duty[p2_reg1.data.D]|=0b10000000;
        }

    }


    if (p2_lenght_counter!=0 && !p2_reg1.data.L && ppu->dot<6)
    {
        if(frame_counter_mode ){
            if (ppu->scanline==52 ||  ppu->scanline==209) {
                p2_lenght_counter -= 1;
            }
        }
        else{
            if (ppu->scanline==65 ||ppu->scanline==196){
                p2_lenght_counter-=1;
            }
        }

    }

    if (!p2_reg1.data.C && ppu->dot<6){
        if(frame_counter_mode ){
            if (ppu->scanline==0 ||ppu->scanline==52 ||ppu->scanline==105||  ppu->scanline==209) {
                p2_envelope-=p2_reg1.data.V;
            }
        }
        else{
            if (ppu->scanline==0 ||ppu->scanline==65 || ppu->scanline==131 ||ppu->scanline==196){
                p2_envelope-=p2_reg1.data.V;
            }
        }
        if (p2_envelope<=0){
            p2_envelope=15;
        }
    }

    //TRIANGLE

    triangle_timer-=2;
    if (triangle_timer<=0){
        triangle_timer=triangle_timer_value;
        triangle_sequencer_index=(triangle_sequencer_index+1)%32;
    }

    //NOISE
    noise_timer-=1;
    if (noise_timer<0)
    {
        noise_timer=noise_timer_value;

        bool set_14= false;
        if(noise_mode_flag){
            set_14=((noise_shift_reg>>6)^noise_shift_reg) & 0b1;
        }
        else{
            set_14=((noise_shift_reg>>1)^noise_shift_reg) & 0b1;
        }

        noise_shift_reg>>=1;
        if (set_14){
            noise_shift_reg |=0b100000000000000;
        } else{
            noise_shift_reg &=0b011111111111111;
        }

    }

    if (!noise_reg.data.L && ppu->dot<6 && noise_lenght>0){
        if(frame_counter_mode ){
            if (ppu->scanline==52 ||  ppu->scanline==209) {
                noise_lenght -= 1;
            }
        }
        else{
            if (ppu->scanline==65 ||ppu->scanline==196){
                noise_lenght-=1;
            }
        }
    }

}