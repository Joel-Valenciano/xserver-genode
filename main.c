#include "main.h"
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

// This is a best effort attempt at a replacement X11 server

request_handler_t _op_handlers[256];
uint32_t _counter;
u8 _cur_opcode;

u8 _xkb_opcode;
u8 _damage_opcode;
u8 _randr_opcode;

u8 alloc_opcode() {
	u8 op = _cur_opcode;
	_cur_opcode++;
	return op;
}

u16 default_handler(int file, const u8* in, u8* out, u8 minor, u16 len) {
	printf("Unhandled Request: op=%u, minor=%u, l=%u\n", in[0], minor, len);
	return 1;
}

void register_handler(request_handler_t handler, uint8_t opcode) {
	_op_handlers[opcode] = handler;
}
void register_damage_handlers() {}  
//void register_xkb_handlers() {}
void register_randr_handlers() {}

void register_all_handlers() {
    for(int i = 0; i < 256; i++) { 
        register_handler(default_handler, (u8) i);
    }

    register_core_handlers();
	register_font_handlers();
    register_damage_handlers();
    register_xkb_handlers();
    register_randr_handlers();
}

u16 read_request(int file, u8* in, u8* out) {
	size_t r;
	
	r = read(file, in, 4);
	
	if(r == -1) {
		printf("Socket closed by remote end\n");
		return 1;
	}

	if(r == 0) {
		usleep(500*1000);
		return 0;
	}	
	
	u8 opcode = in[0];
	u8 minor_opcode = in[1];
	u16 length = *((u16*) &in[2]);

	read(file, in + 4, (length - 1) * 4);
	_op_handlers[opcode](file, in, out, minor_opcode, length);	
	
	return 0;
}

void write_reply(int file, xcb_setup_t* reply) {
	write(file, reply, sizeof(xcb_setup_t));
}

void write_string(int file, char* string, size_t size) {
	write(file, string, size);
}

void write_format(int file, xcb_format_t* format) {
	write(file, format, sizeof(xcb_format_t));
}

void write_depth(int file, xcb_depth_t* depth, xcb_visualtype_t* visual) {
	write(file, depth, sizeof(xcb_depth_t));
	write(file, visual, sizeof(xcb_visualtype_t));
}

void write_screen(int file, xcb_screen_t* screen) {
	write(file, screen, sizeof(xcb_screen_t));
}

xcb_visualid_t visual_id_from(uint32_t id) {
	return XID_MASK & (id |= VISUALID_MASK);
}

void write_accepted(int file) {
	char vendor[] = "Genode";
	size_t s = strlen(vendor) + 1;
	size_t vendor_size = (s + 4) & -4;
	
	char* vendor_buf = malloc(vendor_size);
	memset(vendor_buf, 0, vendor_size);
	strncpy(vendor_buf, vendor, s);

	unsigned num_format = 1,
		 num_screen = 1,
		 num_depth = 1,
		 num_visual = 1;

	size_t depth_size = 	num_depth*sizeof(xcb_depth_t) +
				num_visual*sizeof(xcb_visualtype_t); 
	size_t screen_size =	num_screen*sizeof(xcb_screen_t) + depth_size;
	size_t format_size =	num_format*sizeof(xcb_format_t);
	size_t extra_size =	screen_size + format_size + vendor_size;

	size_t reply_size =	(sizeof(xcb_setup_t) - 8) + extra_size;
	size_t length =		reply_size/4;

	//printf("setup=%lu,vendor=%lu,screen=%lu,format=%lu,extra=%lu,length=%lu\n", sizeof(xcb_setup_t), vendor_size, screen_size, format_size, extra_size, length);
	//printf("reply_size = %lu, sending length=%lu.\n", reply_size, length);
	//printf("setup_size = %lu, extra_size = %lu, vendor_size = %lu, sending length=%lu.\n", sizeof(xcb_setup_t), extra_size, vendor_size, length);
	//size_t length = 8 + 2*num_screens + ((s + pad + v) / 4

	xcb_format_t formats[] = {
		{ 24, 32, 8 }
		//{ 32, 32, 0 },
		//{ 16, 16, 0 }
	};

	xcb_depth_t depths[] = {
		{ .depth=24, .visuals_len=1 }
	};

	xcb_visualtype_t visuals[] = {
		{
			.visual_id = visual_id_from(0),
			._class = XCB_VISUAL_CLASS_DIRECT_COLOR,
			.bits_per_rgb_value = 24,
			.colormap_entries = UINT16_MAX,
			.red_mask = 0xFF000000,
			.green_mask = 0x00FF0000,
			.blue_mask = 0x0000FF00
		}
	};

	xcb_screen_t screen;

	screen.root = 0;
	screen.white_pixel = 0xFFFFFFFF;
	screen.black_pixel = 0x0;
	screen.current_input_masks = 0;
	screen.width_in_pixels = 640;
	screen.height_in_pixels = 480;
	screen.width_in_millimeters = 200;
	screen.height_in_millimeters = 150;
	screen.min_installed_maps = 0;
	screen.max_installed_maps = 1024;
	screen.root_visual = visual_id_from(0);
	screen.backing_stores = 1;
	screen.save_unders = 0;
	screen.root_depth = 24;
	screen.allowed_depths_len = 1;
	
	// XCB is written for client-side use, so this is the server's reply.
	xcb_setup_t reply;

	reply.status = X_CONN_SUCCESS;
	reply.protocol_major_version = 11;
	reply.protocol_minor_version = 0;
	reply.length = length;
	reply.release_number = 6;
	reply.resource_id_base = XID_BASE;
	reply.resource_id_mask = XID_MASK;
	reply.motion_buffer_size = 64;
	reply.vendor_len = s;
	reply.maximum_request_length = REQUEST_SIZE;
	reply.roots_len = 1;
	reply.pixmap_formats_len = 1;
	reply.image_byte_order = BITMAP_LSByte;
	reply.bitmap_format_bit_order = BITMAP_MSBit;
	reply.bitmap_format_scanline_unit = 1;
	reply.bitmap_format_scanline_pad = 32;
	reply.min_keycode = 8;
	reply.max_keycode = 255;

	write_reply(file, &reply);
	write_string(file, vendor_buf, vendor_size);
	write_format(file, &formats[0]);
	write_screen(file, &screen);
	write_depth(file, &depths[0], &visuals[0]);

	//for(int i = 0; i < 4; i++)
		//write_depth(file, &depths[i], &visuals[0]);
}

