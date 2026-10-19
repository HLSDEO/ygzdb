#include "cards.h"
#include "gfx.h"

const CardDef card_db[CARD_COUNT] = {
	/* 0 */ { "FLAMEDRG", ATTR_FIRE,  1800, 1200 },
	/* 1 */ { "AQUASERP", ATTR_WATER, 1600, 1400 },
	/* 2 */ { "STONEGOL", ATTR_EARTH, 1400, 1800 },
	/* 3 */ { "GALEHAWK", ATTR_WIND,  1500, 1100 },
	/* 4 */ { "SHADKNIT", ATTR_DARK,  1700, 1200 },
	/* 5 */ { "RADISAGE", ATTR_LIGHT, 1300, 1500 },
	/* 6 */ { "IRONWARD", ATTR_EARTH, 1200, 2000 },
	/* 7 */ { "UNDEDBAR", ATTR_DARK,  1900, 1000 },
	/* 8 */ { "THUNWYRM", ATTR_LIGHT, 2000, 1500 },
	/* 9 */ { "CRIMBEAST",ATTR_FIRE,  1600, 1300 },
	/*10 */ { "FROSTWOLF",ATTR_WATER, 1400, 1400 },
	/*11 */ { "BLADSQRE", ATTR_LIGHT, 1000, 1000 },
	/*12 */ { "ARCSORCR", ATTR_DARK,  1800, 1500 },
	/*13 */ { "STORMDJIN",ATTR_WIND,  1200, 1000 },
	/*14 */ { "METGUARD", ATTR_EARTH, 1000, 2200 },
	/*15 */ { "BONEDRGON",ATTR_DARK,  2100, 1600 },
	/*16 */ { "SPARKKNIT",ATTR_LIGHT, 1400, 1200 },
	/*17 */ { "MAGMARIDE",ATTR_FIRE,  1700, 1100 },
};

/* Deck list: pairs of most cards, singles of the strongest. 30 cards total. */
static const uint8_t deck_list[DECK_SIZE] = {
	0, 0,  1, 1,  2, 2,  3, 3,  4, 4,  5, 5,
	6, 6,  7, 7,  8,
	9, 9, 10, 10, 11, 11,
	12,
	13,
	14,
	15,
	16,
	17,
};

static uint32_t rng_state = 0x12345678;

static uint32_t rng(void) {
	rng_state = rng_state * 1664525u + 1013904223u;
	return (rng_state >> 16) & 0x7FFF;
}

void deck_build(PlayerState *p) {
	int i;
	for (i = 0; i < DECK_SIZE; i++)
		p->deck[i].def = deck_list[i];
	p->deck_count = DECK_SIZE;
	p->hand_count = 0;
	p->lp = START_LP;
	for (i = 0; i < MAX_SLOTS; i++) {
		p->field[i].card  = -1;
		p->field[i].state = SLOT_EMPTY;
	}
}

void deck_shuffle(PlayerState *p) {
	int i;
	/* seed from a timer-ish value mixed once */
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 17;
	rng_state ^= rng_state << 5;
	for (i = DECK_SIZE - 1; i > 0; i--) {
		int j = (int)(rng() % (uint32_t)(i + 1));
		Card t = p->deck[i];
		p->deck[i] = p->deck[j];
		p->deck[j] = t;
	}
}

int deck_draw(PlayerState *p) {
	if (p->deck_count <= 0)
		return -1;
	p->deck_count--;
	if (p->hand_count < MAX_HAND) {
		p->hand[p->hand_count++] = p->deck[p->deck_count];
		return 0;
	}
	/* hand full: discard drawn card */
	return 1;
}

int first_empty_slot(const PlayerState *p) {
	int i;
	for (i = 0; i < MAX_SLOTS; i++)
		if (p->field[i].state == SLOT_EMPTY)
			return i;
	return -1;
}

uint16_t attr_color(Attribute a) {
	switch (a) {
		case ATTR_FIRE:  return RGB(220, 70, 30);
		case ATTR_WATER: return RGB(50, 120, 230);
		case ATTR_EARTH: return RGB(170, 130, 60);
		case ATTR_WIND:  return RGB(70, 200, 100);
		case ATTR_LIGHT: return RGB(240, 220, 90);
		case ATTR_DARK:  return RGB(150, 70, 200);
		default:         return C_GRAY;
	}
}

const char *attr_name(Attribute a) {
	switch (a) {
		case ATTR_FIRE:  return "FIRE";
		case ATTR_WATER: return "WATER";
		case ATTR_EARTH: return "EARTH";
		case ATTR_WIND:  return "WIND";
		case ATTR_LIGHT: return "LIGHT";
		case ATTR_DARK:  return "DARK";
		default:         return "?";
	}
}
