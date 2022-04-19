/*
 * dressupgeekout's port of Uxn/Varvara to the Atari ST
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gem.h>

#include "uxn.h"
#include "devices/system.h"
#include "devices/file.h"
#include "devices/datetime.h"

static Uint8 nil_dei(Device *d, Uint8 port);
static void nil_deo(Device *d, Uint8 port);
static void console_deo(Device *d, Uint8 port);

/*
 * You need to provide this yourself.
 * XXX why?
 */
void
system_deo_special(Device *d, Uint8 port)
{
	(void)d;
	(void)port;
}


/*
 * A noop-ish DEI I guess
 */
static Uint8
nil_dei(Device *d, Uint8 port)
{
	return d->dat[port];
}


/*
 * A noop-ish DEO I guess
 */
static void
nil_deo(Device *d, Uint8 port)
{
	(void)d;
	(void)port;
}


/*
 * There is no src/devices/console.[ch] so we just do it here
 */
static void
console_deo(Device *d, Uint8 port)
{
	FILE *fd = NULL;

	switch (port) {
	case 0x08:
		fd = stdout;
		break;
	case 0x09:
		fd = stderr;
		break;
	default:
		; /* OK */
	}

	if (fd) {
		fputc(d->dat[port], fd);
		fflush(fd);
	}
}


/*
 * You need to provide this yourself.
 * XXX why?
 */
int
uxn_interrupt(void)
{
	return 1;
}


static int
uxn_start(Uxn *u)
{
	if (!uxn_boot(u, (Uint8 *)calloc(0x10000, sizeof(Uint8)))) {
		return error("Boot", "Failed");
	}

	/* system   */ uxn_port(u, 0x0, system_dei, system_deo);
	/* console  */ uxn_port(u, 0x1, nil_dei, console_deo);
	/* empty    */ uxn_port(u, 0x2, nil_dei, nil_deo);
	/* empty    */ uxn_port(u, 0x3, nil_dei, nil_deo);
	/* empty    */ uxn_port(u, 0x4, nil_dei, nil_deo);
	/* empty    */ uxn_port(u, 0x5, nil_dei, nil_deo);
	/* empty    */ uxn_port(u, 0x6, nil_dei, nil_deo);
	/* empty    */ uxn_port(u, 0x7, nil_dei, nil_deo);
	/* empty    */ uxn_port(u, 0x8, nil_dei, nil_deo);
	/* empty    */ uxn_port(u, 0x9, nil_dei, nil_deo);
	/* file0    */ uxn_port(u, 0xa, file_dei, file_deo);
	/* file1    */ uxn_port(u, 0xb, file_dei, file_deo);
	/* datetime */ uxn_port(u, 0xc, datetime_dei, nil_deo);
	/* empty    */ uxn_port(u, 0xd, nil_dei, nil_deo);
	/* empty    */ uxn_port(u, 0xe, nil_dei, nil_deo);
	/* empty    */ uxn_port(u, 0xf, nil_dei, nil_deo);
	return 1;
}

/*
 * All systems go
 */
int
main(int argc, char *argv[])
{
	Uxn uxn;

	if (!uxn_start(&uxn)) {
		fprintf(stderr, "couldn't start uxn!\n");
		return EXIT_FAILURE;
	}

	(void)appl_init();

	/* Get the VDI handle of the physical workstation */
	short ignore;
	short workstation = graf_handle(&ignore, &ignore, &ignore, &ignore);

	/* Open a virtual workstation based on the physical one, and get a handle */
	short work_in[16];
	short work_out[64];
	memset(work_in, 0, sizeof(work_in));
	memset(work_out, 0, sizeof(work_out));

	work_in[0] = 1; /* Want a handle for the screen @ current resolution */

	v_opnvwk(work_in, &workstation, work_out);

	if (!workstation) {
		return EXIT_FAILURE;
	}

	//void v_gtext ( int16_t handle, int16_t x, int16_t y, int8_t *string ); 
	//int16_t vst_color ( int16_t handle, int16_t color_index );
	//(void)vst_color(workstation, 1);
	//v_gtext(workstation, 24, 100, "HELLO THIS IS THE STRING");
	if (!load_rom(&uxn, "FIZZBUZZ.ROM")) {
		fprintf(stderr, "couldn't load the rom\n");
		return EXIT_FAILURE;
	}

	if (!uxn_eval(&uxn, PAGE_PROGRAM)) {
		fprintf(stderr, "error in the rom\n");
		return EXIT_FAILURE;
	}

	/* Main loop. */
	short msg[8];
	short kreturn;
	short events;
	bool done = false;

	while (!done) {
		events = evnt_multi(
			MU_KEYBD|MU_MESAG,
			0, 0, 0,  /* Forget about mouse button */
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Forget about mouse movement */
			msg,
			0, /* Forget about timer */
			0, 0, 0, 0, /* Forget about mouse button */
			&kreturn,
			0 /* Forget about mouse button */
		);

		if (events & MU_MESAG) {
			switch (msg[0]) {
			case AP_TERM:
				done = true;
				break;
			default:
				; /* OK */
			}
		}

		if (events & MU_KEYBD) {
			done = true; /* XXX quit on keypress */
		}
	}

	/* Clean up */
	v_clsvwk(workstation);
	appl_exit();
	return EXIT_SUCCESS;
}
