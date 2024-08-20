//
// Created by aco on 18.08.24.
//

#include "NES_APU.h"
#include "NES_CPU.h"
#include "NES_PPU.h"
#include <SDL2/SDL.h>

void apu_callback(void *usr_dat,Uint8 *stream,int len){
    NES_APU *apu=(NES_APU*)usr_dat;


    for (int i = 0; i < 20; ++i) {
        apu->cpu->exec();
        apu->ppu->exec();
        apu->ppu->exec();
        apu->ppu->exec();
        apu->cpu->exec();
        apu->ppu->exec();
        apu->ppu->exec();
        apu->ppu->exec();
    }

    int16_t *str=(int16_t *)stream;
    len/=2;
    str[0]= 0;

}

NES_APU::NES_APU(NES_PPU *nes_ppu, NES_CPU *nes_cpu) {
    au_spec.userdata=(void *)this;
    au_spec.freq=44100;
    au_spec.samples=1;
    au_spec.format=AUDIO_S16;
    au_spec.channels=1;

    ppu=nes_ppu;
    cpu=nes_cpu;

    au_spec.callback=apu_callback;

}

void NES_APU::start_clocking() {
    SDL_AudioSpec obt;
    SDL_OpenAudio(&au_spec,&obt);
    SDL_PauseAudio(0);

}