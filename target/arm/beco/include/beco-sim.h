//
// Copyright 2020 BES Technic
//
// Description: ARM coprocessor instructions bindings for verilator sim
//

#ifndef _BECO_SIM_H
#define _BECO_SIM_H

#include <stdint.h>

// From simulation lib.

void     CDP(uint8_t cpnum,    uint8_t opc1, uint8_t opc2, uint32_t cpd, uint8_t cpm, uint8_t cpn);
void     MCR(uint8_t cpnum,    uint8_t opc1, uint8_t opc2, uint32_t rt,  uint8_t cpm, uint8_t cpn);
uint32_t MRC(uint8_t cpnum,    uint8_t opc1, uint8_t opc2, uint8_t cpm,  uint8_t cpn);
uint64_t MRRC_R(uint8_t cpnum, uint8_t opc1, uint8_t cpm);
void     MCRR(uint8_t cpnum,   uint8_t opc1, uint32_t rtl, uint32_t rth, uint8_t cpm);

inline void CDP2(uint8_t cpnum, uint8_t opc1, uint8_t opc2, uint32_t cpd, uint8_t cpm, uint8_t cpn) {
    CDP(cpnum, (opc1 | 0x10), opc2, cpd, cpm, cpn);
}
inline void MCR2(uint8_t cpnum, uint8_t opc1, uint8_t opc2, uint32_t cpd, uint8_t cpm, uint8_t cpn) {
    MCR(cpnum, (opc1 | 0x08), opc2, cpd, cpm, cpn);
}
inline uint32_t MRC2(uint8_t cpnum, uint8_t opc1, uint8_t opc2, uint8_t cpm, uint8_t cpn) {
    return MRC(cpnum, (opc1 | 0x08), opc2, cpm, cpn);
}

#define BECO_BARRIER()

// OS / SIMULATOR HOOK

int beco_init(int argc, char** argv, char** env);
int beco_exit(int ret);

#define BECO_INIT()    beco_init(argc, argv, env)
#define BECO_EXIT(ret) beco_exit(ret)


#endif

