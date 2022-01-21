#ifndef _MAIN_H
#define _MAIN_H

#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <xcb/xproto.h>
#include <xcb/xkb.h>

#define ATOM_UTF8_STRING 130

#define XID_MASK 0xFFFFFF
#define XID_BASE 0
#define VISUALID_MASK 0x01000000

#define REQUEST_SIZE 4*4096
#define REPLY_SIZE 32

#define X_CONN_FAILED 0
#define X_CONN_SUCCESS 1
#define X_CONN_AUTH 2

#define BITMAP_LSBit 0
#define BITMAP_MSBit 1

#define BITMAP_LSByte 0
#define BITMAP_MSByte 1

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

typedef struct server_state {
    u8* in;
    u8* out;

	int file;
    int done;

	u32 counter;
} server_state_t;

typedef u16 (*request_handler_t) (server_state_t*, u8, u16);

typedef struct global_state {
    u8 cur_opcode;

    u8 xkb_opcode;
    u8 damage_opcode;
    u8 randr_opcode;
	
	request_handler_t op_handlers[256];
} global_state_t;

extern global_state_t _globals;

extern u8 alloc_opcode();

u16 default_handler(server_state_t* state, u8 opcode, u16 len);

void register_handler(request_handler_t handler, uint8_t opcode);

extern void register_core_handlers();
extern void register_font_handlers();
extern void register_damage_handlers();
extern void register_xkb_handlers();
extern void register_randr_handlers();

#endif // _MAIN_H