u8 init_connection(u8* in, int file) {
	xcb_setup_request_t* i;
	int n, d, n_real, d_real, pos;

	i = (xcb_setup_request_t*) in;

	read(file, i, sizeof(xcb_setup_request_t));
	
	n = i->authorization_protocol_name_len;
	d = i->authorization_protocol_data_len;

	n_real = (n + 4) & -4;
	d_real = (d + 4) & -4;
	
	pos = sizeof(xcb_setup_request_t);

	if(n > 0) {
		read(file, &in[pos], n_real);
		pos += n_real;
	}
	
	if(d > 0) {
		read(file, &in[pos], d_real);
	}
	
	printf("Connection Request: %c, v%d-%d\n",
			i->byte_order,
			i->protocol_major_version,
			i->protocol_minor_version);

	write_accepted(file);
	return 0;
}

int open_socket(int* listen_fd, int* accept_fd, struct sockaddr_in* saddr, struct sockaddr_in* caddr, socklen_t* caddr_size) {	
	int err;

	*listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int reuseaddr = 1;
	setsockopt(*listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int));

	printf("Waiting...\n");
	err = bind(*listen_fd, (struct sockaddr*) saddr, sizeof(struct sockaddr_in));
	if(err != 0) return 1;

	err = listen(*listen_fd, 10);
	*accept_fd = accept(*listen_fd, (struct sockaddr*) caddr, caddr_size);
	return 0;
}

int send_input(int file, int* focus_in) {
	char buf[REPLY_SIZE];
	
	if (*focus_in) {
		*focus_in = 0;
		
		xcb_focus_in_event_t* e;
		e = (xcb_focus_in_event_t*) buf;
		
		e->response_type = XCB_FOCUS_IN;
		e->detail = XCB_NOTIFY_DETAIL_POINTER;
		e->sequence = _counter;
		e->event = 0;
		e->mode = XCB_NOTIFY_MODE_NORMAL;
	}
	else {
		*focus_in = 1;
		
		xcb_focus_out_event_t* e;
		e = (xcb_focus_in_event_t*) buf;
		
		e->response_type = XCB_FOCUS_OUT;
		e->detail = XCB_NOTIFY_DETAIL_NONE;
		e->sequence = _counter;
		e->event = 0;
		e->mode = XCB_NOTIFY_MODE_NORMAL;
	}

	write(file, buf, 32);
	return 0;
}

int main() {
	u8* init_request;
	u8* in;
	u8* out;
	int listen_fd;
	int accept_fd;
	int done;
	socklen_t caddr_size;
	struct sockaddr_in saddr;
	struct sockaddr_in caddr;

	register_all_handlers();

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(6005);
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	caddr_size = 0;

	if(open_socket(&listen_fd, &accept_fd, &saddr, &caddr, &caddr_size)) {
		printf("Couldn't listen() on socket, errno=%d\n", errno);
		exit(1);
	}
	
	init_request = malloc(REQUEST_SIZE);	
	in = malloc(REQUEST_SIZE);
	out = malloc(REPLY_SIZE);
	
	init_connection(init_request, accept_fd);
	free(init_request);

	done = 0;
	_counter = 1;
	_cur_opcode = 128;
	
	int _focus = 0;

	while(!done) {
		if(read_request(accept_fd, in, out) != 0) {
			done = 1;
		}
		send_input(accept_fd, &_focus);
		
		_counter++;
	}

	//sleep(10);
	close(listen_fd);
	close(accept_fd);
	return 0;
}
