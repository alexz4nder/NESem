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

char NES_CARTRIDGE_RAM::read(unsigned short adr) {

    return mem[adr];
}

char NES_CARTRIDGE_RAM::write(u_int8_t X, u_int16_t adr) {
    mem[adr]=X;
}

NES_CARTRIDGE::NES_CARTRIDGE(char *file_path) {
    FILE *f;
    f = fopen(file_path,"rb");
    header[16];
    fread(header,16,1,f);

    for (int i = 0; i < 16; ++i) {
        printf("%2hhx ",header[i]);
    }
    printf("\n");
    mapper=(header[7] & 0xF0)+(header[6]>>4);

    iNES2= false;
    if ((header[7] & 0b00001100)==0b1000){
        iNES2= true;
        printf("INES2.0\n");
    } else
    {
        printf("INES1.0\n");
    }

    PRG_SIZE=header[4];
    CHR_SIZE=header[5];


    mirroring_changable= true;


    switch (mapper) {
        case 0: {
            NROM *nrom = new NROM;
            mapper_data = (void *) nrom;
            nrom->nrom_ram=new NES_CARTRIDGE_RAM();

            mirroring = (header[6] & 0b00000001) != 0;
            for (int i = 0; i < PRG_SIZE; ++i) {
                memory[i] = new NES_CARTRIDGE_MEMORY;
                fread(memory[i]->mem, 16 * 1024, 1, f);
            }

            for (int i = 0; i < CHR_SIZE; ++i) {
                CHR[i] = new NES_CARTRIDGE_CHR_MEMORY;
                fread(CHR[i]->mem, 8 * 1024, 1, f);
            }
            if (CHR_SIZE==0){
                CHR[0]=new NES_CARTRIDGE_CHR_MEMORY;
            }
        }
            break;

        case 1:
        {
            mirroring=(header[6] & 0b00000001)!=0;
            if (mirroring){
                mirroring=2;
                mirroring_changable= false;
            }
            else{
                mirroring=3;
            }

            mapper_data = malloc(sizeof(mmc1));
            Mmc1 *mmc1 = (Mmc1 *) mapper_data;

            mmc1->chr0_reg=0;
            mmc1->chr1_reg=0;
            mmc1->control_reg=0;
            mmc1->prg_reg=0;

            mmc1->shift_reg=0;
            mmc1->shift_counter=0;

            mmc1->mmc1_ram=new NES_CARTRIDGE_RAM;

            for (int i = 0; i < PRG_SIZE; ++i) {
                memory[i] = new NES_CARTRIDGE_MEMORY;
                fread(memory[i]->mem, 16 * 1024, 1, f);
            }
            if (CHR_SIZE==0){
                CHR[0] = new NES_CARTRIDGE_CHR_MEMORY;
            }
            for (int i = 0; i < CHR_SIZE; ++i) {
                CHR[i] = new NES_CARTRIDGE_CHR_MEMORY;
                fread(CHR[i]->mem, 8 * 1024, 1, f);
            }
        }
            break;
        default:
            printf("MAPPER %hhx NOT IMPLEMENTED\n",mapper);
            exit(0);
            break;
    }


}

char NES_CARTRIDGE::read(u_int16_t adr) {

    char page=0;
    char res;
    switch (mapper) {
        case 0:
        {
            if (adr<0x8000){
                NROM *nrom=(NROM *)mapper_data;
                return nrom->nrom_ram->mem[adr-0x6000];
            }

            if (adr >= 0x8000 && PRG_SIZE == 2) {
                if (adr >= 0xC000) {
                    page = 1;
                }
            }
            adr = (adr - 0x8000) % (16 * 1024);
            res = memory[page]->read(adr);
        }
            break;
        case 1:{
            Mmc1 *mmc1=(Mmc1*)mapper_data;
            page=mmc1->prg_reg;
            if (adr<0x6000){
                return 0;
            }
            if (adr<0x8000){
                return mmc1->mmc1_ram->mem[adr-0x6000];
            }

            switch ((mmc1->control_reg >> 2)& 0b11) {
                case 0:
                case 1:
                    if (adr>=0xC000){
                        return memory[((page & 0b1110) +1)%PRG_SIZE]->mem[adr%0x4000];
                    }
                    else{
                        return memory[(page & 0b1110)%PRG_SIZE]->mem[adr%0x4000];
                    }
                    break;
                case 2:
                    if (adr<0xC000){
                        return memory[0]->mem[adr%0x4000];
                    }
                    else{
                        return memory[page%PRG_SIZE]->mem[adr%0x4000];
                    }
                    break;
                case 3:
                    if (adr<0xC000){
                        return memory[page%PRG_SIZE]->mem[adr%0x4000];
                    }
                    else{
                        return memory[PRG_SIZE-1]->mem[adr%0x4000];
                    }
                    break;
            }


        }
            break;
        default:
            printf("MAPPER NOT IMPLEMENTED - READ\n");
            break;
    }

    return res;
}

