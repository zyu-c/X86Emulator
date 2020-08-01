#include "instruction.h"

instruction_func_t* instructions[256];

static void push_r32(Emulator *emu){
    uint8_t reg = get_code8(emu, 0) - 0x50;
    push32(emu, get_register32(emu, reg));
    emu->eip++;
    return;
}

static void push_imm8(Emulator *emu){
    uint32_t value = get_code8(emu, 1);
    push32(emu, value);
    emu->eip += 2;
    return;
}

static void push_imm32(Emulator *emu){
    uint32_t value = get_code32(emu, 1);
    push32(emu, value);
    emu->eip += 5;
    return;
}

static void pop_r32(Emulator *emu){
    uint8_t reg = get_code8(emu, 0) - 0x58;
    set_register32(emu, reg, pop32(emu));
    emu->eip++;
    return;
}

static void mov_r32_imm32(Emulator *emu){
    uint8_t reg = get_code8(emu, 0) - 0xB8;
    uint32_t value = get_code32(emu, 1);
    emu->registers[reg] = value;
    emu->eip += 5;
    return;
}

static void mov_r8_imm8(Emulator *emu){
    uint8_t reg = get_code8(emu, 0) - 0xB0;
    uint32_t value = get_code8(emu, 1);
    set_register8(emu, reg, value);
    emu->eip += 2;
    return;
}

static void mov_rm32_imm32(Emulator *emu){
    emu->eip++;
    ModRM modrm;
    parse_modrm(emu, &modrm);

    uint32_t value = get_code32(emu, 0);
    emu->eip += 4;
    set_rm32(emu, &modrm, value);
    return;
}

static void mov_rm32_r32(Emulator *emu){
    emu->eip++;
    ModRM modrm;
    parse_modrm(emu, &modrm);

    uint32_t r32 = get_r32(emu, &modrm);
    set_rm32(emu, &modrm, r32);
    return;
}

static void mov_rm8_r8(Emulator *emu){
    emu->eip++;
    ModRM modrm;
    parse_modrm(emu, &modrm);

    uint32_t r8 = get_r8(emu, &modrm);
    set_rm8(emu, &modrm, r8);
    return;
}

static void mov_r32_rm32(Emulator *emu){
    emu->eip++;
    ModRM modrm;
    parse_modrm(emu, &modrm);

    uint32_t rm32 = get_rm32(emu, &modrm);
    set_r32(emu, &modrm, rm32);
    return;
}

static void mov_r8_rm8(Emulator *emu){
    emu->eip++;
    ModRM modrm;
    parse_modrm(emu, &modrm);

    uint32_t rm8 = get_rm8(emu, &modrm);
    set_r8(emu, &modrm, rm8);
    return;
}

static void add_rm32_r32(Emulator *emu){
    emu->eip++;
    ModRM modrm;
    parse_modrm(emu, &modrm);

    uint32_t rm32 = get_rm32(emu, &modrm);
    uint32_t r32 = get_r32(emu, &modrm);
    set_rm32(emu, &modrm, rm32 + r32);
    return;
}

static void add_rm32_imm8(Emulator *emu, ModRM *modrm){
    uint32_t rm32 = get_rm32(emu, modrm);
    uint32_t imm8 = (int32_t)get_sign_code8(emu, 0);
    emu->eip++;
    set_rm32(emu, modrm, rm32 + imm8);
    return;
}

static void sub_rm32_imm8(Emulator *emu, ModRM *modrm){
    uint32_t rm32 = get_rm32(emu, modrm);
    uint32_t imm8 = (int8_t)get_sign_code8(emu, 0);
    emu->eip++;
    uint64_t result = (uint64_t)rm32 - (uint64_t)imm8;
    set_rm32(emu, modrm, rm32 - imm8);
    update_eflags_sub(emu, rm32, imm8, result);
    return;
}

static void inc_rm32(Emulator *emu, ModRM * modrm){
    uint32_t value = get_rm32(emu, modrm);
    set_rm32(emu, modrm, value + 1);
    return;
}

