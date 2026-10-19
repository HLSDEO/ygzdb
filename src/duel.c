#include <stdio.h>
#include <string.h>
#include "duel.h"
#include "gfx.h"

/* Shared layout constants (also used by duel_render.c) */
const int ZONE_W      = 48;
const int ZONE_H      = 36;
const int ZONE_GAP    = 4;
const int FIELD_X0    = ((SCREEN_W - (MAX_SLOTS * 48 + (MAX_SLOTS - 1) * 4)) / 2);
const int CPU_FIELD_Y = 20;
const int YOU_FIELD_Y = 98;
const int LOG_Y       = 62;
const int HAND_Y      = 170;
const int HAND_CARD_W = 40;
const int HAND_CARD_H = 58;

int duel_hand_card_x(int idx, int count) {
	int total, spacing, start;
	if (count <= 0) return SCREEN_W / 2;
	if (count == 1) return (SCREEN_W - HAND_CARD_W) / 2;
	spacing = HAND_CARD_W - 8;
	total = HAND_CARD_W + (count - 1) * spacing;
	if (total > SCREEN_W - 8) {
		spacing = (SCREEN_W - 8 - HAND_CARD_W) / (count - 1);
		total = HAND_CARD_W + (count - 1) * spacing;
	}
	start = (SCREEN_W - total) / 2;
	return start + idx * spacing;
}

int duel_slot_x(int i) {
	return FIELD_X0 + i * (ZONE_W + ZONE_GAP);
}

static void set_msg(Duel *d, const char *a, const char *b) {
	strncpy(d->msg0, a, sizeof(d->msg0) - 1);
	d->msg0[sizeof(d->msg0) - 1] = 0;
	strncpy(d->msg1, b ? b : "", sizeof(d->msg1) - 1);
	d->msg1[sizeof(d->msg1) - 1] = 0;
}

static void wait_then(Duel *d, int frames) {
	d->wait_timer = frames;
	d->sub = SUB_WAIT;
}

static void clear_slot(Slot *s) {
	s->card  = -1;
	s->state = SLOT_EMPTY;
}

static int has_attacker(const PlayerState *p, int mask) {
	int i;
	for (i = 0; i < MAX_SLOTS; i++)
		if (p->field[i].state == SLOT_ATK && !(mask & (1 << i)))
			return 1;
	return 0;
}

static int cpu_has_targets(const Duel *d) {
	int i;
	for (i = 0; i < MAX_SLOTS; i++)
		if (d->cpu.field[i].state != SLOT_EMPTY)
			return 1;
	return 0;
}

static void check_win(Duel *d) {
	if (d->you.lp <= 0) {
		d->you.lp = 0;
		d->winner = 2;
		set_msg(d, "YOU LOSE!", "CPU WINS THE DUEL");
	} else if (d->cpu.lp <= 0) {
		d->cpu.lp = 0;
		d->winner = 1;
		set_msg(d, "YOU WIN!", "DESTROYED THE CPU");
	}
}

/* --- Battle --- */

static void resolve_battle(Duel *d, int atk_slot, int def_slot, int by_cpu) {
	PlayerState *attacker_p = by_cpu ? &d->cpu : &d->you;
	PlayerState *defender_p = by_cpu ? &d->you : &d->cpu;
	Slot *as = &attacker_p->field[atk_slot];
	Slot *ds = &defender_p->field[def_slot];
	const CardDef *ac = &card_db[as->card];
	char b[48];
	int a_atk, d_val, diff;

	d->flash_timer  = 24;
	d->flash_slot   = def_slot;
	d->flash_is_cpu = by_cpu ? 0 : 1;

	if (ds->state == SLOT_DEF_DOWN)
		ds->state = SLOT_DEF_UP;

	a_atk = ac->atk;

	if (ds->state == SLOT_ATK) {
		d_val = card_db[ds->card].atk;
		if (a_atk > d_val) {
			diff = a_atk - d_val;
			defender_p->lp -= diff;
			clear_slot(ds);
			sprintf(b, "%d DAMAGE!", diff);
			set_msg(d, by_cpu ? "CPU ATTACKS!" : "YOU ATTACK!", b);
		} else if (a_atk < d_val) {
			diff = d_val - a_atk;
			attacker_p->lp -= diff;
			clear_slot(as);
			sprintf(b, "TAKE %d DAMAGE", diff);
			set_msg(d, by_cpu ? "CPU REFLECTS!" : "TOO STRONG!", b);
		} else {
			clear_slot(as);
			clear_slot(ds);
			set_msg(d, "EQUAL POWER!", "BOTH DESTROYED");
		}
	} else {
		d_val = card_db[ds->card].def;
		if (a_atk > d_val) {
			clear_slot(ds);
			set_msg(d, by_cpu ? "CPU BREAKS DEF!" : "DEFENSE BROKEN!", "MONSTER DESTROYED");
		} else if (a_atk < d_val) {
			diff = d_val - a_atk;
			attacker_p->lp -= diff;
			sprintf(b, "TAKE %d DAMAGE", diff);
			set_msg(d, by_cpu ? "STRONG DEFENSE!" : "CANNOT BREAK!", b);
		} else {
			set_msg(d, "DEFENSE HOLDS!", "NO DAMAGE");
		}
	}

	check_win(d);
}

