//
// Created by aco on 26.06.24.
//

#ifndef NESEMU_NES_CPU_H
#define NESEMU_NES_CPU_H

#include "NES_MEMORY.h"

union Status_Reg{
    u_int8_t reg;
    struct status_reg_data{
        u_int8_t C:1;
        u_int8_t Z:1;
        u_int8_t I:1;
        u_int8_t D:1;
        u_int8_t B:1;
        u_int8_t unused:1;
        u_int8_t V:1;
        u_int8_t N:1;
    }data;

};

class NES_CPU {
public:
   u_int16_t PC;
    u_int8_t SP;
   u_int16_t STACK_START;
    u_int8_t A;
    u_int8_t X,Y;

    u_int16_t adr;
    u_int8_t op_code;

    bool instruction_started;
    u_int16_t instruction_cycles;
    u_int32_t clock;
    u_int32_t i_start;

    union Status_Reg PS;

    NES_MEMORY *memory;

    bool debug;

//INITIALIZATION
    NES_CPU (NES_MEMORY *MEM,u_int16_t PC_start);
//START PROGRAM
    void RESET();
    void NMI();
//DMA
    void DMA();
//CLEAR STATUS FLAGS
    void PS_clear();
//STACK
    void STACK_PUSH(u_int8_t CH);
    u_int8_t STACK_POP();
//ADDRES MODES
   u_int16_t zero_page( u_int8_t index);
   u_int16_t immediate();
   u_int16_t absolute(u_int8_t index);
   u_int16_t indirect();
   u_int16_t indirectX();
   u_int16_t indirectY();

//INSTRUCTIONS
    void AND(u_int16_t adr);
    void ADC(u_int16_t adr);//C and V flags not set
    void ASL(u_int16_t adr,bool acc);
    void BCC(u_int16_t adr);
    void BCS(u_int16_t adr);
    void BEQ(u_int16_t adr);
    void BIT(u_int16_t adr);
    void BMI(u_int16_t adr);
    void BNE(u_int16_t adr);
    void BPL(u_int16_t adr);
    void BRK();
    void BVC(u_int16_t adr);
    void BVS(u_int16_t adr);
    void CLC();
    void CLD();
    void CLI();
    void CLV();
    void CMP(u_int16_t adr);
    void CPX(u_int16_t adr);
    void CPY(u_int16_t adr);
    void DEC(u_int16_t adr);
    void DEX();
    void DEY();
    void EOR(u_int16_t adr);
    void INC(u_int16_t adr);
    void INX();
    void INY();
    void JMP(u_int16_t adr);
    void JSR(u_int16_t adr);
    void LDA(u_int16_t adr);
    void LDX(u_int16_t adr);
    void LDY(u_int16_t adr);
    void LSR(u_int16_t adr, bool acc);
    void NOP();
    void ORA(u_int16_t adr);
    void PHA();
    void PHP();
    void PLA();
    void PLP();
    void ROL(u_int16_t adr,bool acc);
    void ROR(u_int16_t adr,bool acc);
    void RTI();
    void RTS();
    void SBC(u_int16_t adr);
    void SEC();
    void SED();
    void SEI();
    void STA(u_int16_t adr);
    void STX(u_int16_t adr);
    void STY(u_int16_t adr);
    void TAX();
    void TAY();
    void TSX();
    void TXA();
    void TXS();
    void TYA();

    //EXECUTION
    void exec();

};


#endif //NESEMU_NES_CPU_H
