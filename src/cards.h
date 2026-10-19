#pragma once
#include <stdint.h>

typedef enum {
	ATTR_FIRE = 0,
	ATTR_WATER,
	ATTR_EARTH,
	ATTR_WIND,
	ATTR_LIGHT,
	ATTR_DARK,
	ATTR_COUNT
} Attribute;

typedef struct {
	const char *name;   /* short display name (<= 8 chars) */
	Attribute   attr;
	int         atk;
	int         def;
} CardDef;

#define CARD_COUNT   18
#define DECK_SIZE    30
#define MAX_HAND     8
#define MAX_SLOTS    5
#define START_LP     8000

extern const CardDef card_db[CARD_COUNT];

typedef struct {
	uint8_t def;        /* index into card_db */
} Card;

typedef enum {
	SLOT_EMPTY = 0,
	SLOT_ATK,
	SLOT_DEF_DOWN,   /* set face-down defense */
	SLOT_DEF_UP      /* defense face-up (after flip) */
} SlotState;

typedef struct {
	int       card;      /* index into card_db, -1 if empty */
	SlotState state;
} Slot;

typedef struct {
	Slot  field[MAX_SLOTS];
	Card  hand[MAX_HAND];
	int   hand_count;
	Card  deck[DECK_SIZE];
	int   deck_count;
	int   lp;
} PlayerState;

void   deck_build(PlayerState *p);
void   deck_shuffle(PlayerState *p);
int    deck_draw(PlayerState *p);
int    first_empty_slot(const PlayerState *p);

uint16_t attr_color(Attribute a);
const char *attr_name(Attribute a);