static void direct_attack(Duel *d, int atk_slot, int by_cpu) {
	PlayerState *attacker_p = by_cpu ? &d->cpu : &d->you;
	PlayerState *defender_p = by_cpu ? &d->you : &d->cpu;
	Slot *as = &attacker_p->field[atk_slot];
	char b[48];

	d->flash_timer  = 24;
	d->flash_slot   = -1;
	d->flash_is_cpu = by_cpu ? 0 : 1;

	defender_p->lp -= card_db[as->card].atk;
	sprintf(b, "%d DIRECT!", card_db[as->card].atk);
	set_msg(d, by_cpu ? "CPU DIRECT ATTACK!" : "DIRECT ATTACK!", b);
	check_win(d);
}

/* --- Player actions --- */

static void player_summon(Duel *d, int defense) {
	int slot, i;
	Card c;
	const CardDef *cd;
	char b[48];

	if (d->summoned_this_turn) {
		set_msg(d, "ALREADY SUMMONED", "ONE SUMMON PER TURN");
		wait_then(d, 40);
		return;
	}
	if (d->hand_cursor < 0 || d->hand_cursor >= d->you.hand_count)
		return;
	slot = first_empty_slot(&d->you);
	if (slot < 0) {
		set_msg(d, "FIELD IS FULL", "NO EMPTY ZONE");
		wait_then(d, 40);
		return;
	}

	c  = d->you.hand[d->hand_cursor];
	cd = &card_db[c.def];
	d->you.field[slot].card  = c.def;
	d->you.field[slot].state = defense ? SLOT_DEF_DOWN : SLOT_ATK;

	for (i = d->hand_cursor; i < d->you.hand_count - 1; i++)
		d->you.hand[i] = d->you.hand[i + 1];
	d->you.hand_count--;
	if (d->hand_cursor >= d->you.hand_count && d->hand_cursor > 0)
		d->hand_cursor--;

	d->summoned_this_turn = 1;
	sprintf(b, "%s %s", cd->name, defense ? "SET" : "SUMMONED");
	set_msg(d, b, defense ? "FACE DOWN DEF" : "READY TO FIGHT");
	wait_then(d, 30);
}

static void next_attacker(Duel *d) {
	int i, start = d->field_cursor + 1;
	for (i = 0; i < MAX_SLOTS; i++) {
		int s = (start + i) % MAX_SLOTS;
		if (d->you.field[s].state == SLOT_ATK && !(d->attacked_slots & (1 << s))) {
			d->field_cursor = s;
			d->sub = SUB_SELECT_ATTACKER;
			return;
		}
	}
	d->phase = PHASE_END;
	set_msg(d, "NO MORE ATTACKS", "TURN ENDS");
	wait_then(d, 40);
}

static void begin_target_select(Duel *d) {
	int i;
	d->sub = SUB_SELECT_TARGET;
	if (!cpu_has_targets(d)) {
		d->target_cursor = -1;
		return;
	}
	for (i = 0; i < MAX_SLOTS; i++)
		if (d->cpu.field[i].state != SLOT_EMPTY) {
			d->target_cursor = i;
			return;
		}
}

