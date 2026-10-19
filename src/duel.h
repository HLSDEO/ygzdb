#pragma once
#include <stdint.h>
#include "cards.h"

typedef enum {
	PHASE_DRAW = 0,
	PHASE_MAIN,
	PHASE_BATTLE,
	PHASE_END
} Phase;

typedef enum {
	SUB_IDLE = 0,
	SUB_SELECT_HAND,     /* player choosing card in hand */
	SUB_SELECT_ATTACKER, /* player choosing own monster to attack with */
	SUB_SELECT_TARGET,   /* player choosing enemy target / direct */
	SUB_WAIT,            /* delay showing a message */
	SUB_ENEMY_ACT,       /* AI acting step by step */
	SUB_GAME_OVER
} SubState;

typedef struct {
	PlayerState you;
	PlayerState cpu;
	int         turn;          /* 1 = player, 2 = cpu */
	Phase       phase;
	SubState    sub;
	int         hand_cursor;
	int         field_cursor;  /* attacker slot */
	int         target_cursor; /* enemy slot or -1 direct */
	int         wait_timer;
	int         summoned_this_turn;
	int         attacked_slots; /* bitmask of own slots that already attacked */
	int         winner;        /* 0 none, 1 you, 2 cpu */
	char        msg0[48];
	char        msg1[48];
	int         flash_timer;
	int         flash_slot;    /* slot being highlighted */
	int         flash_is_cpu;
} Duel;

void duel_init(Duel *d);
void duel_update(Duel *d, uint32_t btn);
void duel_render(const Duel *d);
int  duel_finished(const Duel *d);

/* layout helpers shared with duel_render.c */
extern const int ZONE_W, ZONE_H, ZONE_GAP, FIELD_X0;
extern const int CPU_FIELD_Y, YOU_FIELD_Y, LOG_Y, HAND_Y;
extern const int HAND_CARD_W, HAND_CARD_H;
int duel_hand_card_x(int idx, int count);
int duel_slot_x(int i);
