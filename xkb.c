#include "main.h"

u16 handle_xkb(server_state_t* state, u8 minor, u16 len) {
    switch(minor) {
        case XCB_XKB_USE_EXTENSION: {
            // Respond with whatever version is requested
            xcb_xkb_use_extension_request_t* i;
            xcb_xkb_use_extension_reply_t* r;

            i = (xcb_xkb_use_extension_request_t*) state->in;
            r = (xcb_xkb_use_extension_reply_t*) state->out;

            printf("Xkb version %u.%u\n", i->wantedMajor, i->wantedMinor);

            r->response_type = 1;
            r->supported = 1;
            r->sequence = state->counter;
            r->length = 0;

            r->serverMajor = i->wantedMajor;
            r->serverMinor = i->wantedMinor;

            write(state->file, r, 32);
            break;
                                    }
        default: {}
    }
    return 0;
}

void register_xkb_handlers() {
	_globals.xkb_opcode = alloc_opcode();

	register_handler(handle_xkb, _globals.xkb_opcode);
}
