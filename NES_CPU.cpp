//
// Created by aco on 26.06.24.
//

#include "NES_CPU.h"
#include "NES_MEMORY.h"

NES_CPU::NES_CPU(NES_MEMORY *MEM, u_int16_t PC_start) {
    PC = PC_start;
    memory = MEM;
    A = 0;
    X = 0;
    Y = 0;
    SP = 0xFD;
    STACK_START = 0x100;
    PS.data.unused = 1;
    PS.data.D = 0;
}

void NES_CPU::RESET() {
    PC = 0xFFFB;
    PC = absolute(0);
    instruction_started = 0;
    clock = 0;
}

void NES_CPU::NMI() {
        STACK_PUSH((u_int8_t) (PC >> 8));
        STACK_PUSH((u_int8_t) (PC & 0xFF));

        STACK_PUSH(PS.reg & 0xEF);

        u_int16_t new_PC = memory->read(0xFFFA) & 0x00FF;
        new_PC += ((u_int16_t) (memory->read(0xFFFB))) << 8;

    if (debug){
        printf("NMI\n");
    }
    printf("NMI\n");

        PC = new_PC;
    instruction_started=0;
}

void NES_CPU::DMA() {
    instruction_cycles=255;
}

//CLEAR STATUS FLAGS
void NES_CPU::PS_clear() {

}

//STACK
void NES_CPU::STACK_PUSH(u_int8_t CH) {
    memory->write(CH, STACK_START + SP);
    SP -= 1;
}

u_int8_t NES_CPU::STACK_POP() {
    SP += 1;
    u_int8_t M = memory->read(STACK_START + SP);
    return M;
}

//ADDRESSING MODES
u_int16_t NES_CPU::zero_page(u_int8_t index) {//COMPUTE ZERO-PAGE ADR
    u_int8_t adress = memory->read(PC + 1);
    u_int16_t zero_page_adr = (adress + index) & 0x00FF;
    PC += 2;
    return zero_page_adr;
}

u_int16_t NES_CPU::immediate() {
    u_int16_t R = PC + 1;
    PC += 2;
    return R;
}

u_int16_t NES_CPU::absolute(u_int8_t index) {
    u_int16_t low= memory->read(PC + 1) & 0x00FF;
    u_int16_t high= (memory->read(PC + 2) & 0x00FF) * 0x100;

    adr= low + high + index;

    if ((adr & 0xFF00)!=(high)){
        clock+=1;
    }

    PC += 3;
    return adr;
}

u_int16_t NES_CPU::indirect() {
    u_int16_t adr = absolute(0);


    u_int16_t adr_low = memory->read(adr) & 0x00FF;
    u_int16_t adr_high = memory->read((adr & 0xFF00) + (((adr & 0x00FF) + 1) & 0x00FF)) << 8;

    adr = adr_high + adr_low;
    return adr;
}

u_int16_t NES_CPU::indirectX() {
    u_int16_t adr_zero = zero_page(X);

    u_int16_t adr_low = memory->read(adr_zero) & 0x00FF;
    u_int16_t adr_high = (memory->read((adr_zero + 1) & 0x00FF) & 0x00FF) << 8;

    u_int16_t adr = adr_high + adr_low;
    return adr;
}

u_int16_t NES_CPU::indirectY() {
    u_int16_t adr_zero = zero_page(0);

    u_int16_t adr_low = memory->read(adr_zero) & 0x00FF;
    u_int16_t adr_high = (memory->read((adr_zero + 1) & 0x00FF) & 0x00FF) << 8;



    adr = adr_low + adr_high + ((u_int16_t) Y & 0x00FF);
    if ( (adr & 0xFF00 )!=adr_high ){
        clock+=1;
    }
    return adr;
}
//INSTRUCTIONS

void NES_CPU::AND(u_int16_t adr) {
    A = A & memory->read(adr);
    //PS_clear();
    PS.data.Z = A == 0;
    PS.data.N = (A & 0b10000000) != 0;
    if (debug) {
        printf("AND %hx", adr);
    }
}

void NES_CPU::ADC(u_int16_t adr) {
    u_int8_t M = memory->read(adr);
    u_int8_t A_old = A;

    u_int16_t res = A & 0x00FF;
    u_int16_t M_long = M & 0x00FF;
    res += M_long;
    if (PS.data.C) {
        res += 1;
        A += 1;
    }

    PS.data.C = (res & 0xFF00) != 0;

    //A = A + M;
    A=res;

    PS.data.N = (A & 0b10000000) != 0;
    PS.data.Z = A == 0;

    PS.data.V = 0;

    if (PS.data.N && !(A_old & 0b10000000) && !(M & 0b10000000)) {
        PS.data.V = 1;
    }

    if (!PS.data.N && (A_old & 0b10000000) && (M & 0b10000000)) {
        PS.data.V = 1;
    }

    if (debug) {
        printf("ADC %hx", adr);
    }
}

