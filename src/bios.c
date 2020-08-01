#include "bios.h"

static int bios_to_terminal[] = {
    30, 34, 32, 36, 31, 35, 33, 37
};

static void put_string(const char *s, size_t n){
    for(size_t i = 0; i < n; i++){
        io_out8(0x03f8, s[i]);
    }
    return;
}

static void bios_video_teletype(Emulator *emu){
    uint8_t color = get_register8(emu, BL) & 0x0f;
    uint8_t ch = get_register8(emu, AL);

    char buf[32];
    int terminal_color = bios_to_terminal[color & 0x07];
    int bright;
    if(color & 0x08){
        bright = 1;
    }else{
        bright = 0;
    }
    int len = sprintf(buf, "\e[%d;%dm%c\e[0m", bright, terminal_color, ch);
    put_string(buf, len);
    return;
}

void bios_video(Emulator *emu){
    uint8_t func = get_register8(emu, AH);
    switch (func){
        case 0x0e:
            bios_video_teletype(emu);
            break;
        default:
            printf("実装されていません．\n");
    }
    return;
}
