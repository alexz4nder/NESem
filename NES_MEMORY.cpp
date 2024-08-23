//
// Created by aco on 26.06.24.
//

#include "NES_MEMORY.h"
#include <cstdlib>
#include "NES_CPU.h"
#include "NES_APU.h"

NES_MEMORY::NES_MEMORY() {

}

void NES_MEMORY::LoadCartridge(NES_CARTRIDGE *new_cartridge) {
    Cartridge=new_cartridge;
}

void NES_MEMORY::capture_controller_state() {
    const Uint8 *keyboard= SDL_GetKeyboardState(NULL);
    SDL_PumpEvents();

    Controller.reg=0;

    if (keyboard[SDL_SCANCODE_D]){
        Controller.reg|=0b10000000;
    }
    if (keyboard[SDL_SCANCODE_A]){
        Controller.reg|=0b01000000;
    }
    if (keyboard[SDL_SCANCODE_S]){
        Controller.reg|=0b00100000;
    }
    if (keyboard[SDL_SCANCODE_W]){
        Controller.reg|=0b00010000;
    }
    if (keyboard[SDL_SCANCODE_H]){
        Controller.reg|=0b00001000;
    }
    if (keyboard[SDL_SCANCODE_J]){
        Controller.reg|=0b00000100;
    }
    if (keyboard[SDL_SCANCODE_K]){
        Controller.reg|=0b00000010;
    }
    if (keyboard[SDL_SCANCODE_L]){
        Controller.reg|=0b00000001;
    }

}

