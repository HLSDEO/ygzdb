#include <psxgpu.h>
#include <psxapi.h>
#include "gfx.h"
#include "font8x8.h"

/* Software renderer: draw in main RAM, upload to VRAM via LoadImage. */

static uint16_t framebuf[2][SCREEN_W * SCREEN_H] __attribute__((aligned(64)));
static uint16_t *draw_ptr;
static int db_index;
static DISPENV disp_env;

void gfx_init(void) {
	ResetGraph(0);
	SetVideoMode(MODE_NTSC);

	db_index = 0;
	draw_ptr = framebuf[0];

	SetDefDispEnv(&disp_env, 0, 0, SCREEN_W, SCREEN_H);
	PutDispEnv(&disp_env);
	SetDispMask(1);
}

void gfx_flip(void) {
	RECT rect;
	int vram_y = db_index * SCREEN_H;

	setRECT(&rect, 0, vram_y, SCREEN_W, SCREEN_H);
	LoadImage(&rect, (const uint32_t *)framebuf[db_index]);
	DrawSync(0);
	VSync(0);

	SetDefDispEnv(&disp_env, 0, vram_y, SCREEN_W, SCREEN_H);
	PutDispEnv(&disp_env);

	db_index ^= 1;
	draw_ptr = framebuf[db_index];
}

/* --- Drawing primitives --- */

void gfx_clear(uint16_t color) {
	int i, n = SCREEN_W * SCREEN_H;
	for (i = 0; i < n; i++)
		draw_ptr[i] = color;
}

void gfx_px(int x, int y, uint16_t color) {
	if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H)
		return;
	draw_ptr[y * SCREEN_W + x] = color;
}

void gfx_fill(int x, int y, int w, int h, uint16_t color) {
	int i, j;
	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if (x + w > SCREEN_W) w = SCREEN_W - x;
	if (y + h > SCREEN_H) h = SCREEN_H - y;
	if (w <= 0 || h <= 0) return;

	for (j = y; j < y + h; j++) {
		uint16_t *p = draw_ptr + j * SCREEN_W + x;
		for (i = 0; i < w; i++)
			p[i] = color;
	}
}

void gfx_rect(int x, int y, int w, int h, uint16_t color) {
	gfx_fill(x, y, w, 1, color);
	gfx_fill(x, y + h - 1, w, 1, color);
	gfx_fill(x, y, 1, h, color);
	gfx_fill(x + w - 1, y, 1, h, color);
}

static char upc(char c) {
	if (c >= 'a' && c <= 'z') return (char)(c - 32);
	return c;
}

static void draw_glyph(int x, int y, char c, uint16_t color) {
	const uint8_t *glyph;
	int row, col;
	if (c < 32 || c > 127) return;
	glyph = font8x8[c - 32];
	for (row = 0; row < 8; row++) {
		uint8_t bits = glyph[row];
		for (col = 0; col < 8; col++) {
			if (bits & (0x80 >> col))
				gfx_px(x + col, y + row, color);
		}
	}
}

void gfx_text(int x, int y, const char *s, uint16_t color) {
	int cx = x;
	while (*s) {
		char c = *s++;
		if (c == '\n') { cx = x; y += 8; continue; }
		draw_glyph(cx, y, upc(c), color);
		cx += 8;
	}
}

void gfx_text_shadow(int x, int y, const char *s, uint16_t fg, uint16_t bg) {
	gfx_text(x + 1, y + 1, s, bg);
	gfx_text(x, y, s, fg);
}

int gfx_text_width(const char *s) {
	int n = 0;
	while (*s && *s != '\n') { n++; s++; }
	return n * 8;
}

void gfx_num_pad(int x, int y, int value, int digits, uint16_t color) {
	char buf[12];
	int i, n = 0;

	if (value < 0) value = 0;
	if (value == 0) buf[n++] = '0';
	while (value > 0 && n < 10) { buf[n++] = (char)('0' + (value % 10)); value /= 10; }
	while (n < digits && n < 10) buf[n++] = '0';

	for (i = 0; i < n; i++)
		draw_glyph(x + (n - 1 - i) * 8, y, buf[i], color);
}

void gfx_num(int x, int y, int value, uint16_t color) {
	char buf[12];
	int i, n = 0;

	if (value < 0) { buf[n++] = '-'; value = -value; }
	if (value == 0) buf[n++] = '0';
	while (value > 0 && n < 10) { buf[n++] = (char)('0' + (value % 10)); value /= 10; }

	for (i = n - 1; i >= 0; i--)
		draw_glyph(x + (n - 1 - i) * 8, y, buf[i], color);
}

/* --- Pads --- */

static uint8_t  pad_buff[2][34] __attribute__((aligned(4)));
static uint32_t pad_prev;

void pads_init(void) {
	InitPAD(pad_buff[0], 34, pad_buff[1], 34);
	StartPAD();
	ChangeClearPAD(0);
	pad_prev = 0;
}

uint32_t pads_held(void) {
	uint16_t raw = (uint16_t)pad_buff[0][3] | ((uint16_t)pad_buff[0][4] << 8);
	return (~raw) & 0xFFFF;
}

uint32_t pads_pressed(void) {
	uint32_t cur = pads_held();
	uint32_t p = cur & ~pad_prev;
	pad_prev = cur;
	return p;
}