void NES_CPU::ASL(u_int16_t adr, bool acc) {
    if (acc) {
        PS.data.C = (A & 0b10000000) != 0;
        A = A << 1;
        PS.data.Z = A == 0;
        PS.data.N = (A & 0b10000000) != 0;
        PC += 1;
        return;
    }
    u_int8_t M = memory->read(adr);
    PS.data.C = (M & 0b10000000) != 0;
    M = M << 1;
    M = M & 0b11111110;
    PS.data.Z = M == 0;
    PS.data.N = (M & 0b10000000) != 0;
    memory->write(M, adr);

    if (debug) {
        printf("ASL %hx", adr);
    }
}

void NES_CPU::BCC(u_int16_t adr) {
    if (PS.data.C == 0) {
        PC = PC + memory->read(adr);
    }

    if (debug) {
        printf("BCC %hx", adr);
    }
}

void NES_CPU::BCS(u_int16_t adr) {

    if (PS.data.C == 1) {
        PC = PC + memory->read(adr);
    }

    if (debug) {
        printf("BCS %hx", adr);
    }
}

void NES_CPU::BEQ(u_int16_t adr) {
    if (PS.data.Z == 1) {
        PC = PC + memory->read(adr);
    }

    if (debug) {
        printf("BEQ %hx", adr);
    }
}

void NES_CPU::BIT(u_int16_t adr) {
    u_int8_t M = memory->read(adr);
    //PS_clear();
    PS.data.V = (M & 0b01000000) != 0;
    PS.data.N = (M & 0b10000000) != 0;

    M = M & A;

    PS.data.Z = M == 0;

    if (debug) {
        printf("BIT %hx", adr);
    }
}

void NES_CPU::BMI(u_int16_t adr) {
    if (PS.data.N == 1) {
        PC = PC + memory->read(adr);
    }

    if (debug) {
        printf("BMI %hx", adr);
    }
}

void NES_CPU::BNE(u_int16_t adr) {
    if (PS.data.Z == 0) {
        PC = PC + memory->read(adr);
    }

    if (debug) {
        printf("BNE %hx", adr);
    }
}

void NES_CPU::BPL(u_int16_t adr) {
    if (PS.data.N == 0) {
        PC = PC + memory->read(adr);
    }

    if (debug) {
        printf("BPL %hx", adr);
    }
}

void NES_CPU::BRK() {
    STACK_PUSH((u_int8_t) (PC >> 8));
    STACK_PUSH((u_int8_t) (PC & 0xFF));


    STACK_PUSH(PS.reg|0x10);

    u_int16_t new_PC = memory->read(0xFFFF);
    new_PC += ((u_int16_t) (memory->read(0xFFFE))) << 8;

    PC = new_PC;

    if (debug) {
        printf("BRK");
    }
}

void NES_CPU::BVC(u_int16_t adr) {
    if (PS.data.V == 0) {
        PC = PC + memory->read(adr);
    }
    if (debug) {
        printf("BVC %hx", adr);
    }
}

void NES_CPU::BVS(u_int16_t adr) {
    if (PS.data.V == 1) {
        PC = PC + memory->read(adr);
    }

    if (debug) {
        printf("BVS %hx", adr);
    }
}

void NES_CPU::CLC() {
    PS.data.C = 0;
    PC += 1;

    if (debug) {
        printf("CLC");
    }
}

void NES_CPU::CLD() {
    PS.data.D = 0;
    PC += 1;
    if (debug) {
        printf("CLD");
    }
}

void NES_CPU::CLI() {
    PS.data.I = 0;
    PC += 1;

    if (debug) {
        printf("CLI");
    }
}

void NES_CPU::CLV() {
    PS.data.V = 0;
    PC += 1;

    if (debug) {
        printf("CLV");
    }
}

void NES_CPU::CMP(u_int16_t adr) {
    u_int8_t M = (u_int8_t) memory->read(adr);
    if ((u_int8_t) A < M) {
        PS.data.N = (bool)(0b10000000 & (A - M));
        PS.data.Z = 0;
        PS.data.C = 0;
    }
    if ((u_int8_t) A == M) {
        PS.data.N = 0;
        PS.data.Z = 1;
        PS.data.C = 1;
    }
    if ((u_int8_t) A > M) {
        PS.data.N = (bool)(0b10000000 & (A - M));
        PS.data.Z = 0;
        PS.data.C = 1;
    }


    if (debug) {
        printf("CMP %hx", adr);
    }
}

void NES_CPU::CPX(u_int16_t adr) {
    u_int8_t M = (u_int8_t) memory->read(adr);

    if ((u_int8_t) X < M) {
        PS.data.N = (bool)(0b10000000 & (X - M));
        PS.data.Z = 0;
        PS.data.C = 0;
    }
    if ((u_int8_t) X == M) {
        PS.data.N = 0;
        PS.data.Z = 1;
        PS.data.C = 1;
    }
    if ((u_int8_t) X > M) {
        PS.data.N = (bool)(0b10000000 & (X - M));
        PS.data.Z = 0;
        PS.data.C = 1;
    }

    if (debug) {
        printf("CPX %hx", adr);
    }
}

