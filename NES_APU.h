//
// Created by aco on 18.08.24.
//

#include <cstdlib>

#ifndef NESEMU_NES_APU_H
#define NESEMU_NES_APU_H


class NES_APU {
public:

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

};


#endif //NESEMU_NES_APU_H
