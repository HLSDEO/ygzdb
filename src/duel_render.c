#include <stdio.h>
#include "duel.h"
#include "gfx.h"

static void draw_card_face(int x, int y, int w, int h, int def_idx, int selected) {
	const CardDef *cd = &card_db[def_idx];
	uint16_t ac = attr_color(cd->attr);
	char nm[8], s[16];
	int i;

	gfx_fill(x, y, w, h, RGB(30, 30, 50));
	gfx_fill(x + 1, y + 1, w - 2, 10, ac);
	gfx_fill(x + 3, y + 13, w - 6, h - 30, RGB(20, 20, 35));
	gfx_fill(x + 1, y + h - 16, w - 2, 7, RGB(50, 50, 80));
	gfx_fill(x + 1, y + h - 8, w - 2, 7, RGB(40, 40, 60));
	gfx_rect(x, y, w, h, selected ? C_WHITE : C_LGRAY);

	for (i = 0; i < 7 && cd->name[i]; i++) nm[i] = cd->name[i];
	nm[i] = 0;
	gfx_text(x + 2, y + h - 15, nm, C_WHITE);

	sprintf(s, "%4d/%4d", cd->atk, cd->def);
	gfx_text(x + 1, y + h - 7, s, C_GOLD);
}

static void draw_card_back(int x, int y, int w, int h, int selected) {
	int i, j;
	gfx_fill(x, y, w, h, RGB(20, 25, 60));
	for (j = y + 2; j < y + h - 2; j += 4)
		for (i = x + 2; i < x + w - 2; i += 4)
			gfx_px(i, j, RGB(40, 50, 100));
	gfx_rect(x, y, w, h, selected ? C_YELLOW : RGB(80, 90, 160));
}

static void draw_slot(int x, int y, const Slot *s, int selected, int flash) {
	if (flash)
		gfx_fill(x - 1, y - 1, ZONE_W + 2, ZONE_H + 2, C_RED);

	if (s->state == SLOT_EMPTY) {
		gfx_fill(x, y, ZONE_W, ZONE_H, RGB(15, 15, 25));
		gfx_rect(x, y, ZONE_W, ZONE_H, RGB(50, 50, 70));
		return;
	}

	if (s->state == SLOT_DEF_DOWN) {
		draw_card_back(x, y, ZONE_W, ZONE_H, selected);
		gfx_text(x + 4, y + 4, "DEF", C_CYAN);
	} else {
		draw_card_face(x, y, ZONE_W, ZONE_H, s->card, selected);
		if (s->state == SLOT_DEF_UP)
			gfx_text(x + 2, y + 13, "SET", C_CYAN);
	}

	if (selected)
		gfx_rect(x - 1, y - 1, ZONE_W + 2, ZONE_H + 2, C_YELLOW);
}

static void draw_lp_bar(int x, int y, int lp, const char *label, int active) {
	char s[24];
	uint16_t col = active ? C_YELLOW : C_WHITE;
	int w, maxw = 80;

	sprintf(s, "%s %d", label, lp);
	gfx_text(x, y, s, col);

	w = (lp > 0 ? lp : 0) * maxw / START_LP;
	if (w > maxw) w = maxw;
	gfx_fill(x, y + 9, maxw, 5, RGB(40, 40, 40));
	gfx_fill(x, y + 9, w, 5, (lp <= 2000) ? C_RED : C_GREEN);
	gfx_rect(x, y + 9, maxw, 5, C_WHITE);
}

