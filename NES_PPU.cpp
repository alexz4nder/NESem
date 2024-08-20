//
// Created by aco on 09.07.24.
//

#include "NES_PPU.h"
#include <SDL2/SDL.h>
#include "NES_CPU.h"
#include <algorithm>
NES_PPU::NES_PPU(SDL_Renderer* sdl_renderer,SDL_Window *sdl_window) {
    renderer=sdl_renderer;
    window=sdl_window;

    scrol_x_internal=0;
    scrol_y_internal=0;

    scanline=0;
    dot=1;

    color[0x00]={.r=98,.g=98,.b=98,.a=255};
    color[0x01]={.r=0,.g=31,.b=178,.a=255};
    color[0x02]={.r=36,.g=4,.b=200,.a=255};
    color[0x03]={.r=82,.g=0,.b=178,.a=255};
    color[0x04]={.r=115,.g=0,.b=118,.a=255};
    color[0x05]={.r=128,.g=0,.b=36,.a=255};
    color[0x06]={.r=115,.g=11,.b=0,.a=255};
    color[0x07]={.r=82,.g=40,.b=0,.a=255};
    color[0x08]={.r=36,.g=68,.b=0,.a=255};
    color[0x09]={.r=0,.g=87,.b=0,.a=255};
    color[0x0A]={.r=0,.g=92,.b=0,.a=255};
    color[0x0B]={.r=0,.g=83,.b=36,.a=255};
    color[0x0C]={.r=0,.g=60,.b=118,.a=255};
    color[0x0D]={.r=0,.g=0,.b=0,.a=255};
    color[0x0E]={.r=0,.g=0,.b=0,.a=255};
    color[0x0F]={.r=0,.g=0,.b=0,.a=255};

    color[0x10]={.r=171,.g=171,.b=171,.a=255};
    color[0x11]={.r=13,.g=87,.b=255,.a=255};
    color[0x12]={.r=75,.g=48,.b=255,.a=255};
    color[0x13]={.r=138,.g=19,.b=255,.a=255};
    color[0x14]={.r=188,.g=8,.b=214,.a=255};
    color[0x15]={.r=210,.g=18,.b=105,.a=255};
    color[0x16]={.r=199,.g=46,.b=0,.a=255};
    color[0x17]={.r=157,.g=84,.b=0,.a=255};
    color[0x18]={.r=96,.g=123,.b=0,.a=255};
    color[0x19]={.r=32,.g=152,.b=0,.a=255};
    color[0x1A]={.r=0,.g=163,.b=0,.a=255};
    color[0x1B]={.r=0,.g=153,.b=66,.a=255};
    color[0x1C]={.r=0,.g=125,.b=180,.a=255};
    color[0x1D]={.r=0,.g=0,.b=0,.a=255};
    color[0x1E]={.r=0,.g=0,.b=0,.a=255};
    color[0x1F]={.r=0,.g=0,.b=0,.a=255};


    color[0x20]={.r=255,.g=255,.b=255,.a=255};
    color[0x21]={.r=83,.g=174,.b=255,.a=255};
    color[0x22]={.r=144,.g=133,.b=255,.a=255};
    color[0x23]={.r=211,.g=101,.b=118,.a=255};
    color[0x24]={.r=255,.g=87,.b=255,.a=255};
    color[0x25]={.r=255,.g=93,.b=207,.a=255};
    color[0x26]={.r=255,.g=119,.b=87,.a=255};
    color[0x27]={.r=250,.g=158,.b=0,.a=255};
    color[0x28]={.r=189,.g=199,.b=0,.a=255};
    color[0x29]={.r=122,.g=231,.b=0,.a=255};
    color[0x2A]={.r=67,.g=246,.b=17,.a=255};
    color[0x2B]={.r=38,.g=239,.b=126,.a=255};
    color[0x2C]={.r=44,.g=213,.b=246,.a=255};
    color[0x2D]={.r=78,.g=78,.b=78,.a=255};
    color[0x2E]={.r=0,.g=0,.b=0,.a=255};
    color[0x2F]={.r=0,.g=0,.b=0,.a=255};


    color[0x30]={.r=255,.g=255,.b=255,.a=255};
    color[0x31]={.r=182,.g=225,.b=255,.a=255};
    color[0x32]={.r=206,.g=209,.b=255,.a=255};
    color[0x33]={.r=233,.g=195,.b=255,.a=255};
    color[0x34]={.r=255,.g=188,.b=255,.a=255};
    color[0x35]={.r=255,.g=189,.b=244,.a=255};
    color[0x36]={.r=255,.g=198,.b=195,.a=255};
    color[0x37]={.r=255,.g=213,.b=154,.a=255};
    color[0x38]={.r=233,.g=230,.b=129,.a=255};
    color[0x39]={.r=206,.g=244,.b=129,.a=255};
    color[0x3A]={.r=182,.g=251,.b=154,.a=255};
    color[0x3B]={.r=169,.g=250,.b=195,.a=255};
    color[0x3C]={.r=169,.g=240,.b=244,.a=255};
    color[0x3D]={.r=184,.g=184,.b=184,.a=255};
    color[0x3E]={.r=0,.g=0,.b=0,.a=255};
    color[0x3F]={.r=0,.g=0,.b=0,.a=255};



}