void NES_CPU::CPY(u_int16_t adr) {
    u_int8_t M = (u_int8_t) memory->read(adr);
    if ((u_int8_t) Y < M) {
        PS.data.N = (bool)(0b10000000 & (Y - M));
        PS.data.Z = 0;
        PS.data.C = 0;
    }
    if ((u_int8_t) Y == M) {
        PS.data.N = 0;
        PS.data.Z = 1;
        PS.data.C = 1;
    }
    if ((u_int8_t) Y > M) {
        PS.data.N = (bool)(0b10000000 & (Y - M));
        PS.data.Z = 0;
        PS.data.C = 1;
    }


    if (debug) {
        printf("CPY %hx", adr);
    }
}

void NES_CPU::DEC(u_int16_t adr) {
    u_int8_t M = memory->read(adr);
    M -= 1;
    memory->write(M, adr);
    PS.data.Z = M == 0;
    PS.data.N = (0b10000000 & M) != 0;

    if (debug) {
        printf("DEC %hx", adr);
    }
}

void NES_CPU::DEX() {
    X -= 1;
    PS.data.Z = X == 0;
    PS.data.N = (0b10000000 & X) != 0;
    PC += 1;

    if (debug) {
        printf("DEX");
    }
}

void NES_CPU::DEY() {
    Y = Y - 1;
    PS.data.Z = Y == 0;
    PS.data.N = (0b10000000 & Y) != 0;
    PC += 1;


    if (debug) {
        printf("DEY");
    }
}

void NES_CPU::EOR(u_int16_t adr) {
    u_int8_t M = memory->read(adr);
    A = A ^ M;
    PS.data.N = (0b10000000 & A) != 0;
    PS.data.Z = A == 0;

    if (debug) {
        printf("EOR %hx", adr);
    }
}

void NES_CPU::INC(u_int16_t adr) {
    u_int8_t M = memory->read(adr);
    M += 1;
    memory->write(M, adr);
    PS.data.N = (0b10000000 & M) != 0;
    PS.data.Z = M == 0;

    if (debug) {
        printf("INC %hx", adr);
    }
}

void NES_CPU::INX() {
    X += 1;
    PS.data.N = (0b10000000 & X) != 0;
    PS.data.Z = X == 0;
    PC += 1;

    if (debug) {
        printf("INX");
    }
}

void NES_CPU::INY() {
    Y += 1;
    PS.data.N = (0b10000000 & Y) != 0;
    PS.data.Z = Y == 0;
    PC += 1;

    if (debug) {
        printf("INY");
    }
}

void NES_CPU::JMP(u_int16_t adr) {
    PC = adr;

    if (debug) {
        printf("JMP %hx", adr);
    }
}

void NES_CPU::JSR(u_int16_t adr) {
    PC -= 1;
    u_int16_t old_adr = PC;
    PC = adr;
    STACK_PUSH(old_adr >> 8);
    STACK_PUSH(0x00FF & old_adr);


    if (debug) {
        printf("JSR %hx", adr);
    }
}

void NES_CPU::LDA(u_int16_t adr) {
    u_int8_t M = memory->read(adr);
    A = M;

    PS.data.Z = A == 0;
    PS.data.N = (A & 0b10000000) != 0;

    if (debug) {
        printf("LDA %hx", adr);
    }
}

void NES_CPU::LDX(u_int16_t adr) {
    u_int8_t M = memory->read(adr);
    X = M;

    PS.data.Z = X == 0;
    PS.data.N = (X & 0b10000000) != 0;

    if (debug) {
        printf("LDX %hx", adr);
    }
}

void NES_CPU::LDY(u_int16_t adr) {
    u_int8_t M = memory->read(adr);
    Y = M;

    PS.data.Z = Y == 0;
    PS.data.N = (Y & 0b10000000) != 0;

    if (debug) {
        printf("LDY %hx", adr);
    }
}

void NES_CPU::LSR(u_int16_t adr, bool acc) {
    if (acc) {
        PS.data.C = (A & 0b00000001) == 1;
        A = A >> 1;
        A = A & 0b01111111;
        PS.data.Z = A == 0;
        PS.data.N = (A & 0b10000000) != 0;
        PC += 1;

        if (debug) {
            printf("LSR A");
        }

        return;
    }

    u_int8_t M = memory->read(adr);
    PS.data.C = (M & 0b00000001) == 1;
    M = M >> 1;
    M = M & 0b01111111;
    PS.data.Z = M == 0;
    PS.data.N = (M & 0b10000000) != 0;
    memory->write(M, adr);

    if (debug) {
        printf("LSR %hx", adr);
    }
}

void NES_CPU::NOP() {
    PC += 1;

    if (debug) {
        printf("NOP");
    }
}