//READ
char NES_MEMORY::read(unsigned short int adr) {
    if(adr<0x2000){
        return RAM[adr%0x800];
    }
    else if(adr<0x4000){

        u_int16_t reg=(adr-0x2000)%0x8;
        switch (reg) {
            case 0x2:
                ppu->read_latch=0;
                return ppu->PPUSTATUS.reg;
            case 0x04:
                return ppu->OAM[ppu->OAM_adr].memory;
            case 0x7:
                u_int8_t X=ppu->read_buffer;
                ppu->read_buffer = Cartridge->ppu_read(ppu->adr);
                if (ppu->PPUCTRL.data.adr_increment){
                    ppu->adr+=32;
                } else{
                    ppu->adr+=1;
                }
                return X;


        }
        return 0;

    }
    else if (adr<0x4020){
        switch (adr) {
            case 0x4016:
                u_int8_t M=0;
                if (Controller.reg&0x1){
                    M=1;
                }
                Controller.reg>>=1;
                return M;
                break;
        }

        return 0;
    }

    return Cartridge->read(adr);
}
//WRITE
void NES_MEMORY::write(char X,unsigned short adr) {
    if(adr<0x2000){
        RAM[adr%0x800]=X;
    }
    else if(adr<0x4000){
        u_int16_t reg=(adr-0x2000)%0x8;

        switch (reg) {
            case 0x0:
                ppu->PPUCTRL.reg=X;
                //printf("PPUCTRL %hhx\n",X);
                break;
            case 0x3:
                ppu->OAM_adr=X & 0b111111;
                break;
            case 0x04:
                ppu->OAM[ppu->OAM_adr].memory=X;
                ppu->OAM_adr=(ppu->OAM_adr+1) & 0b111111;
                break;
            case 0x5:
                if (ppu->read_latch){
                    ppu->scrol_y=X;
                    //printf("SCROLL-Y =%hhx\n",X);

                }
                else{
                    ppu->scrol_x=X;
                    //printf("SCROLL-X =%hhx\n",X);
                }
                ppu->read_latch=!ppu->read_latch;
                return;
                break;
            case 0x6:
                if (!ppu->read_latch){
                    ppu->adr=((X<<8)&0x3F00)|(ppu->adr&0x00FF);

                    ppu->scrol_y=(ppu->scrol_y & 0b11111000) | ((X>>4) & 0b11);
                    ppu->PPUCTRL.data.base_nametable_y=(bool)(X & 0b1000);
                    ppu->PPUCTRL.data.base_nametable_x=(bool)(X & 0b100);
                    ppu->scrol_y=(ppu->scrol_y & 0b00111111)|((X<<6) & 0b11000000);
                }
                else{
                    ppu->adr=(ppu->adr & 0xFF00)|(X & 0x00FF);

                    ppu->scrol_y=(ppu->scrol_y & 0b11000111)|((X>>2) & 0b00111000);
                    ppu->scrol_x=(ppu->scrol_x & 0x11111000)|((X<<3)& 0b11111000);

                    ppu->scrol_x_internal=ppu->scrol_x;
                    ppu->scrol_y_internal=ppu->scrol_y;
                    ppu->base_nametable_x_internal=ppu->PPUCTRL.data.base_nametable_x;
                    ppu->base_nametable_y_internal=ppu->PPUCTRL.data.base_nametable_y;

                }
                ppu->read_latch=!ppu->read_latch;
                break;
            case 0x7:
                if (ppu->adr<0x4000) {
                    Cartridge->ppu_write(X, ppu->adr);
                } else{

                }

                if (ppu->PPUCTRL.data.adr_increment){
                    ppu->adr+=32;
                } else{
                    ppu->adr+=1;
                }

                break;
        }

    }
    else if (adr<0x4020){
        switch (adr) {
            //P1
            case 0x4000:
                apu->p1_reg1.reg=X;
                //printf("0x4000 %hd\n",apu->p1_reg1.data.V);
                break;
            case 0x4001:

                break;
            case 0x4002:
                apu->p1_timer_value=(apu->p1_timer_value & 0x0700) | (((u_int16_t)X) & 0x00FF);
                break;
            case 0x4003:
                apu->p1_timer_value=(apu->p1_timer_value & 0x00FF) | ((((u_int16_t)X)<<8) & 0x0700);
                apu->p1_lenght_counter = apu->lenght_counter_table[(X >> 3) & 0x1F];

                //printf("0x4003 %hd -timer_value %hx\n",apu->p1_lenght_counter,apu->p1_timer_value);
                break;
            //P2
            case 0x4004:
                apu->p2_reg1.reg=X;
                break;
            case 0x4005:

                break;
            case 0x4006:
                apu->p2_timer_value=(apu->p2_timer_value & 0x0700) | (((u_int16_t)X) & 0x00FF);
                break;
            case 0x4007:
                apu->p2_timer_value=(apu->p2_timer_value & 0x00FF) | ((((u_int16_t)X)<<8) & 0x0700);
                apu->p2_lenght_counter = apu->lenght_counter_table[(X >> 3) & 0x1F];

                //printf("0x4003 %hd -timer_value %hx\n",apu->p1_lenght_counter,apu->p1_timer_value);
                break;
            //TRIANGLE
            case 0x4008:
                apu->triangle_C=X&0b10000000;
                apu->triangle_linear_counter_reload_value=X & 0b01111111;
                break;
            case 0x400A:
                apu->triangle_timer_value=(apu->triangle_timer_value & 0x0700) | (((u_int16_t)X) & 0x00FF);
                break;
            case 0x400B:
                apu->reload_flag=1;
                apu->triangle_timer_value=(apu->triangle_timer_value & 0x00FF) | ((((u_int16_t)X)<<8) & 0x0700);
                apu->triangle_lenght_counter=apu->lenght_counter_table[(X >> 3 ) & 0x1F];
                break;
            //NOISE
            case 0x400C:
                apu->noise_reg.reg=X;

                break;
            case 0x400D:

                break;
            case 0x400E:
                apu->noise_timer_value=apu->noise_sequencer_timer_table[X & 0xF];
                apu->noise_mode_flag= X & 0b10000000;
                break;
            case 0x400F:
                apu->noise_lenght=apu->lenght_counter_table[(X >> 3 ) & 0x1F];
                //printf("%hd\n",apu->noise_lenght);
                break;
            case 0x4015:
                apu->status_reg.reg=X;
                //printf("0x4015 %hx\n",X);
                break;
            case 0x4016:
                capture_controller_state();
                break;
            case 0x4014:
                //cpu->DMA();

                //cpu->instruction_cycles=256;
                for (int i = 0; i < 0x100; ++i) {
                    ppu->OAM[i].memory= read(X*0x100+i);
                }
                return;
                break;
            case 0x4017:
                if (X & 0b10000000){
                    apu->frame_counter_mode=1;
                } else
                {
                    apu->frame_counter_mode=0;
                }
                break;

        }
    } else{

    }
}