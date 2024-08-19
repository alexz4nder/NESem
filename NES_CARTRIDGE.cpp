//
// Created by aco on 02.07.24.
//

#include "NES_CARTRIDGE.h"
#include "NES_PPU.h"
#include <fstream>
#include <stdlib.h>

char NES_CARTRIDGE_MEMORY::read(unsigned short adr) {

    return mem[adr];
}

char NES_CARTRIDGE_CHR_MEMORY::read(u_int16_t adr) {

    return mem[adr];
}

NES_CARTRIDGE::NES_CARTRIDGE(char *file_path) {
    FILE *f;
    //ifstream f;
    f = fopen(file_path,"rb");
    header[16];
    fread(header,16,1,f);
    for (int i = 0; i < 16; ++i) {
        //printf("%2hhx ",header[i]);
    }
    printf("\n");
    mapper=(header[7] & 0xF0)+(header[6]>>4);

    PRG_SIZE=header[4];
    CHR_SIZE=header[5];

    mirroring=(header[6] & 0b00000001)!=0;

    switch (mapper) {
        case 0:
            for (int i = 0; i < PRG_SIZE; ++i) {
                memory[i]=new NES_CARTRIDGE_MEMORY;
                fread(memory[i]->mem,16*1024,1,f);
            }

            for (int i = 0; i < CHR_SIZE; ++i) {
                CHR[i]=new NES_CARTRIDGE_CHR_MEMORY;
                fread(CHR[i]->mem,8*1024,1,f);
            }

            break;

        default:
            printf("MAPPER NOT IMPLEMENTED\n");
            break;
    }


}

char NES_CARTRIDGE::read(u_int16_t adr) {

    char page=0;
    char res;
    switch (mapper) {
        case 0:
            if (adr>=0x8000 && PRG_SIZE==2)
            {
                if (adr>=0xC000){
                    page=1;
                }
            }
            adr=(adr-0x8000)%(16*1024);
            res=memory[page]->read(adr);
            break;

        default:
            printf("MAPPER NOT IMPLEMENTED\n");
            break;
    }

    return res;
}

char NES_CARTRIDGE::ppu_read(u_int16_t adr) {
    char page;
    char offset;

    switch (mapper) {
        case 0:

            if(adr<0x2000){
                if (CHR_SIZE==0){
                    return 0;
                }
                return CHR[0]->mem[adr];
            }
            else if (adr<0x3000){
                adr-=0x2000;

                if (mirroring){ //VERTICAL-MIRROR
                    if (adr>=0x800){
                        adr-=0x800;
                    }
                    return ppu->VRAM[adr];

                }
                else if (!mirroring){//HORIZONTAL-MIRROR
                    if ((adr>=0x400 && adr<0x800)||(adr>=0xC00 && adr<=0x1000)){
                        adr-=0x400;
                    }

                    return ppu->VRAM[adr];
                }

            }
            else if(adr>=0x3F00){
                if ((adr-0x3F00)%4==0){
                    return ppu->pallets[0];
                }

                return ppu->pallets[(adr-0x3F00)%0x20];
            }
            return 0;
            break;
        default:
            printf("MAPPER NOT IMPLEMENTED\n");
            break;
    }

}

void NES_CARTRIDGE::ppu_write(u_int8_t X, u_int16_t adr) {

    char page;
    char offset;

    switch (mapper) {
        case 0:

            if(adr<0x2000){
                if (CHR_SIZE==0){
                    return;
                }
                CHR[0]->mem[adr]=X;
                return;
            }
            else if (adr<0x3000){
                adr-=0x2000;

                if (mirroring){//VERTICAL-MIRROR
                    if (adr>=0x800){
                        adr-=0x800;
                    }

                    ppu->VRAM[adr]=X;
                    return;

                }
                else if (!mirroring){
                    if ((adr>=0x400 && adr<0x800)||(adr>=0xC00 && adr<=0x1000)){
                        adr-=0x400;
                    }
                    ppu->VRAM[adr]=X;
                    return;
                }

            }
            else if (adr>=0x3F00){
                if ((adr-0x3F00)==0 || (adr-0x3F00)==0x10){
                    ppu->pallets[0]=X;
                    return;
                }
                else{
                    ppu->pallets[(adr-0x3F00)%0x20]=X;
                    return;
                }
            }
            break;
        default:
            printf("MAPPER NOT IMPLEMENTED\n");
            break;
    }




}