void NES_CPU::ORA(u_int16_t adr) {
    u_int8_t M = memory->read(adr);
    A = A | M;
    PS.data.Z = A == 0;
    PS.data.N = (A & 0b10000000) != 0;
    if (debug) {
        printf("ORA %hx", adr);
    }
}

void NES_CPU::PHA() {
    STACK_PUSH(A);
    PC += 1;

    if (debug) {
        printf("PHA");
    }
}

void NES_CPU::PHP() {
    STACK_PUSH(PS.reg | 0x30);
    PC += 1;

    if (debug) {
        printf("PHP");
    }
}

void NES_CPU::PLA() {
    A = STACK_POP();
    PC += 1;

    PS.data.Z = A == 0;
    PS.data.N = (A & 0b10000000) != 0;

    if (debug) {
        printf("PLA");
    }
}

void NES_CPU::PLP() {
    u_int8_t STATUS = STACK_POP();
    PS.data.C = (STATUS & 0b00000001) != 0;
    PS.data.Z = (STATUS & 0b00000010) != 0;
    PS.data.I = (STATUS & 0b00000100) != 0;
    PS.data.D = (STATUS & 0b00001000) != 0;
    PS.data.B = (STATUS & 0b00010000) != 0;
    PS.data.unused = (STATUS & 0b00100000) != 0;
    PS.data.V = (STATUS & 0b01000000) != 0;
    PS.data.N = (STATUS & 0b10000000) != 0;

    PC += 1;

    if (debug) {
        printf("PLP");
    }
}

void NES_CPU::ROL(u_int16_t adr, bool acc) {
    if (acc) {
        bool old_C = PS.data.C;
        PS.data.C = (A & 0b10000000) != 0;
        A = A << 1;

        if (old_C) {
            A |= 1;
        } else {
            A &= 0b11111110;
        }

        PS.data.Z = A == 0;
        PS.data.N = (A & 0b10000000) != 0;
        PC += 1;
        return;
    }
    u_int8_t M = memory->read(adr);
    bool old_C = PS.data.C;
    PS.data.C = (M & 0b10000000) != 0;
    M = M << 1;

    if (old_C) {
        M |= 0b00000001;
    } else {
        M &= 0b11111110;
    }

    PS.data.Z = M == 0;
    PS.data.N = (M & 0b10000000) != 0;

    memory->write(M, adr);

    if (debug) {
        printf("ROL %hx", adr);
    }
}

void NES_CPU::ROR(u_int16_t adr, bool acc) {
    if (acc) {
        bool old_C = PS.data.C;
        PS.data.C = (A & 0b1) != 0;
        A = A >> 1;
        if (old_C) {
            A |= 0b10000000;
        } else {
            A &= 0b01111111;
        }
        PS.data.Z = A == 0;
        PS.data.N = (A & 0b10000000) != 0;
        PC += 1;
        return;
    }
    u_int8_t M = memory->read(adr);
    bool old_C = PS.data.C;
    PS.data.C = (M & 0b00000001) != 0;
    M = M >> 1;
    if (old_C) {
        M |= 0b10000000;
    } else {
        M &= 0b01111111;
    }

    PS.data.Z = M == 0;
    PS.data.N = (M & 0b10000000) != 0;

    memory->write(M, adr);

    if (debug) {
        printf("ROR %hx", adr);
    }
}

void NES_CPU::RTI() {

    u_int8_t STATUS = STACK_POP();
    PS.reg= STATUS;

    u_int16_t adr = (u_int16_t) STACK_POP() & 0x00FF;
    adr += STACK_POP() * 0x100;
    PC = adr;


    if (debug) {
        printf("RTI");
    }
}

void NES_CPU::RTS() {
    //u_int16_t adr=STACK_POP()*0x100;
    //adr+= ((u_int16_t )STACK_POP() & 0x00FF);
    u_int16_t adr_low = ((u_int16_t) STACK_POP()) & 0x00FF;
    u_int16_t adr_high = (((u_int16_t) STACK_POP()) << 8) & 0xFF00;

    u_int16_t adr = adr_low + adr_high;

    PC = adr + 1;

    if (debug) {
        printf("RTS");
    }
}

void NES_CPU::SBC(u_int16_t adr) {

    u_int8_t M = ~memory->read(adr);
    u_int8_t A_old = A;

    u_int16_t res = A & 0x00FF;
    u_int16_t M_long = M & 0x00FF;
    res += M_long;
    if (PS.data.C) {
        res += 1;
        A += 1;
    }

    PS.data.C = (res & 0xFF00) != 0;


    //A = A + M;
    A=res;

    PS.data.N = (A & 0b10000000) != 0;
    PS.data.Z = A == 0;

    PS.data.V = 0;

    if (PS.data.N && !(A_old & 0b10000000) && !(M & 0b10000000)) {
        PS.data.V = 1;
    }

    if (!PS.data.N && (A_old & 0b10000000) && (M & 0b10000000)) {
        PS.data.V = 1;
    }


    if (debug) {
        printf("SBC %hx", adr);
    }
}

