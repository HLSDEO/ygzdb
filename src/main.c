#include <stdint.h>
#include "gfx.h"
#include "cards.h"
#include "duel.h"

typedef enum {
	ST_TITLE = 0,
	ST_DUEL,
	ST_GAMEOVER
} GameState;

static GameState state;
static Duel duel;

static void go_title(void) {
	state = ST_TITLE;
}

static void go_duel(void) {
	duel_init(&duel);
	state = ST_DUEL;
}

static void render_title(uint32_t frame) {
	int i, y;
	uint16_t star;

	gfx_clear(RGB(8, 8, 24));

	/* starfield */
	for (i = 0; i < 40; i++) {
		int sx = (i * 97 + (int)(frame / 2)) % SCREEN_W;
		int sy = (i * 53) % SCREEN_H;
		star = (i & 1) ? RGB(120, 120, 180) : RGB(60, 60, 100);
		gfx_px(sx, sy, star);
	}

	/* decorative card backs */
	gfx_fill(30, 40, 36, 50, RGB(20, 25, 60));
	gfx_rect(30, 40, 36, 50, RGB(80, 90, 160));
	gfx_fill(SCREEN_W - 66, 40, 36, 50, RGB(20, 25, 60));
	gfx_rect(SCREEN_W - 66, 40, 36, 50, RGB(80, 90, 160));

	gfx_text_shadow(SCREEN_W / 2 - 88, 50, "CARTAS DO", C_WHITE, C_BLACK);
	gfx_text_shadow(SCREEN_W / 2 - 72, 66, "DESTINO", C_GOLD, RGB(40, 20, 0));
	gfx_text_shadow(SCREEN_W / 2 - 104, 84, "A CARD DUEL FOR PS1", C_LGRAY, C_BLACK);

	/* blink prompt */
	if ((frame / 30) & 1) {
		gfx_text_shadow(SCREEN_W / 2 - 68, 150, "PRESS START", C_YELLOW, C_BLACK);
	}

	gfx_text_shadow(4, SCREEN_H - 18, "TURN-BASED MONSTER CARD BATTLE", C_GRAY, C_BLACK);
	gfx_text_shadow(4, SCREEN_H - 8, "HOMEBREW - PSN00BSDK", C_DGRAY, C_BLACK);

	/* small card samples row */
	y = 110;
	for (i = 0; i < 6; i++) {
		int cx = 40 + i * 40;
		uint16_t ac = attr_color((Attribute)i);
		gfx_fill(cx, y, 28, 20, RGB(30, 30, 50));
		gfx_fill(cx + 1, y + 1, 26, 6, ac);
		gfx_rect(cx, y, 28, 20, C_LGRAY);
	}
}

static void render(void) {
	static uint32_t frame;

	switch (state) {
		case ST_TITLE:
			render_title(frame);
			break;
		case ST_DUEL:
			duel_render(&duel);
			break;
		case ST_GAMEOVER:
			duel_render(&duel);
			break;
	}

	frame++;
	gfx_flip();
}

int main(void) {
	uint32_t btn;

	gfx_init();
	pads_init();

	go_title();

	for (;;) {
		btn = pads_pressed();

		switch (state) {
			case ST_TITLE:
				if (btn & (1 << 3)) /* START */
					go_duel();
				break;

			case ST_DUEL:
				duel_update(&duel);
				if (duel_finished(&duel)) {
					if (btn & (1 << 3))   /* START = title */
						go_title();
					if (btn & (1 << 12))  /* TRIANGLE = rematch */
						go_duel();
				}
				break;

			case ST_GAMEOVER:
				go_title();
				break;
		}

		render();
	}

	return 0;
}