/* --- AI --- */

static void ai_main_phase(Duel *d) {
	int best = -1, best_atk = -1, i, slot;
	Card c;
	const CardDef *cd;
	char b[48];

	d->summoned_this_turn = 1;
	for (i = 0; i < d->cpu.hand_count; i++) {
		int a = card_db[d->cpu.hand[i].def].atk;
		if (a > best_atk) { best_atk = a; best = i; }
	}

	slot = first_empty_slot(&d->cpu);
	if (best >= 0 && slot >= 0) {
		c  = d->cpu.hand[best];
		cd = &card_db[c.def];
		d->cpu.field[slot].card  = c.def;
		d->cpu.field[slot].state = (cd->atk >= 1400) ? SLOT_ATK : SLOT_DEF_DOWN;

		for (i = best; i < d->cpu.hand_count - 1; i++)
			d->cpu.hand[i] = d->cpu.hand[i + 1];
		d->cpu.hand_count--;

		sprintf(b, "%s %s", cd->name,
			d->cpu.field[slot].state == SLOT_ATK ? "SUMMONED" : "SET");
		set_msg(d, "CPU PLAYS:", b);
	} else {
		set_msg(d, "CPU THINKS...", "");
	}

	d->phase = PHASE_BATTLE;
	wait_then(d, 45);
}

static void ai_battle_step(Duel *d) {
	int i, atk_slot = -1, weakest = -1, weak_val = 99999, v;

	for (i = 0; i < MAX_SLOTS; i++) {
		if (d->cpu.field[i].state == SLOT_ATK && !(d->attacked_slots & (1 << i))) {
			atk_slot = i;
			break;
		}
	}

	if (atk_slot < 0) {
		d->phase = PHASE_END;
		set_msg(d, "CPU ENDS TURN", "YOUR MOVE");
		wait_then(d, 40);
		return;
	}

	d->attacked_slots |= (1 << atk_slot);

	for (i = 0; i < MAX_SLOTS; i++) {
		if (d->you.field[i].state == SLOT_EMPTY) continue;
		if (d->you.field[i].state == SLOT_ATK)
			v = card_db[d->you.field[i].card].atk;
		else
			v = card_db[d->you.field[i].card].def;
		if (v < weak_val) { weak_val = v; weakest = i; }
	}

	if (weakest >= 0)
		resolve_battle(d, atk_slot, weakest, 1);
	else
		direct_attack(d, atk_slot, 1);

	wait_then(d, 55);
}

/* --- Turns --- */

static void start_turn(Duel *d, int who) {
	PlayerState *p = who == 1 ? &d->you : &d->cpu;
	char b[48];

	d->turn = who;
	d->phase = PHASE_DRAW;
	d->summoned_this_turn = 0;
	d->attacked_slots = 0;
	d->hand_cursor = 0;
	d->field_cursor = -1;

	if (deck_draw(p) < 0) {
		d->winner = who == 1 ? 2 : 1;
		set_msg(d, who == 1 ? "NO CARDS LEFT!" : "CPU DECK OUT!",
			who == 1 ? "YOU LOSE BY DECKOUT" : "CPU OUT OF CARDS");
		wait_then(d, 60);
		return;
	}

	sprintf(b, "%s DRAWS", who == 1 ? "YOU" : "CPU");
	set_msg(d, b, "");

	d->phase = PHASE_MAIN;
	if (who == 1) {
		d->sub = SUB_SELECT_HAND;
	} else {
		d->sub = SUB_ENEMY_ACT;
		wait_then(d, 45);
	}
}

/* Called when a wait expires - resumes flow */
static void on_wait_done(Duel *d) {
	if (d->winner) {
		d->sub = SUB_GAME_OVER;
		return;
	}
	if (d->turn == 2) {
		if (d->phase == PHASE_MAIN)
			ai_main_phase(d);
		else if (d->phase == PHASE_BATTLE)
			ai_battle_step(d);
		else if (d->phase == PHASE_END)
			start_turn(d, 1);
	} else if (d->turn == 1) {
		if (d->phase == PHASE_END)
			start_turn(d, 2);
		else if (d->phase == PHASE_BATTLE)
			next_attacker(d);
		else if (d->phase == PHASE_MAIN)
			d->sub = SUB_SELECT_HAND;
	}
}