void duel_render(const Duel *d) {
	int i, x, y;
	char s[48];

	gfx_clear(RGB(12, 14, 28));

	/* table background bands */
	gfx_fill(0, CPU_FIELD_Y - 14, SCREEN_W, ZONE_H + 18, RGB(18, 20, 38));
	gfx_fill(0, YOU_FIELD_Y - 14, SCREEN_W, ZONE_H + 18, RGB(18, 20, 38));

	/* ---- CPU bar ---- */
	draw_lp_bar(4, 2, d->cpu.lp, "CPU", d->turn == 2);
	sprintf(s, "HAND %d  DECK %d", d->cpu.hand_count, d->cpu.deck_count);
	gfx_text(SCREEN_W - gfx_text_width(s) - 4, 2, s, C_LGRAY);

	/* CPU hand (backs, stacked small) */
	for (i = 0; i < d->cpu.hand_count && i < 8; i++) {
		x = 4 + i * 10;
		y = 12;
		gfx_fill(x, y, 8, 10, RGB(20, 25, 60));
		gfx_rect(x, y, 8, 10, RGB(80, 90, 160));
	}

	/* CPU field */
	for (i = 0; i < MAX_SLOTS; i++) {
		int sel = 0, flash = 0;
		x = duel_slot_x(i);
		if (d->sub == SUB_SELECT_TARGET && d->target_cursor == i)
			sel = 1;
		if (d->flash_timer > 0 && d->flash_is_cpu && d->flash_slot == i)
			flash = 1;
		draw_slot(x, CPU_FIELD_Y, &d->cpu.field[i], sel, flash);
	}

	/* ---- Log ---- */
	gfx_fill(8, LOG_Y - 2, SCREEN_W - 16, 22, RGB(8, 8, 16));
	gfx_rect(8, LOG_Y - 2, SCREEN_W - 16, 22, RGB(60, 60, 90));
	gfx_text_shadow(12, LOG_Y, d->msg0, C_WHITE, C_BLACK);
	gfx_text_shadow(12, LOG_Y + 10, d->msg1, C_LGRAY, C_BLACK);

	/* Direct-attack flash bar */
	if (d->flash_timer > 0 && d->flash_slot < 0) {
		int fw = (d->flash_timer * 4) % SCREEN_W;
		gfx_fill(fw, LOG_Y + 8, 24, 3, C_RED);
	}

	/* ---- Player field ---- */
	for (i = 0; i < MAX_SLOTS; i++) {
		int sel = 0, flash = 0;
		x = duel_slot_x(i);
		if (d->sub == SUB_SELECT_ATTACKER && d->field_cursor == i)
			sel = 1;
		if (d->flash_timer > 0 && !d->flash_is_cpu && d->flash_slot == i)
			flash = 1;
		draw_slot(x, YOU_FIELD_Y, &d->you.field[i], sel, flash);
	}

	/* ---- Player bar ---- */
	draw_lp_bar(4, 140, d->you.lp, "YOU", d->turn == 1);

	/* Phase indicator (right) */
	{
		const char *pn = "?";
		int px;
		switch (d->phase) {
			case PHASE_DRAW:   pn = "DRAW";   break;
			case PHASE_MAIN:   pn = "MAIN";   break;
			case PHASE_BATTLE: pn = "BATTLE"; break;
			case PHASE_END:    pn = "END";    break;
		}
		sprintf(s, "TURN %d  %s", d->turn, pn);
		px = SCREEN_W - gfx_text_width(s) - 4;
		gfx_text(px, 140, s, C_CYAN);

		sprintf(s, "HAND %d  DECK %d", d->you.hand_count, d->you.deck_count);
		gfx_text(px - gfx_text_width(s) - 6, 140, s, C_LGRAY);
	}

	/* ---- Hand ---- */
	for (i = 0; i < d->you.hand_count; i++) {
		int sel = (d->sub == SUB_SELECT_HAND && d->hand_cursor == i);
		int hx = duel_hand_card_x(i, d->you.hand_count);
		int hy = sel ? HAND_Y - 8 : HAND_Y;
		draw_card_face(hx, hy, HAND_CARD_W, HAND_CARD_H, d->you.hand[i].def, sel);
	}

	/* ---- Help line ---- */
	if (d->sub == SUB_SELECT_HAND && d->phase == PHASE_MAIN) {
		gfx_text_shadow(4, SCREEN_H - 10,
			"L/R CARD  X SUMMON  TRI SET  O END",
			C_YELLOW, C_BLACK);
	} else if (d->sub == SUB_SELECT_ATTACKER) {
		gfx_text_shadow(4, SCREEN_H - 10,
			"L/R NEXT  X ATTACK  O END BATTLE",
			C_YELLOW, C_BLACK);
	} else if (d->sub == SUB_SELECT_TARGET) {
		if (d->target_cursor < 0)
			gfx_text_shadow(4, SCREEN_H - 10, "X: DIRECT ATTACK  O: CANCEL",
				C_RED, C_BLACK);
		else
			gfx_text_shadow(4, SCREEN_H - 10, "L/R TARGET  X ATTACK  O CANCEL",
				C_YELLOW, C_BLACK);
	}

	/* ---- Game over overlay ---- */
	if (d->winner) {
		gfx_fill(40, 80, SCREEN_W - 80, 70, RGB(10, 10, 20));
		gfx_rect(40, 80, SCREEN_W - 80, 70, C_GOLD);
		if (d->winner == 1) {
			gfx_text_shadow(SCREEN_W / 2 - 48, 92, "YOU WIN!", C_GOLD, C_BLACK);
			gfx_text_shadow(SCREEN_W / 2 - 80, 110, "DESTROYED THE CPU", C_WHITE, C_BLACK);
		} else {
			gfx_text_shadow(SCREEN_W / 2 - 48, 92, "YOU LOSE", C_RED, C_BLACK);
			gfx_text_shadow(SCREEN_W / 2 - 72, 110, "CPU WINS THE DUEL", C_WHITE, C_BLACK);
		}
		gfx_text_shadow(SCREEN_W / 2 - 72, 130, "PRESS START", C_YELLOW, C_BLACK);
	}
}
