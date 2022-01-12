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

/*
template<typename T> struct Writer {
	FILE* _file;

	public:
	Writer(FILE* file) : _file(file) {}
	
	ssize_t write(T* v) {
		return fwrite((void*) v, sizeof(T), 1, _file);
	}
};
*/

typedef u16 (*request_handler_t) (int, const u8*, u8*, u8, u16);

extern u8 alloc_opcode();
extern u32 _counter;

extern u8 _xkb_opcode;
extern u8 _damage_opcode;
extern u8 _randr_opcode;

u16 default_handler(int file, const u8* in, u8* out, u8 opcode, u16 len);

void register_handler(request_handler_t handler, uint8_t opcode);

extern void register_core_handlers();
extern void register_font_handlers();
extern void register_damage_handlers();
extern void register_xkb_handlers();
extern void register_randr_handlers();

#endif // _MAIN_H