void NES_CARTRIDGE::write(u_int8_t X, u_int16_t adr) {

    switch (mapper) {
        case 0:{

            if (adr<0x8000){
                NROM *nrom=(NROM *)mapper_data;
                nrom->nrom_ram->mem[adr-0x6000]=X;
            }

        }
            break;
        case 1:{
         Mmc1 *mmc1=(Mmc1*)mapper_data;

            if (adr<0x6000){
                return;
            }
            if (adr<0x8000 && adr>=0x6000){
                if (mmc1->prg_reg & 0b10000){
                    return;
                }
                mmc1->mmc1_ram->mem[adr-0x6000]=X;
                return;
            }

            if (X & 0b10000000){
                mmc1->control_reg |= 0b1100;
                mirroring=0;
                mmc1->shift_counter=0;
                mmc1->shift_reg=0;
                return;
            }

            mmc1->shift_reg>>=1;
            if (X & 0b1){
                mmc1->shift_reg|=0b10000;
            }
            else{
                mmc1->shift_reg&=0b1111;
            }

            mmc1->shift_reg&=0b11111;
            mmc1->shift_counter+=1;
            //printf("%hhx shift:%hhx\n",X%2,mmc1->shift_reg);

            if (mmc1->shift_counter==5){
                if (adr>=0xE000){
                    mmc1->prg_reg=mmc1->shift_reg;
                    //printf("mmc1 prg %2hhx\n",mmc1->prg_reg);
                }
                else if(adr>=0xC000){
                    mmc1->chr1_reg=mmc1->shift_reg;
                    //printf("mmc1 chr1 %hhx\n");
                }
                else if(adr>=0xA000){
                    mmc1->chr0_reg=mmc1->shift_reg;
                    //printf("mmc1 chr0 %hhx\n");
                }
                else{
                    mmc1->control_reg=mmc1->shift_reg;
                    if (mirroring_changable | (!iNES2)){
                        mirroring= mmc1->control_reg & 0b11;
                    }
                    //printf("mmc1 ctrl %2hhx\n",mmc1->control_reg);
                    //printf("%hhx\n",mirroring);

                }


                mmc1->shift_counter=0;
            }

        }
            break;
    }



}

char NES_CARTRIDGE::ppu_read(u_int16_t adr) {
    char page;
    char offset;

    switch (mapper) {
        case 0: {
            if (adr < 0x2000) {

                return CHR[0]->mem[adr];
            } else if (adr < 0x3000) {
                adr -= 0x2000;

                if (mirroring == 1) { //VERTICAL-MIRROR
                    if (adr >= 0x800) {
                        adr -= 0x800;
                    }
                    return ppu->VRAM[adr];

                } else if (mirroring == 0) {//HORIZONTAL-MIRROR

                    if (adr<0x800){
                        adr=adr%0x400;
                    }
                    else{
                        adr=adr%0x400 + 0x400;
                    }

                    /*if ((adr >= 0x400 && adr < 0x800) || (adr >= 0xC00 && adr <= 0x1000)) {
                        adr -= 0x400;
                    }*/

                    return ppu->VRAM[adr];
                }

            } else if (adr >= 0x3F00) {
                if ((adr - 0x3F00) % 4 == 0) {
                    return ppu->pallets[0];
                }

                return ppu->pallets[(adr - 0x3F00) % 0x20];
            }
            return 0;
        }
            break;
        case 1:{
            Mmc1 *mmc1=(Mmc1*)mapper_data;
            if (adr < 0x2000) {
                if(CHR_SIZE==0){


                return CHR[0]->mem[adr];
                }

                if (!(mmc1->control_reg & 0b10000)){
                    /*adr=adr%0x1000;
                    if (mmc1->chr0_reg%2==1){
                        adr+=0x1000;
                    }*/

                    return CHR[mmc1->chr0_reg/2]->mem[adr];
                }
                else{
                    if (adr<0x1000){
                        if (mmc1->chr0_reg%2==1){
                            adr+=0x1000;
                        }
                        return CHR[mmc1->chr0_reg/2]->mem[adr];
                    }
                    else{
                        if (mmc1->chr1_reg%2==0){
                            adr-=0x1000;
                        }
                        return CHR[mmc1->chr1_reg/2]->mem[adr];
                    }
                }

                return CHR[0]->mem[adr];
            }
            else if (adr < 0x3000) {
                adr -= 0x2000;
                if (mirroring ==0){
                    return ppu->VRAM[adr%0x400];
                }
                else if(mirroring==1){
                    return ppu->VRAM[adr%0x400+0x400];
                }
                else if (mirroring == 2) { //VERTICAL-MIRROR
                    if (adr >= 0x800) {
                        adr -= 0x800;
                    }
                    return ppu->VRAM[adr];

                } else if (mirroring == 3) {//HORIZONTAL-MIRROR
                    if (adr<0x800){
                        adr=adr % 0x400;
                    }
                    else{
                        adr=adr % 0x400 + 0x400;
                    }

                    /*if ((adr >= 0x400 && adr < 0x800) || (adr >= 0xC00 && adr <= 0x1000)) {
                        adr -= 0x400;
                    }*/

                    return ppu->VRAM[adr];
                }

            }
            else if (adr >= 0x3F00) {
                if ((adr - 0x3F00) % 4 == 0) {
                    return ppu->pallets[0];
                }

                return ppu->pallets[(adr - 0x3F00) % 0x20];
            }
            return 0;
        }
            break;
        default:
            printf("MAPPER NOT IMPLEMENTED - PPU_READ\n");
            break;
    }
    return 0;
}