void NES_CPU::SEC() {
    PS.data.C = true;
    PC += 1;

    if (debug) {
        printf("SEC");
    }
}

void NES_CPU::SED() {
    PS.data.D = true;
    PC += 1;
    if (debug) {
        printf("SED");
    }
}

void NES_CPU::SEI() {
    PS.data.I = true;
    PC += 1;

    if (debug) {
        printf("SEI");
    }
}

void NES_CPU::STA(u_int16_t adr) {
    memory->write(A, adr);

    if (debug) {
        printf("STA %hx", adr);
    }
}

void NES_CPU::STX(u_int16_t adr) {
    memory->write(X, adr);

    if (debug) {
        printf("STX %hx", adr);
    }
}

void NES_CPU::STY(u_int16_t adr) {
    memory->write(Y, adr);

    if (debug) {
        printf("STY %hx", adr);
    }
}

void NES_CPU::TAX() {
    X = A;
    PS.data.Z=A==0;
    PS.data.N=( A & 0b10000000)!=0;
    PC += 1;
}

void NES_CPU::TAY() {
    Y = A;
    PS.data.Z=A==0;
    PS.data.N=( A & 0b10000000)!=0;
    PC += 1;
}

void NES_CPU::TSX() {
    X = SP;

    PS.data.Z = X == 0;
    PS.data.N = (X & 0b10000000) != 0;

    PC += 1;
}

void NES_CPU::TXA() {
    A = X;

    PS.data.Z = A == 0;
    PS.data.N = (A & 0b10000000) != 0;

    PC += 1;
}

void NES_CPU::TXS() {
    SP = X;
    PC += 1;
}

void NES_CPU::TYA() {
    A = Y;
    PS.data.Z=A==0;
    PS.data.N=( A & 0b10000000)!=0;
    PC += 1;
}

