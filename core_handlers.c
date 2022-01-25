#include "main.h"

const char* _extensions[] = { "XKEYBOARD", "DAMAGE", "RANDR" };
size_t _extensions_cnt = 3;

u16 handle_change_window_attributes(server_state_t* state, u8 minor, u16 len) {
	xcb_change_window_attributes_request_t* i;

	i = (xcb_change_window_attributes_request_t*) state->in;

	printf("ChangeWindowAttributes: window=%u, mask=0x%x, n=%u\n",
			i->window, i->value_mask, len - 3);
	for (int j = 0; j < (len - 3); j++) {
		printf("v=%u\n", ((u32*) state->in)[len - 3]);
	}
	return 0;
}

u16 handle_create_pixmap(server_state_t* state, u8 minor, u16 len) {
	xcb_create_pixmap_request_t* i;

	i = (xcb_create_pixmap_request_t*) state->in;
	printf("CreatePixmap: pid=%u, drawable=%u, depth=%u, s=%ux%u\n", i->pid, i->drawable, i->depth, i->width, i->height);
	return 0;
}

u16 handle_free_pixmap(server_state_t* state, u8 minor, u16 len) {
	xcb_free_pixmap_request_t* i;

	i = (xcb_free_pixmap_request_t*) state->in;
	printf("FreePixmap: pid=%u\n", i->pixmap);
	return 0;
}

u16 handle_intern_atom(server_state_t* state, u8 minor, u16 len) {
	xcb_intern_atom_request_t* i;
	xcb_intern_atom_reply_t* r;

	i = (xcb_intern_atom_request_t*) state->in;
	r = (xcb_intern_atom_reply_t*) state->out;

	char buf[128];
	memcpy(buf, &state->in[sizeof(xcb_intern_atom_request_t)], i->name_len);

	buf[i->name_len] = '\0';
	printf("InternAtom: name=\"%s\"", buf);

	r->response_type = 1;
	r->sequence = state->counter;
	r->length = 0;

	r->atom = XCB_ATOM_NONE;

	if(!strcmp(buf, "UTF8_STRING")) {
		r->atom = ATOM_UTF8_STRING;
	}

	if(r->atom == XCB_ATOM_NONE) {
		printf(", returned None\n");
	} else {
		printf(", returned %u\n", r->atom);
	}
	
	write(state->file, r, 32);
	return 0;
}

u16 handle_query_best_size(server_state_t* state, u8 minor, u16 len) {
	xcb_query_best_size_request_t* i;
	xcb_query_best_size_reply_t* r;

	i = (xcb_query_best_size_request_t*) state->in;
	r = (xcb_query_best_size_reply_t*) state->out;
	printf("QueryBestSize: drawable=%u, w=%u, h=%u\n", i->drawable, i->width, i->height);

	r->response_type = 1;
	r->sequence = state->counter;
	r->length = 0;

	switch(i->_class) {
		case XCB_QUERY_SHAPE_OF_LARGEST_CURSOR:
			// Return nearest power of 2
			if (i->width < 8) {
				r->width = 8;
				r->height = 8;
				break;
			}
			if (i->width < 16) {
				r->width = 16;
				r->height = 16;
				break;	
			}
			if (i->width < 32) {
				r->width = 32;
				r->height = 32;
				break;	
			}

			r->width = 64;
			r->height = 64;
			break;	
		default:
			break;
	}

	write(state->file, r, 32);

	return 0;
}

u16 handle_list_extensions(server_state_t* state, u8 minor, u16 len) {
	xcb_list_extensions_reply_t* r;
	r = (xcb_list_extensions_reply_t*) state->out;

	char buf[1024];
	char* pos = buf;

	printf("ListExtensions: ");

	for(int i = 0; i < _extensions_cnt; i++) {
		printf("%s ", _extensions[i]);
		int n = strlen(_extensions[i]);
		pos[0] = (uint8_t) n;
		pos++;
		memcpy(pos, _extensions[i], n);
		pos += n;
	}
	
	printf("\n");

	size_t l = (size_t) (pos - buf);
	l = (l + 4) & -4;

	r->response_type = 1;
	r->names_len = _extensions_cnt;
	r->sequence = state->counter;
	r->length = l / 4;

	write(state->file, r, 32);
	write(state->file, buf, l);
	return 0;
}