/* --- Public --- */

void duel_init(Duel *d) {
	int i;
	memset(d, 0, sizeof(*d));
	deck_build(&d->you);
	deck_build(&d->cpu);
	deck_shuffle(&d->you);
	deck_shuffle(&d->cpu);

	for (i = 0; i < 5; i++) {
		deck_draw(&d->you);
		deck_draw(&d->cpu);
	}

	d->winner = 0;
	d->turn   = 0;
	set_msg(d, "DUEL START!", "YOUR FIRST TURN");
	wait_then(d, 70);
}

void duel_update(Duel *d) {
	uint32_t btn = pads_pressed();

	if (d->flash_timer > 0)
		d->flash_timer--;

	if (d->winner) {
		if (d->wait_timer > 0) d->wait_timer--;
		if (d->wait_timer <= 0) d->sub = SUB_GAME_OVER;
		return;
	}

	if (d->wait_timer > 0) {
		d->wait_timer--;
		if (d->wait_timer == 0)
			on_wait_done(d);
		return;
	}

	if (d->turn == 0) {
		start_turn(d, 1);
		return;
	}

	/* ---- Player MAIN ---- */
	if (d->sub == SUB_SELECT_HAND && d->phase == PHASE_MAIN) {
		if (btn & (1 << 7))
			if (d->hand_cursor > 0) d->hand_cursor--;
		if (btn & (1 << 5))
			if (d->hand_cursor < d->you.hand_count - 1) d->hand_cursor++;
		if (btn & (1 << 14)) player_summon(d, 0);
		if (btn & (1 << 12)) player_summon(d, 1);
		if (btn & (1 << 13)) {
			if (has_attacker(&d->you, 0)) {
				d->phase = PHASE_BATTLE;
				d->field_cursor = -1;
				next_attacker(d);
			} else {
				d->phase = PHASE_END;
				set_msg(d, "END OF YOUR TURN", "CPU PLAYS NEXT");
				wait_then(d, 45);
			}
		}
		return;
	}

	/* ---- Player BATTLE: choose attacker ---- */
	if (d->sub == SUB_SELECT_ATTACKER) {
		if (btn & ((1 << 4) | (1 << 6) | (1 << 7) | (1 << 5)))
			next_attacker(d);
		if (btn & (1 << 14))
			begin_target_select(d);
		if (btn & (1 << 13)) {
			d->phase = PHASE_END;
			set_msg(d, "END OF YOUR TURN", "CPU PLAYS NEXT");
			wait_then(d, 45);
		}
		return;
	}

	/* ---- Player BATTLE: choose target ---- */
	if (d->sub == SUB_SELECT_TARGET) {
		if (btn & ((1 << 7) | (1 << 4))) {
			int i;
			if (d->target_cursor < 0) {
				for (i = MAX_SLOTS - 1; i >= 0; i--)
					if (d->cpu.field[i].state != SLOT_EMPTY) {
						d->target_cursor = i;
						break;
					}
			} else {
				for (i = d->target_cursor - 1; i >= 0; i--)
					if (d->cpu.field[i].state != SLOT_EMPTY) {
						d->target_cursor = i;
						break;
					}
			}
		}
		if (btn & ((1 << 5) | (1 << 6))) {
			int i, found = 0;
			if (d->target_cursor < 0) return;
			for (i = d->target_cursor + 1; i < MAX_SLOTS; i++)
				if (d->cpu.field[i].state != SLOT_EMPTY) {
					d->target_cursor = i;
					found = 1;
					break;
				}
			if (!found) d->target_cursor = -1;
		}
		if (btn & (1 << 14)) {
			if (d->target_cursor < 0)
				direct_attack(d, d->field_cursor, 0);
			else
				resolve_battle(d, d->field_cursor, d->target_cursor, 0);
			d->attacked_slots |= (1 << d->field_cursor);
			if (!d->winner)
				wait_then(d, 55);
		}
		if (btn & (1 << 13))
			d->sub = SUB_SELECT_ATTACKER;
		return;
	}
}

int duel_finished(const Duel *d) {
	return d->winner != 0;
}