//EXECUTION
void NES_CPU::exec() {
    if (instruction_cycles > 2) {
        clock += 1;
        instruction_cycles -= 1;
        return;
    }

    if (!instruction_started) {
        op_code = memory->read(PC);
        clock+=1;
        if (debug) {
            printf("%hx %hhx ", PC, op_code);
        }
    }

    switch (op_code) {
        //ADC
        case 0x69:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                adr = immediate();
                instruction_started = 1;
                return;
            }
            ADC(adr);
            break;
        case 0x65:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            ADC(adr);
            break;
        case 0x75:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = zero_page(X);
                instruction_started = 1;
                return;
            }
            ADC(adr);
            break;
        case 0x6d:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            ADC(adr);
            break;
        case 0x7D:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(X);
                instruction_started = 1;
                return;
            }
            ADC(adr);
            break;
        case 0x79:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(Y);
                instruction_started = 1;
                return;
            }
            ADC(adr);
            break;
        case 0x61:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = indirectX();
                instruction_started = 1;
                return;
            }
            ADC(adr);
            break;
        case 0x71:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 5;
                adr = indirectY();
                instruction_started = 1;
                return;
            }
            ADC(adr);
            break;
            //AND
        case 0x29:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                adr = immediate();
                instruction_started = 1;
                return;
            }
            AND(adr);
            break;
        case 0x25:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            AND(adr);
            break;
        case 0x35:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = zero_page(X);
                instruction_started = 1;
                return;
            }
            AND(adr);
            break;
        case 0x2D:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            AND(adr);
            break;
        case 0x3D:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(X);
                instruction_started = 1;
                return;
            }
            AND(adr);
            break;
        case 0x39:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(Y);
                instruction_started = 1;
                return;
            }
            AND(adr);
            break;
        case 0x21:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = indirectX();
                instruction_started = 1;
                return;
            }
            AND(adr);
            break;
        case 0x31:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 5;
                adr = indirectY();
                instruction_started = 1;
                return;
            }
            AND(adr);
            break;
            //ASL
        case 0x0A:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            ASL(0, 1);
            break;
        case 0x06:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 5;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            ASL(adr, 0);
            break;
        case 0x16:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = zero_page(X);
                instruction_started = 1;
                return;
            }
            ASL(adr, 0);
            break;
        case 0x0E:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            ASL(adr, 0);
            break;
        case 0x1E:
            if (!instruction_started) {
                i_start = clock;
                adr = absolute(X);
                instruction_cycles = 7;
                instruction_started = 1;
                return;
            }
            ASL(adr, 0);
            break;
            //BCC
        case 0x90:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                adr = immediate();
                if (!PS.data.C){
                    instruction_cycles+=1;
                    if (PC/256!=(PC+memory->read(adr))/256){
                        instruction_cycles+=1;
                    }
                }

                instruction_started = 1;
                return;
            }
            BCC(adr);
            break;
            //BCS
        case 0xB0:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;

                adr = immediate();
                if (PS.data.C){
                    instruction_cycles+=1;
                    if (PC/256!=(PC+memory->read(adr))/256){
                        instruction_cycles+=1;
                    }
                }
                instruction_started = 1;
                return;
            }
            BCS(adr);
            break;
            //BEQ
        case 0xF0:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;

                adr = immediate();
                if (PS.data.Z){
                    instruction_cycles+=1;
                    if (PC/256!=(PC+memory->read(adr))/256){
                        instruction_cycles+=1;
                    }
                }
                instruction_started = 1;
                return;
            }
            BEQ(adr);
            break;
            //BIT
        case 0x24:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            BIT(adr);
            break;
        case 0x2C:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            BIT(adr);
            break;
            //BMI
        case 0x30:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;

                adr = immediate();
                if (PS.data.N){
                    instruction_cycles+=1;
                    if (PC/256!=(PC+memory->read(adr))/256){
                        instruction_cycles+=1;
                    }
                }
                instruction_started = 1;
                return;
            }
            BMI(adr);
            break;
            //BNE
        case 0xD0:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;

                adr = immediate();
                if (!PS.data.Z){
                    instruction_cycles+=1;
                    if (PC/256!=(PC+memory->read(adr))/256){
                        instruction_cycles+=1;
                    }
                }
                instruction_started = 1;
                return;
            }
            BNE(adr);
            break;
            //BPL
        case 0x10:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;

                adr = immediate();
                if (!PS.data.N){
                    instruction_cycles+=1;
                    if (PC/256!=(PC+memory->read(adr))/256){
                        instruction_cycles+=1;
                    }
                }
                instruction_started = 1;
                return;
            }
            BPL(adr);
            break;
            //BRK
        case 0x00:
            BRK();
            break;
            //BVC
        case 0x50:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;

                adr = immediate();
                if (!PS.data.V){
                    instruction_cycles+=1;
                    if (PC/256!=(PC+memory->read(adr))/256){
                        instruction_cycles+=1;
                    }
                }
                instruction_started = 1;
                return;
            }
            BVC(adr);
            break;
            //BVS
        case 0x70:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;

                adr = immediate();
                if (PS.data.V){
                    instruction_cycles+=1;
                    if (PC/256!=(PC+memory->read(adr))/256){
                        instruction_cycles+=1;
                    }
                }
                instruction_started = 1;
                return;
            }
            BVS(adr);
            break;
            //CLC
        case 0x18:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            CLC();
            break;
            //CLD
        case 0xD8:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            CLD();
            break;
            //CLI
        case 0x58:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            CLI();
            break;
            //CLV
        case 0xB8:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            CLV();
            break;
            //CMP
        case 0xC9:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                adr = immediate();
                instruction_started = 1;
                return;
            }
            CMP(adr);
            break;
        case 0xC5:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            CMP(adr);
            break;
        case 0xD5:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = zero_page(X);
                instruction_started = 1;
                return;
            }
            CMP(adr);
            break;
        case 0xCD:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            CMP(adr);
            break;
        case 0xDD:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(X);
                instruction_started = 1;
                return;
            }
            CMP(adr);
            break;
        case 0xD9:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(Y);
                instruction_started = 1;
                return;
            }
            CMP(adr);
            break;
        case 0xC1:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = indirectX();
                instruction_started = 1;
                return;
            }
            CMP(adr);
            break;
        case 0xD1:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 5;
                adr = indirectY();
                instruction_started = 1;
                return;
            }
            CMP(adr);
            break;
            //CPX
        case 0xE0:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                adr = immediate();
                instruction_started = 1;
                return;
            }
            CPX(adr);
            break;
        case 0xE4:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            CPX(adr);
            break;
        case 0xEC:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            CPX(adr);
            break;
            //CPY
        case 0xC0:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                adr = immediate();
                instruction_started = 1;
                return;
            }
            CPY(adr);
            break;
        case 0xC4:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            CPY(adr);
            break;
        case 0xCC:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            CPY(adr);
            break;
            //DEC
        case 0xC6:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 5;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            DEC(adr);
            break;
        case 0xD6:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = zero_page(X);
                instruction_started = 1;
                return;
            }
            DEC(adr);
            break;
        case 0xCE:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            DEC(adr);
            break;
        case 0xDE:
            if (!instruction_started) {
                i_start = clock;
                adr = absolute(X);
                instruction_cycles = 7;
                instruction_started = 1;
                return;
            }
            DEC(adr);
            break;
            //DEX
        case 0xCA:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            DEX();
            break;
            //DEY
        case 0x88:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            DEY();
            break;
            //EOR
        case 0x49:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                adr = immediate();
                instruction_started = 1;
                return;
            }
            EOR(adr);
            break;
        case 0x45:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            EOR(adr);
            break;
        case 0x55:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = zero_page(X);
                instruction_started = 1;
                return;
            }
            EOR(adr);
            break;
        case 0x4D:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            EOR(adr);
            break;
        case 0x5D:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(X);
                instruction_started = 1;
                return;
            }
            EOR(adr);
            break;
        case 0x59:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(Y);
                instruction_started = 1;
                return;
            }
            EOR(adr);
            break;
        case 0x41:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = indirectX();
                instruction_started = 1;
                return;
            }
            EOR(adr);
            break;
        case 0x51:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 5;
                adr = indirectY();
                instruction_started = 1;
                return;
            }
            EOR(adr);
            break;
            //INC
        case 0xE6:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 5;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            INC(adr);
            break;
        case 0xF6:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = zero_page(X);
                instruction_started = 1;
                return;
            }
            INC(adr);
            break;
        case 0xEE:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            INC(adr);
            break;
        case 0xFE:
            if (!instruction_started) {
                i_start = clock;
                adr = absolute(X);
                instruction_cycles = 7;
                instruction_started = 1;
                return;
            }
            INC(adr);
            break;
            //INX
        case 0xE8:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            INX();
            break;
            //INY
        case 0xC8:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            INY();
            break;
            //JMP
        case 0x4C:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                instruction_started = 1;
                return;
            }
            adr = absolute(0);
            JMP(adr);
            break;
        case 0x6C:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 5;
                instruction_started = 1;
                return;
            }
            adr = indirect();
            JMP(adr);
            break;
            //JSR
        case 0x20:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                instruction_started = 1;
                return;
            }
            adr = absolute(0);

            JSR(adr);
            break;
            //LDA
        case 0xA9:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                adr = immediate();
                instruction_started = 1;
                return;
            }
            LDA(adr);
            break;
        case 0xA5:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            LDA(adr);
            break;
        case 0xB5:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = zero_page(X);
                instruction_started = 1;
                return;
            }
            LDA(adr);
            break;
        case 0xAD:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            LDA(adr);
            break;
        case 0xBD:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(X);
                instruction_started = 1;
                return;
            }
            LDA(adr);
            break;
        case 0xB9:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(Y);
                instruction_started = 1;
                return;
            }
            LDA(adr);
            break;
        case 0xA1:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = indirectX();
                instruction_started = 1;
                return;
            }
            LDA(adr);
            break;
        case 0xB1:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 5;
                adr = indirectY();
                instruction_started = 1;
                return;
            }
            LDA(adr);
            break;
            //LDX
        case 0xA2:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                adr = immediate();
                instruction_started = 1;
                return;
            }
            LDX(adr);
            break;
        case 0xA6:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            LDX(adr);
            break;
        case 0xB6:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = zero_page(Y);
                instruction_started = 1;
                return;
            }
            LDX(adr);
            break;
        case 0xAE:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            LDX(adr);
            break;
        case 0xBE:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(Y);
                instruction_started = 1;
                return;
            }
            LDX(adr);
            break;
            //LDY
        case 0xA0:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                adr = immediate();
                instruction_started = 1;
                return;
            }
            LDY(adr);
            break;
        case 0xA4:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            LDY(adr);
            break;
        case 0xB4:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = zero_page(X);
                instruction_started = 1;
                return;
            }
            LDY(adr);
            break;
        case 0xAC:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            LDY(adr);
            break;
        case 0xBC:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(X);
                instruction_started = 1;
                return;
            }
            LDY(adr);
            break;
            //LSR
        case 0x4A:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            LSR(0, 1);
            break;
        case 0x46:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 5;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            LSR(adr, 0);
            break;
        case 0x56:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = zero_page(X);
                instruction_started = 1;
                return;
            }
            LSR(adr, 0);
            break;
        case 0x4E:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            LSR(adr, 0);
            break;
        case 0x5E:
            if (!instruction_started) {
                i_start = clock;
                adr = absolute(X);
                instruction_cycles = 7;
                instruction_started = 1;
                return;
            }
            LSR(adr, 0);
            break;
            //NOP
        case 0xEA:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            NOP();
            break;
            //ORA
        case 0x09:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                adr = immediate();
                instruction_started = 1;
                return;
            }
            ORA(adr);
            break;
        case 0x05:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            ORA(adr);
            break;
        case 0x15:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = zero_page(X);
                instruction_started = 1;
                return;
            }
            ORA(adr);
            break;
        case 0x0D:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            ORA(adr);
            break;
        case 0x1D:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(X);
                instruction_started = 1;
                return;
            }
            ORA(adr);
            break;
        case 0x19:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(Y);
                instruction_started = 1;
                return;
            }
            ORA(adr);
            break;
        case 0x01:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = indirectX();
                instruction_started = 1;
                return;
            }
            ORA(adr);
            break;
        case 0x11:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 5;
                adr = indirectY();
                instruction_started = 1;
                return;
            }
            ORA(adr);
            break;
            //PHA
        case 0x48:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                instruction_started = 1;
                return;
            }
            PHA();
            break;
            //PHP
        case 0x08:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                instruction_started = 1;
                return;
            }
            PHP();
            break;
            //PLA
        case 0x68:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                instruction_started = 1;
                return;
            }
            PLA();
            break;
            //PLP
        case 0x28:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                instruction_started = 1;
                return;
            }
            PLP();
            break;
            //ROL
        case 0x2A:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            ROL(0, 1);
            break;
        case 0x26:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 5;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            ROL(adr, 0);
            break;
        case 0x36:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = zero_page(X);
                instruction_started = 1;
                return;
            }
            ROL(adr, 0);
            break;
        case 0x2E:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            ROL(adr, 0);
            break;
        case 0x3E:
            if (!instruction_started) {
                i_start = clock;
                adr = absolute(X);
                instruction_cycles = 7;
                instruction_started = 1;
                return;
            }
            ROL(adr, 0);
            break;
            //ROR
        case 0x6A:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            ROR(0, 1);
            break;
        case 0x66:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 5;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            ROR(adr, 0);
            break;
        case 0x76:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = zero_page(X);
                instruction_started = 1;
                return;
            }
            ROR(adr, 0);
            break;
        case 0x6E:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            ROR(adr, 0);
            break;
        case 0x7E:
            if (!instruction_started) {
                i_start = clock;
                adr = absolute(X);
                instruction_cycles = 7;
                instruction_started = 1;
                return;
            }
            ROR(adr, 0);
            break;
            //RTI
        case 0x40:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                instruction_started = 1;
                return;
            }
            RTI();
            break;
            //RTS
        case 0x60:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                instruction_started = 1;
                return;
            }
            RTS();
            break;
            //SBC
        case 0xE9:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                adr = immediate();
                instruction_started = 1;
                return;
            }
            SBC(adr);
            break;
        case 0xE5:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            SBC(adr);
            break;
        case 0xF5:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = zero_page(X);
                instruction_started = 1;
                return;
            }
            SBC(adr);
            break;
        case 0xED:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            SBC(adr);
            break;
        case 0xFD:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(X);
                instruction_started = 1;
                return;
            }
            SBC(adr);
            break;
        case 0xF9:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(Y);
                instruction_started = 1;
                return;
            }
            SBC(adr);
            break;
        case 0xE1:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = indirectX();
                instruction_started = 1;
                return;
            }
            SBC(adr);
            break;
        case 0xF1:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 5;
                adr = indirectY();
                instruction_started = 1;
                return;
            }
            SBC(adr);
            break;
            //SEC
        case 0x38:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            SEC();
            break;
            //SED
        case 0xF8:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            SED();
            break;
            //SEI
        case 0x78:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            SEI();
            break;
            //STA
        case 0x85:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            STA(adr);
            break;
        case 0x95:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = zero_page(X);
                instruction_started = 1;
                return;
            }
            STA(adr);
            break;
        case 0x8D:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            STA(adr);
            break;
        case 0x9D:
            if (!instruction_started) {
                i_start = clock;
                adr = absolute(X);
                instruction_cycles = 5;
                instruction_started = 1;
                return;
            }
            STA(adr);
            break;
        case 0x99:
            if (!instruction_started) {
                i_start = clock;
                adr = absolute(Y);
                instruction_cycles = 5;
                instruction_started = 1;
                return;
            }
            STA(adr);
            break;
        case 0x81:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 6;
                adr = indirectX();
                instruction_started = 1;
                return;
            }
            STA(adr);
            break;
        case 0x91:
            if (!instruction_started) {
                i_start = clock;
                adr = indirectY();
                instruction_cycles = 6;
                instruction_started = 1;
                return;
            }
            STA(adr);
            break;
            //STX
        case 0x86:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            STX(adr);
            break;
        case 0x96:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = zero_page(Y);
                instruction_started = 1;
                return;
            }
            STX(adr);
            break;
        case 0x8E:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            STX(adr);
            break;
            //STY
        case 0x84:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 3;
                adr = zero_page(0);
                instruction_started = 1;
                return;
            }
            STY(adr);
            break;
        case 0x94:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = zero_page(X);
                instruction_started = 1;
                return;
            }
            STY(adr);
            break;
        case 0x8C:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 4;
                adr = absolute(0);
                instruction_started = 1;
                return;
            }
            STY(adr);
            break;
            //TAX
        case 0xAA:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            TAX();
            break;
            //TAY
        case 0xA8:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            TAY();
            break;
            //TSX
        case 0xBA:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            TSX();
            break;
            //TXA
        case 0x8A:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            TXA();
            break;
            //TXS
        case 0x9A:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            TXS();
            break;
            //TYA
        case 0x98:
            if (!instruction_started) {
                i_start = clock;
                instruction_cycles = 2;
                instruction_started = 1;
                return;
            }
            TYA();
            break;

        default:
            printf( "OP-CODE DOENT EXIST\n");
            PC += 1;
            break;
    }

    instruction_started = 0;
    clock += 1;

    if (debug) {
        printf(" SP=%hhx CLK=%d\n", SP, i_start);
    }


}


