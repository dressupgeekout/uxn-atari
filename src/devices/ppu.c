#include "ppu.h"

/*
Copyright (c) 2021 Devine Lu Linvega
Copyright (c) 2021 Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

static Uint8 blending[5][16] = {
	{0, 0, 0, 0, 1, 0, 1, 1, 2, 2, 0, 2, 3, 3, 3, 0},
	{0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3},
	{1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1},
	{2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2},
	{1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0}};

static Uint8 font[][8] = {
	{0x00, 0x7c, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c},
	{0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
	{0x00, 0x7c, 0x82, 0x02, 0x7c, 0x80, 0x80, 0xfe},
	{0x00, 0x7c, 0x82, 0x02, 0x1c, 0x02, 0x82, 0x7c},
	{0x00, 0x0c, 0x14, 0x24, 0x44, 0x84, 0xfe, 0x04},
	{0x00, 0xfe, 0x80, 0x80, 0x7c, 0x02, 0x82, 0x7c},
	{0x00, 0x7c, 0x82, 0x80, 0xfc, 0x82, 0x82, 0x7c},
	{0x00, 0x7c, 0x82, 0x02, 0x1e, 0x02, 0x02, 0x02},
	{0x00, 0x7c, 0x82, 0x82, 0x7c, 0x82, 0x82, 0x7c},
	{0x00, 0x7c, 0x82, 0x82, 0x7e, 0x02, 0x82, 0x7c},
	{0x00, 0x7c, 0x82, 0x02, 0x7e, 0x82, 0x82, 0x7e},
	{0x00, 0xfc, 0x82, 0x82, 0xfc, 0x82, 0x82, 0xfc},
	{0x00, 0x7c, 0x82, 0x80, 0x80, 0x80, 0x82, 0x7c},
	{0x00, 0xfc, 0x82, 0x82, 0x82, 0x82, 0x82, 0xfc},
	{0x00, 0x7c, 0x82, 0x80, 0xf0, 0x80, 0x82, 0x7c},
	{0x00, 0x7c, 0x82, 0x80, 0xf0, 0x80, 0x80, 0x80}};

void
ppu_palette(Ppu *p, Uint8 *addr)
{
	int i;
	for(i = 0; i < 4; ++i) {
		Uint8
			r = (*(addr + i / 2) >> (!(i % 2) << 2)) & 0x0f,
			g = (*(addr + 2 + i / 2) >> (!(i % 2) << 2)) & 0x0f,
			b = (*(addr + 4 + i / 2) >> (!(i % 2) << 2)) & 0x0f;
		p->palette[i] = 0xff000000 | (r << 20) | (r << 16) | (g << 12) | (g << 8) | (b << 4) | b;
	}
	for(i = 4; i < 16; ++i)
		p->palette[i] = p->palette[i / 4];
	p->fg.reqdraw = p->bg.reqdraw = 1;
}

void
ppu_resize(Ppu *p, Uint16 width, Uint16 height)
{
	Uint8 
	*bg = realloc(p->bg.p, width * height),
	*fg = realloc(p->fg.p, width * height);
	if(!bg || !fg)
		return;
	p->bg.p = bg;
	p->fg.p = fg;
	p->width = width;
	p->height = height;
	ppu_clear(p, &p->bg);
	ppu_clear(p, &p->fg);
}

void
ppu_clear(Ppu *p, Layer *layer)
{
	Uint32 i, size = p->width * p->height;
	for(i = 0; i < size; ++i)
		layer->p[i] = 0x00;
}

void
ppu_redraw(Ppu *p, Uint32 *screen)
{
	Uint32 i, size = p->width * p->height;
	for(i = 0; i < size; ++i)
		screen[i] = p->palette[p->fg.p[i] ? p->fg.p[i] : p->bg.p[i]];
	p->fg.reqdraw = p->bg.reqdraw = 0;
}

void
ppu_write(Ppu *p, Layer *layer, Uint16 x, Uint16 y, Uint8 color)
{
	if(x < p->width && y < p->height) {
		Uint32 i = (x + y * p->width);
		Uint8 prev = layer->p[i];
		if(color != prev) {
			layer->p[i] = color;
			layer->reqdraw = 1;
		}
	}
}

void
ppu_1bpp(Ppu *p, Layer *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color, Uint8 flipx, Uint8 flipy)
{
	Uint16 v, h;
	for(v = 0; v < 8; ++v)
		for(h = 0; h < 8; ++h) {
			Uint8 ch1 = (sprite[v] >> (7 - h)) & 0x1;
			if(ch1 || blending[4][color])
				ppu_write(p,
					layer,
					x + (flipx ? 7 - h : h),
					y + (flipy ? 7 - v : v),
					blending[ch1][color]);
		}
}

void
ppu_2bpp(Ppu *p, Layer *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color, Uint8 flipx, Uint8 flipy)
{
	Uint16 v, h;
	for(v = 0; v < 8; ++v)
		for(h = 0; h < 8; ++h) {
			Uint8 ch1 = ((sprite[v] >> (7 - h)) & 0x1);
			Uint8 ch2 = ((sprite[v + 8] >> (7 - h)) & 0x1);
			Uint8 ch = ch1 + ch2 * 2;
			if(ch || blending[4][color])
				ppu_write(p,
					layer,
					x + (flipx ? 7 - h : h),
					y + (flipy ? 7 - v : v),
					blending[ch][color]);
		}
}

void
ppu_debug(Ppu *p, Uint8 *stack, Uint8 wptr, Uint8 rptr, Uint8 *memory)
{
	Uint8 i, x, y, b;
	for(i = 0; i < 0x20; ++i) {
		x = ((i % 8) * 3 + 1) * 8, y = (i / 8 + 1) * 8, b = stack[i];
		/* working stack */
		ppu_1bpp(p, &p->fg, x, y, font[(b >> 4) & 0xf], 1 + (wptr == i) * 0x7, 0, 0);
		ppu_1bpp(p, &p->fg, x + 8, y, font[b & 0xf], 1 + (wptr == i) * 0x7, 0, 0);
		y = 0x28 + (i / 8 + 1) * 8;
		b = memory[i];
		/* return stack */
		ppu_1bpp(p, &p->fg, x, y, font[(b >> 4) & 0xf], 3, 0, 0);
		ppu_1bpp(p, &p->fg, x + 8, y, font[b & 0xf], 3, 0, 0);
	}
	/* return pointer */
	ppu_1bpp(p, &p->fg, 0x8, y + 0x10, font[(rptr >> 4) & 0xf], 0x2, 0, 0);
	ppu_1bpp(p, &p->fg, 0x10, y + 0x10, font[rptr & 0xf], 0x2, 0, 0);
	/* guides */
	for(x = 0; x < 0x10; ++x) {
		ppu_write(p, &p->fg, x, p->height / 2, 2);
		ppu_write(p, &p->fg, p->width - x, p->height / 2, 2);
		ppu_write(p, &p->fg, p->width / 2, p->height - x, 2);
		ppu_write(p, &p->fg, p->width / 2, x, 2);
		ppu_write(p, &p->fg, p->width / 2 - 0x10 / 2 + x, p->height / 2, 2);
		ppu_write(p, &p->fg, p->width / 2, p->height / 2 - 0x10 / 2 + x, 2);
	}
}