void NES_PPU::LoadCartridge(NES_CARTRIDGE *cart) {
    cartridge=cart;
}

u_int8_t NES_PPU::read(u_int16_t adr) {
    if(adr<0x3F00){
        return cartridge->ppu_read(adr);
    } else{
        return pallets[adr%0x20];
    }
}

void NES_PPU::exec() {
    while (safe_to_render){

    }
    dot=(dot+1)%341;
    if(dot==0){
    base_nametable_x_internal=PPUCTRL.data.base_nametable_x;
    scrol_x_internal=scrol_x;

//OAM RAM
        if (scanline<240) {
            OAM_num = 0;
            for (int i = 0; (i < 64) && (OAM_num != 8); i += 1) {
                if (scanline >= OAM[i * 4].memory && scanline < OAM[i * 4].memory + 8) {
                    OAM_que[OAM_num] = i;
                    OAM_num += 1;
                }
            }
        }

        //auto compare_oam=[&](u_int8_t a,u_int8_t b){return OAM[4*a+3].memory<OAM[4*b+3].memory;};
        //sort(OAM_que,OAM_que+OAM_num-1,compare_oam);

        scanline=(scanline+1)%262;
        if (scanline==241){
            PPUSTATUS.data.vblank=1;
            if (PPUCTRL.data.vblank_nmi){
                //printf("NMI %hx\n",cpu->PC);
                cpu->NMI();
            }
            /*if (1){
                SDL_SetRenderDrawColor(renderer,255,0,0,255);
                for (int i = 0; i < 16; ++i) {
                    SDL_RenderDrawLine(renderer,i*16,0,i*16,240);
                }
                for (int i = 0; i < 16; ++i) {
                    SDL_RenderDrawLine(renderer,0,i*16,256,i*16);
                }
            }*/
            safe_to_render= true;
        }
        if (scanline==0){
            PPUSTATUS.data.vblank=0;

            scrol_x_internal=scrol_x;
            scrol_y_internal=scrol_y;

            base_nametable_x_internal=PPUCTRL.data.base_nametable_x;
            base_nametable_y_internal=PPUCTRL.data.base_nametable_y;

        }
        if (scanline==261){
            PPUSTATUS.data.S=0;
        }
    }
//RENDERING PIXEL
    if (scanline>=0 && scanline<240 && dot<256){
        u_int16_t x_cord=dot+(scrol_x_internal & 0x00FF);
        if (base_nametable_x_internal){
            x_cord+=256;
        }
        u_int16_t y_cord=scanline+(scrol_y_internal & 0x00FF);
        if (base_nametable_y_internal){
            y_cord+=240;
        }

        u_int16_t x_block = x_cord/8;
        u_int16_t y_block= y_cord/8;

        u_int8_t page=0;
        if (x_block>=32){
            page+=1;
        }
        if (y_block>=30){
            page+=2;
        }
        if (x_block>=64){
            page-=1;
        }
        if (y_block>=60){
            page-=2;
        }

        u_int16_t block_adr=0x2000+page*0x400+(x_block%32)+(y_block%30)*32;

        u_int8_t pattern_id=cartridge->ppu_read(block_adr);



        u_int16_t sprite_adress=0;
        if (PPUCTRL.data.background_pattern_table){
            sprite_adress=0x1000;
        }
        sprite_adress=sprite_adress+pattern_id*16;

        u_int8_t row_high= read(sprite_adress+ (scrol_y_internal+scanline)%8);
        u_int8_t row_low= read(sprite_adress+ (scrol_y_internal+scanline)%8 + 8);

        //row_high= 0xFF;
        //row_low= 0x00;


        u_int16_t attribute_adress=0x2000+0x3C0+page*0x400+(x_block%32)/4+((y_block%30)/4)*8;
        u_int8_t background_attribute= read(attribute_adress);

        if ((y_block/2) % 2){
            background_attribute>>=4;
        }
        if ((x_block/2) % 2){
            background_attribute>>=2;
        }

        background_attribute &=0x3;

        row_high>>=7-(scrol_x_internal+dot) %8;
        row_low>>=7-(scrol_x_internal+dot) %8;

        u_int8_t pixel_value =(0x1 & row_low)*2+(0x1 & row_high);



        //OAM

        u_int8_t sprite_pixel = 0;
        u_int8_t sprite_pixel_priority=64;
        OAM_memory data;


        for (int i = 0; i < OAM_num; ++i) {
            if (dot>=OAM[OAM_que[i]*4+3].memory && dot<OAM[OAM_que[i]*4+3].memory+8){

                OAM_memory sprite_data=OAM[OAM_que[i]*4+2];
                u_int8_t sprite_id=OAM[OAM_que[i]*4+1].memory;

                sprite_adress=0;
                if (PPUCTRL.data.sprite_pattern_table){
                    sprite_adress=0x1000;
                }

                sprite_adress=sprite_adress+ sprite_id* 16;
                u_int8_t sprite_row=scanline-((u_int16_t)(OAM[OAM_que[i]*4].memory) & 0x00FF)-1;

                if (sprite_data.data.flip_vertical){
                    sprite_row=7-sprite_row;
                }

                u_int8_t row_high= read(sprite_adress+ sprite_row);
                u_int8_t row_low= read(sprite_adress+sprite_row+ 8);




                if (sprite_data.data.flip_horizontal)
                {
                    row_high >>=  (dot - OAM[OAM_que[i] * 4 + 3].memory) % 8;
                    row_low >>=  (dot - OAM[OAM_que[i] * 4 + 3].memory) % 8;
                }
                else {
                    row_high >>= 7 - (dot - OAM[OAM_que[i] * 4 + 3].memory) % 8;
                    row_low >>= 7 - (dot - OAM[OAM_que[i] * 4 + 3].memory) % 8;
                }


                u_int8_t sprite_pixel_value =(0x1 & row_low)*2+(0x1 & row_high);

                if (sprite_pixel_value!=0){
                    if (sprite_pixel==0){
                        sprite_pixel=sprite_pixel_value;
                        sprite_pixel_priority=OAM_que[i];
                        data.memory=sprite_data.memory;
                    }
                    else{
                        if (OAM_que[i]<sprite_pixel_priority){
                            sprite_pixel=sprite_pixel_value;
                            sprite_pixel_priority=OAM_que[i];
                            data.memory=sprite_data.memory;
                        }
                    }
                }


            }
        }


        if (sprite_pixel_priority==0 && !PPUSTATUS.data.S) {
            if (sprite_pixel != 0 && pixel_value != 0) {
                PPUSTATUS.data.S = 1;
                //printf("SPRITE ZERO HIT %hhx % hhx\n",dot,scanline);
            }
        }


        if (sprite_pixel!=0 && OAM_num!=0 && (!data.data.priority ||(pixel_value==0)   )){
            pixel_value=sprite_pixel;
            u_int8_t sprite_color=pallets[sprite_pixel+16+4*data.data.pallet];
            SDL_Color clr=color[sprite_color];

            SDL_SetRenderDrawColor(renderer,clr.r,clr.g,clr.b,255);
            SDL_RenderDrawPoint(renderer,dot,scanline);
            return;
        }

        u_int8_t background_color=pallets[pixel_value+background_attribute*4];
        SDL_Color clr=color[background_color];
        if (pixel_value==0){
            clr=color[pallets[0]];
        }
        SDL_SetRenderDrawColor(renderer,clr.r,clr.g,clr.b,255);
        SDL_RenderDrawPoint(renderer,dot,scanline);
        return;

        switch (pixel_value) {
            case 0:
                SDL_SetRenderDrawColor(renderer,0,0,0,255);
                SDL_RenderDrawPoint(renderer,dot,scanline);
                break;
            case 1:
                SDL_SetRenderDrawColor(renderer,84,84,84,255);
                SDL_RenderDrawPoint(renderer,dot,scanline);
                break;
            case 2:
                SDL_SetRenderDrawColor(renderer,176,176,176,255);
                SDL_RenderDrawPoint(renderer,dot,scanline);
                break;
            case 3:
                SDL_SetRenderDrawColor(renderer,255,255,255,255);
                SDL_RenderDrawPoint(renderer,dot,scanline);
                break;
        }


    }





}