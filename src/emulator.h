#ifndef EMULATOR_H_
#define EMULATOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

enum Register{
    EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, REGISTERS_COUNT,
    AL = EAX, CL = ECX, DL = EDX, BL = EBX,
    AH = AL + 4, CH = CL + 4, DH = DL + 4, BH = BL + 4
};

extern char *registers_name[];

typedef struct{
    uint32_t registers[REGISTERS_COUNT];
    uint32_t eflags;
    uint8_t *memory;
    uint32_t eip;
} Emulator;

Emulator *create_emu(size_t size, uint32_t eip, uint32_t esp);
void destroy_emu(Emulator* emu);
void dump_registers(Emulator *emu);
void read_binary(Emulator *emu, const char *filename);
int opt_remove_at(int argc, char *argv[], int index);

#endif
