#include "emulator.h"

char *registers_name[] = {
    "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"
};

Emulator *create_emu(size_t size, uint32_t eip, uint32_t esp){
    Emulator *emu = malloc(sizeof(Emulator));
    emu->memory = malloc(size);

    memset(emu->registers, 0, sizeof(emu->registers));

    emu->eip = eip;
    emu->registers[ESP] = esp;

    return emu;
}

void destroy_emu(Emulator* emu){
    free(emu->memory);
    free(emu);
    return;
}

void dump_registers(Emulator *emu){
    for(int i = 0; i < REGISTERS_COUNT; i++){
        printf("%s : 0x%08x\n", registers_name[i], emu->registers[i]);
    }
    printf("EIP : 0x%08x\n", emu->eip);
    return;
}

void read_binary(Emulator *emu, const char *filename){
    FILE *binary;

    binary = fopen(filename, "rb");
    if(binary == NULL){
        printf("バイナリファイルが開けません．\n");
        exit(1);
    }

    fread(emu->memory + 0x7c00, 1, 0x200, binary);
    fclose(binary);
    return;
}

int opt_remove_at(int argc, char *argv[], int index){
    if(index < 0 || argc <= index){
        return argc;
    }else{
        for(int i = index; i < argc - 1; i++){
            argv[i] = argv[i + 1];
        }
        argv[argc] = NULL;
        return argc - 1;
    }
}
