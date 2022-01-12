#include "main.h"

u16 handle_create_glyph_cursor(int file, const u8* in, u8* out, u8 minor, u16 len) {
    xcb_create_glyph_cursor_request_t* i;
    i = (xcb_create_glyph_cursor_request_t*) in;
    printf("CreateGlyphCursor: cid=%u, src=%u, dest=%u, src_char=%u, mask_char=%u, fg=%2x%2x%2x, bg=%2x%2x%2x\n",
            i->cid, i->source_font, i->mask_font, 
            i->source_char, i->mask_char,
            i->fore_red, i->fore_green, i->fore_blue,
            i->back_red, i->back_green, i->back_blue);
    return 0;
}

u16 handle_open_font(int file, const u8* in, u8* out, u8 minor, u16 len) {
	xcb_open_font_request_t* i;

	i = (xcb_open_font_request_t*) in;
	char buf[100];
	
	int pos = sizeof(xcb_open_font_request_t);
	memcpy(buf, &in[pos], i->name_len);
	
	buf[i->name_len] = '\0';
	printf("OpenFont: id=%u, name=\"%s\"\n", i->fid, buf);
	
	return 0;
}

u16 handle_query_font(int file, const u8* in, u8* out, u8 minor, u16 len) {
	xcb_query_font_request_t* i;
	xcb_query_font_reply_t* r;

	i = (xcb_query_font_request_t*) in;
	r = (xcb_query_font_reply_t*) out;
	printf("QueryFont: id=%u\n", i->font);
	
	return 0;
}

void register_font_handlers() {
	register_handler(handle_create_glyph_cursor, XCB_CREATE_GLYPH_CURSOR);
	
	register_handler(handle_open_font, XCB_OPEN_FONT);
	register_handler(handle_query_font, XCB_QUERY_FONT);
}