static void inc_r32(Emulator *emu){
    uint8_t reg = get_code8(emu, 0) - 0x40;
    uint32_t value = get_register32(emu, reg);
    set_register32(emu, reg, value + 1);
    emu->eip++;
    return;
}

static void short_jump(Emulator *emu){
    int8_t diff = get_sign_code8(emu, 1);
    emu->eip += (diff + 2);
    return;
}

static void near_jump(Emulator *emu){
    int32_t diff = get_sign_code32(emu, 1);
    emu->eip += (diff + 5);
    return;
}

static void call_rel32(Emulator *emu){
    int32_t diff = get_sign_code32(emu, 1);
    push32(emu, emu->eip + 5);
    emu->eip += (diff + 5);
    return;
}

static void ret(Emulator *emu){
    emu->eip = pop32(emu);
    return;
}

static void leave(Emulator *emu){
    uint32_t ebp = get_register32(emu, EBP);
    set_register32(emu, ESP, ebp);
    set_register32(emu, EBP, pop32(emu));
    emu->eip++;
    return;
}

static void cmp_r32_rm32(Emulator *emu){
    emu->eip++;
    ModRM modrm;
    parse_modrm(emu, &modrm);

    uint32_t r32 = get_r32(emu, &modrm);
    uint32_t rm32 = get_rm32(emu, &modrm);
    uint64_t result = (uint64_t)r32 - (uint64_t)rm32;
    update_eflags_sub(emu, r32, rm32, result);
    return;
}

static void cmp_rm32_imm8(Emulator *emu, ModRM *modrm){
    uint32_t rm32 = get_rm32(emu, modrm);
    uint32_t imm8 = (int32_t)get_sign_code8(emu, 0);
    emu->eip++;

    uint64_t result = (uint64_t)rm32 - (uint64_t)imm8;
    update_eflags_sub(emu, rm32, imm8, result);
    return;
}

static void cmp_al_imm8(Emulator *emu){
    uint32_t value = get_code8(emu, 1);
    uint32_t al = get_register8(emu, AL);
    uint64_t result = (uint64_t)al - (uint64_t)value;
    update_eflags_sub(emu, al, value, result);
    emu->eip += 2;
    return;
}

static void cmp_eax_imm32(Emulator *emu){
    uint32_t value = get_code32(emu, 1);
    uint32_t eax = get_register32(emu, EAX);
    uint64_t result = (uint64_t)eax - (uint64_t)value;
    update_eflags_sub(emu, eax, value, result);
    emu->eip += 5;
    return;
}

static void jc(Emulator *emu){
    int diff = 0;
    if(is_carry(emu)){
        diff = get_sign_code8(emu, 1);
    }
    emu->eip += (diff + 2);
    return;
}

static void js(Emulator *emu){
    int diff = 0;
    if(is_sign(emu)){
        diff = get_sign_code8(emu, 1);
    }
    emu->eip += (diff + 2);
    return;
}

static void jz(Emulator *emu){
    int diff = 0;
    if(is_zero(emu)){
        diff = get_sign_code8(emu, 1);
    }
    emu->eip += (diff + 2);
    return;
}

static void jo(Emulator *emu){
    int diff = 0;
    if(is_overflow(emu)){
        diff = get_sign_code8(emu, 1);
    }
    emu->eip += (diff + 2);
    return;
}

static void jnc(Emulator *emu){
    int diff = 0;
    if(!is_carry(emu)){
        diff = get_sign_code8(emu, 1);
    }
    emu->eip += (diff + 2);
    return;
}

static void jns(Emulator *emu){
    int diff = 0;
    if(!is_sign(emu)){
        diff = get_sign_code8(emu, 1);
    }
    emu->eip += (diff + 2);
    return;
}

static void jnz(Emulator *emu){
    int diff = 0;
    if(!is_zero(emu)){
        diff = get_sign_code8(emu, 1);
    }
    emu->eip += (diff + 2);
    return;
}

static void jno(Emulator *emu){
    int diff = 0;
    if(!is_overflow(emu)){
        diff = get_sign_code8(emu, 1);
    }
    emu->eip += (diff + 2);
    return;
}