u16 handle_get_input_focus(server_state_t* state, u8 minor, u16 len) {
	xcb_get_input_focus_reply_t* r;

	r = (xcb_get_input_focus_reply_t*) state->out;
	printf("GetInputFocus\n");

	// Pretend there is no focus
	r->response_type = 1;
	r->revert_to = XCB_INPUT_FOCUS_PARENT;
	r->sequence = state->counter;
	r->length = 0;

	r->focus = XCB_INPUT_FOCUS_NONE;
	write(state->file, r, 32);
	return 0;
}

u16 handle_get_property(server_state_t* state, u8 minor, u16 len) {
	xcb_get_property_request_t* i;
	xcb_get_property_reply_t* r;

	i = (xcb_get_property_request_t*) state->in;
	r = (xcb_get_property_reply_t*) state->out;
	
	printf("GetProperty: prop=%u, get=0x%x+0x%x\n", i->property, 4 * i->long_offset, 4 * i->long_length);
	
	r->response_type = 1;
	r->sequence = state->counter;

	switch(i->property) {
//		case XCB_ATOM_RESOURCE_MANAGER:		
//			break;
		default:
			r->format = 0;
			r->length = 0;

			r->type = XCB_ATOM_NONE;
			r->bytes_after = 0;
			r->value_len = 0;
			
			break;
	}

	write(state->file, r, 32);

	return 0;
}

u16 handle_create_gc(server_state_t* state, u8 minor, u16 len) {
	xcb_create_gc_request_t* i;
	i = (xcb_create_gc_request_t*) state->in;

	printf("CreateGC: cid=%d, drawable=%u, mask=%d\n", i->cid, i->drawable, i->value_mask);
	return 0;
}

u16 handle_free_gc(server_state_t* state, u8 minor, u16 len) {
	xcb_free_gc_request_t* i;
	i = (xcb_free_gc_request_t*) state->in;

	printf("FreeGC: cid=%d\n", i->gc);
	return 0;
}

u16 handle_query_extension(server_state_t* state, u8 minor, u16 len) {
	char buf[128];
	xcb_query_extension_request_t* i;
	xcb_query_extension_reply_t* r;

	i = (xcb_query_extension_request_t*) state->in;
	r = (xcb_query_extension_reply_t*) state->out;

	memcpy(buf, &state->in[sizeof(xcb_query_extension_request_t)], i->name_len);
	buf[i->name_len] = '\0';
	
	printf("QueryExtension: \"%s\"", buf);
		
	r->response_type = 1;
	r->sequence = (uint16_t) state->counter;
	r->length = 0;
	
	r->present = 0;
	r->major_opcode = 0;
	r->first_event = 0;
	r->first_error = 0;

	if(!strcmp(buf, "XKEYBOARD")) {
		r->present = 1;
		r->major_opcode = _globals.xkb_opcode;
	}
	/*
	if(!strcmp(buf, "DAMAGE")) {
		r->present = 1;
		r->major_opcode = _globals.damage_opcode;
	}
	if(!strcmp(buf, "RANDR")) {
		r->present = 1;
		r->major_opcode = _globals.randr_opcode;
	}
	*/

	if(!r->present) {
		printf(", not present\n");
	} else {
		printf(", present\n");
	}

	write(state->file, r, 32);
	return 0;
}

void register_core_handlers() {
	// The first allocated opcode is 128 (see _cur_opcode in main.c),
	// and they go up from there.
	register_handler(handle_change_window_attributes, XCB_CHANGE_WINDOW_ATTRIBUTES);
	
	register_handler(handle_query_extension, XCB_QUERY_EXTENSION);
	register_handler(handle_create_pixmap, XCB_CREATE_PIXMAP);
	register_handler(handle_free_pixmap, XCB_FREE_PIXMAP);
	register_handler(handle_create_gc, XCB_CREATE_GC);
	register_handler(handle_free_gc, XCB_FREE_GC);
	register_handler(handle_get_property, XCB_GET_PROPERTY);
	register_handler(handle_get_input_focus, XCB_GET_INPUT_FOCUS);
	register_handler(handle_list_extensions, XCB_LIST_EXTENSIONS);
	register_handler(handle_query_best_size, XCB_QUERY_BEST_SIZE);
	register_handler(handle_intern_atom, XCB_INTERN_ATOM);
}

