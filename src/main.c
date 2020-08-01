#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "emulator.h"
#include "emulator_function.h"
#include "modrm.h"
#include "instruction.h"

#define MEMORY_SIZE 1024*1024

int main(int argc, char *argv[]){
    Emulator *emu;

    int quiet = 0;
    int i = 1;
    while(i < argc){
        if(!strcmp(argv[i], "-q")){
            quiet = 1;
            argc = opt_remove_at(argc, argv, i);
        }else{
            i++;
        }
    }

    if(argc != 2){
        printf("バイナリファイルを指定してください．\n");
        return 1;
    }

    emu = create_emu(MEMORY_SIZE, 0x7c00, 0x7c00);

    read_binary(emu, argv[1]);

    init_instructions();

    if(!quiet){
        printf("EIP       , FUNC\n");
    }

    while(emu->eip < MEMORY_SIZE){
        uint8_t code = get_code8(emu, 0);

        if(!quiet){
            printf("0x%08x, %02X\n", emu->eip, code);
        }

        if(instructions[code] == NULL){
            printf("この関数は実装されていません．:%x\n", code);
            exit(0);
        }

        instructions[code](emu);

        if(emu->eip == 0x00){
            break;
        }
    }

    printf("\n\nfinished.\n\n");

    dump_registers(emu);
    destroy_emu(emu);
    return 0;
}
