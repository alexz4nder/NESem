#include <cstdlib>
#include "NES_CPU.h"
#include "NES_CARTRIDGE.h"
#include "NES_MEMORY.h"
#include "NES_PPU.h"
#include <SDL2/SDL.h>
#include <string.h>

using namespace std;


int main(int argc,char **argv) {
    NES_MEMORY *memory=new NES_MEMORY();
    NES_CPU *cpu=new NES_CPU(memory,0x0000);

    SDL_Window *window=NULL;
    SDL_Renderer *renderer=NULL;


    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(1024,960,0,&window,&renderer);

    SDL_RenderSetLogicalSize(renderer,256,240);

    NES_CARTRIDGE* cart=new NES_CARTRIDGE(argv[1]);

    memory->LoadCartridge(cart);
    NES_PPU *ppu=new NES_PPU(renderer,window);
    ppu->refresh_rate=90;

    memory->ppu=ppu;
    memory->cpu=cpu;
    cart->ppu=ppu;
    ppu->LoadCartridge(cart);
    ppu->cpu=cpu;
    cpu->RESET();

    SDL_SetRenderDrawColor(renderer,255,255,255,255);
    SDL_RenderClear(renderer);


    while(1){
        cpu->exec();
        ppu->exec();
        ppu->exec();
        ppu->exec();
    }
    return 0;
}
