#include <stdio.h>

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

typedef unsigned char Uint8;
typedef signed char Sint8;
typedef unsigned short Uint16;
typedef signed short Sint16;

#define FLAG_HALT 0x01
#define FLAG_LIT1 0x04
#define FLAG_LIT2 0x08
#define PAGE_DEVICE 0x0100
#define PAGE_VECTORS 0x0200

typedef struct {
	Uint8 ptr, error;
	Uint8 dat[256];
} Stack;

typedef struct {
	Uint16 ptr;
	Uint8 dat[65536];
} Memory;

struct Uxn;

typedef struct Device {
	Uint16 addr;
	Uint8 (*poke)(struct Uxn *, Uint16, Uint8, Uint8);
} Device;

typedef struct Uxn {
	Uint8 literal, status;
	Stack wst, rst, *src, *dst;
	Memory ram;
	Device dev[16];
} Uxn;

void setflag(Uint8 *status, char flag, int b);
int getflag(Uint8 *status, char flag);
int loaduxn(Uxn *c, char *filepath);
int bootuxn(Uxn *c);
int evaluxn(Uxn *u, Uint16 vec);
Device *portuxn(Uxn *u, Uint8 id, char *name, Uint8 (*pofn)(Uxn *, Uint16, Uint8, Uint8));