void NES_CARTRIDGE::ppu_write(u_int8_t X, u_int16_t adr) {

    char page;
    char offset;

    switch (mapper) {
        case 0: {
            if (adr < 0x2000) {

                CHR[0]->mem[adr]=X;
                return;
            } else if (adr < 0x3000) {
                adr -= 0x2000;

                if (mirroring == 1) {//VERTICAL-MIRROR
                    if (adr >= 0x800) {
                        adr -= 0x800;
                    }

                    ppu->VRAM[adr] = X;
                    return;

                } else if (mirroring == 0) {
                    if (adr<0x800){
                        adr=adr%0x400;
                    }
                    else{
                        adr=adr%0x400 + 0x400;
                    }
                    /*if ((adr >= 0x400 && adr < 0x800) || (adr >= 0xC00 && adr <= 0x1000)) {
                        adr -= 0x400;
                    }*/
                    ppu->VRAM[adr] = X;
                    return;
                }

            } else if (adr >= 0x3F00) {
                if ((adr - 0x3F00) == 0 || (adr - 0x3F00) == 0x10) {
                    ppu->pallets[0] = X;
                    return;
                } else {
                    ppu->pallets[(adr - 0x3F00) % 0x20] = X;
                    return;
                }
            }
        }
            break;
        case 1:{
            Mmc1 *mmc1 = (Mmc1*)mapper_data;
            if (adr < 0x2000) {
                if (CHR_SIZE==0){
                    CHR[0]->mem[adr]=X;
                }
                return;
            } else if (adr < 0x3000) {
                adr -= 0x2000;


                if (mirroring ==0){
                    ppu->VRAM[adr%0x400]=X;
                }
                else if(mirroring==1){
                    ppu->VRAM[adr%0x400+0x400]=X;
                }
                else if (mirroring == 2) {//VERTICAL-MIRROR
                    if (adr >= 0x800) {
                        adr -= 0x800;
                    }

                    ppu->VRAM[adr] = X;
                    return;

                } else if (mirroring == 3) {
                    if (adr<0x800){
                        adr=adr%0x400;
                    }
                    else{
                        adr=adr%0x400 + 0x400;
                    }
                    /*if ((adr >= 0x400 && adr < 0x800) || (adr >= 0xC00 && adr <= 0x1000)) {
                        adr -= 0x400;
                    }*/
                    ppu->VRAM[adr] = X;
                    return;
                }

            } else if (adr >= 0x3F00) {
                if ((adr - 0x3F00) == 0 || (adr - 0x3F00) == 0x10) {
                    ppu->pallets[0] = X;
                    return;
                } else {
                    ppu->pallets[(adr - 0x3F00) % 0x20] = X;
                    return;
                }
            }
        }
            break;
        default:
            printf("MAPPER NOT IMPLEMENTED - PPU WRITE\n");
            break;
    }




}