static void jl(Emulator *emu){
    int diff = 0;
    if(is_sign(emu) != is_overflow(emu)){
        diff = get_sign_code8(emu, 1);
    }
    emu->eip += (diff + 2);
    return;
}

static void jle(Emulator *emu){
    int diff = 0;
    if(is_zero(emu) || (is_sign(emu) != is_overflow(emu))){
        diff = get_sign_code8(emu, 1);
    }
    emu->eip += (diff + 2);
    return;
}

static void in_al_dx(Emulator *emu){
    uint16_t address = get_register32(emu, EDX) & 0xffff;
    uint8_t value = io_in8(address);
    set_register8(emu, AL, value);
    emu->eip++;
    return;
}

static void out_dx_al(Emulator *emu){
    uint16_t address = get_register32(emu, EDX) & 0xffff;
    uint8_t value = get_register8(emu, AL);
    io_out8(address, value);
    emu->eip++;
    return;
}

static void swi(Emulator *emu){
    uint8_t int_index = get_code8(emu, 1);
    emu->eip += 2;

    switch(int_index){
        case 0x10:
            bios_video(emu);
            break;
        default:
            printf("実装されていません．\n");
    }
    return;
}

static void code_83(Emulator *emu){
    emu->eip++;
    ModRM modrm;
    parse_modrm(emu, &modrm);

    switch(modrm.opecode){
        case 0x00:
            add_rm32_imm8(emu, &modrm);
            break;
        case 0x05:
            sub_rm32_imm8(emu, &modrm);
            break;
        case 0x07:
            cmp_rm32_imm8(emu, &modrm);
        default:
            printf("実装されていません．\n");
            exit(1);
    }
    return;
}

static void code_ff(Emulator *emu){
    emu->eip++;
    ModRM modrm;
    parse_modrm(emu, &modrm);

    switch(modrm.opecode){
        case 0x00:
            inc_rm32(emu, &modrm);
            break;
        default:
            printf("実装されていません．\n");
            exit(1);
    }
    return;
}

void init_instructions(){
    memset(instructions, 0, sizeof(instructions));
    instructions[0x01] = add_rm32_r32;
    instructions[0x3B] = cmp_r32_rm32;
    instructions[0x3C] = cmp_al_imm8;
    instructions[0x3D] = cmp_eax_imm32;
    for(int i = 0; i < 8; i++){
        instructions[0x40 + i] = inc_r32;
    }
    for(int i = 0; i < 8; i++){
        instructions[0x50 + i] = push_r32;
    }
    for(int i = 0; i < 8; i++){
        instructions[0x58 + i] = pop_r32;
    }
    instructions[0x68] = push_imm32;
    instructions[0x6A] = push_imm8;
    instructions[0x70] = jo;
    instructions[0x71] = jno;
    instructions[0x72] = jc;
    instructions[0x73] = jnc;
    instructions[0x74] = jz;
    instructions[0x75] = jnz;
    instructions[0x78] = js;
    instructions[0x79] = jns;
    instructions[0x7C] = jl;
    instructions[0x7E] = jle;
    instructions[0x83] = code_83;
    instructions[0x88] = mov_rm8_r8;
    instructions[0x89] = mov_rm32_r32;
    instructions[0x8A] = mov_r8_rm8;
    instructions[0x8B] = mov_r32_rm32;
    for(int i = 0; i < 8; i++){
        instructions[0xB0 + i] = mov_r8_imm8;
    }
    for(int i = 0; i < 8; i++){
        instructions[0xB8 + i] = mov_r32_imm32;
    }
    instructions[0xC3] = ret;
    instructions[0xCD] = swi;
    instructions[0xC7] = mov_rm32_imm32;
    instructions[0xC9] = leave;
    instructions[0xE8] = call_rel32;
    instructions[0xE9] = near_jump;
    instructions[0xEB] = short_jump;
    instructions[0xEC] = in_al_dx;
    instructions[0xEE] = out_dx_al;
    instructions[0xFF] = code_ff;
    return;
}
