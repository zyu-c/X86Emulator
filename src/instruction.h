#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include <stdint.h>
#include "emulator.h"
#include "emulator_function.h"
#include "modrm.h"
#include "io.h"
#include "bios.h"

void init_instructions();
typedef void instruction_func_t(Emulator*);
extern instruction_func_t* instructions[256];

#endif
