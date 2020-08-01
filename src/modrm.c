#include "modrm.h"

void parse_modrm(Emulator *emu, ModRM *modrm){
    uint8_t code;
    memset(modrm, 0, sizeof(ModRM));

    code = get_code8(emu, 0);
    modrm->mod = ((code & 0b11000000) >> 6);
    modrm->opecode = ((code & 0b00111000) >> 3);
    modrm->rm = code & 0b00000111;
    emu->eip++;

    if(modrm->mod != 0b11 && modrm->rm == 0b100){
        modrm->sib = get_code8(emu, 0);
        emu->eip++;
    }

    if((modrm->mod == 0b00 && modrm->rm == 0b101) || modrm->mod == 0b10){
        modrm->disp32 = get_code32(emu, 0);
        emu += 4;
    }else if(modrm->mod == 0b01){
        modrm->disp8 = get_code8(emu, 0);
        emu->eip++;
    }
    return;
}

uint32_t calc_memory_address(Emulator *emu, ModRM *modrm){
    if(modrm->mod == 0b00){
        if(modrm->rm ==0b100){
            printf("実装されていません．\n");
            exit(0);
        }else if(modrm->rm == 0b101){
            return modrm->disp32;
        }else{
            return get_register32(emu, modrm->rm);
        }
    }else if(modrm->mod == 0b01){
        if(modrm->rm == 0b100){
            printf("実装されていません．\n");
            exit(0);
        }else{
            return get_register32(emu, modrm->rm) + modrm->disp8;
        }
    }else if(modrm->mod == 0b10){
        if(modrm->rm == 0b100){
            printf("実装されていません．\n");
            exit(0);
        }else{
            return get_register32(emu, modrm->rm) + modrm->disp32;
        }
    }else{
        printf("実装されていません．\n");
        exit(0);
    }
}

uint32_t get_r32(Emulator *emu, ModRM *modrm){
    return get_register32(emu, modrm->reg_index);
}

uint8_t get_r8(Emulator *emu, ModRM *modrm){
    return get_register8(emu, modrm->reg_index);
}

uint32_t get_rm32(Emulator *emu, ModRM *modrm){
    if(modrm->mod == 0b11){
        return get_register32(emu, modrm->rm);
    }else{
        uint32_t address = calc_memory_address(emu, modrm);
        return get_memory32(emu, address);
    }
}

uint8_t get_rm8(Emulator *emu, ModRM *modrm){
    if(modrm->mod == 0b11){
        return get_register8(emu, modrm->rm);
    }else{
        uint32_t address = calc_memory_address(emu, modrm);
        return get_memory8(emu, address);
    }
}

void set_r32(Emulator *emu, ModRM *modrm, uint32_t value){
    set_register32(emu, modrm->reg_index, value);
    return;
}

void set_r8(Emulator *emu, ModRM *modrm, uint8_t value){
    set_register8(emu, modrm->reg_index, value);
    return;
}

void set_rm32(Emulator *emu, ModRM *modrm, uint32_t value){
    if(modrm->mod == 0b11){
        set_register32(emu, modrm->rm, value);
    }else{
        uint32_t address = calc_memory_address(emu, modrm);
        set_memory32(emu, address, value);
    }
    return;
}

void set_rm8(Emulator *emu, ModRM *modrm, uint8_t value){
    if(modrm->mod == 0b11){
        set_register8(emu, modrm->rm, value);
    }else{
        uint32_t address = calc_memory_address(emu, modrm);
        set_memory8(emu, address, value);
    }
    return;
}
