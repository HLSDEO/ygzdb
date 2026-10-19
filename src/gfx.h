#pragma once
#include <stdint.h>

#define SCREEN_W 320
#define SCREEN_H 240
#define VRAM_W   1024

#define RGB(r,g,b) (uint16_t)((((r)>>3)&0x1F) | ((((g)>>3)&0x1F)<<5) | ((((b)>>3)&0x1F)<<10))

#define C_BLACK   RGB(0,0,0)
#define C_WHITE   RGB(255,255,255)
#define C_GRAY    RGB(80,80,80)
#define C_DGRAY   RGB(40,40,40)
#define C_LGRAY   RGB(160,160,160)
#define C_RED     RGB(220,40,40)
#define C_GREEN   RGB(40,200,80)
#define C_BLUE    RGB(60,120,240)
#define C_YELLOW  RGB(240,220,60)
#define C_ORANGE  RGB(240,140,30)
#define C_PURPLE  RGB(170,80,220)
#define C_CYAN    RGB(80,220,220)
#define C_BROWN   RGB(140,100,50)
#define C_GOLD    RGB(220,180,40)

typedef struct {
	int x, y, w, h;
} Rect;

void gfx_init(void);
void gfx_flip(void);
void gfx_clear(uint16_t color);

void gfx_px(int x, int y, uint16_t color);
void gfx_fill(int x, int y, int w, int h, uint16_t color);
void gfx_rect(int x, int y, int w, int h, uint16_t color);
void gfx_text(int x, int y, const char *s, uint16_t color);
void gfx_text_shadow(int x, int y, const char *s, uint16_t fg, uint16_t bg);
int  gfx_text_width(const char *s);
void gfx_num(int x, int y, int value, uint16_t color);
void gfx_num_pad(int x, int y, int value, int digits, uint16_t color);

/* Pads: active-high (bit set = pressed this frame) */
void     pads_init(void);
uint32_t pads_held(void);
uint32_t pads_pressed(void);
