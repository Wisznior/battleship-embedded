#include "battleship.h"

#include <string.h>

static const ShipDef g_ship_defs[NUM_SHIPS] = {
    { "Carrier", 5 },
    { "Battleship", 4 },
    { "Cruiser", 3 },
    { "Submarine", 3 },
    { "Destroyer", 2 }
};

const ShipDef *ship_defs(void) { return g_ship_defs; }

void player_init(Player *p) {
    memset(p->own, WATER, sizeof(p->own));
    memset(p->track, WATER, sizeof(p->track));
    memset(p->ship_id, -1, sizeof(p->ship_id));

    for (uint8_t i = 0; i < NUM_SHIPS; i++) {
        p->ships[i].row        = 0;
        p->ships[i].col        = 0;
        p->ships[i].size       = g_ship_defs[i].size;
        p->ships[i].horizontal = 0;
        p->ships[i].hits       = 0;
        p->ships[i].placed     = 0;
    }
    p->ships_placed = 0;
    p->ships_alive  = 0;
}

PlaceResult place_ship(Player *p, uint8_t idx, uint8_t row, uint8_t col, uint8_t horizontal) {
    if (idx >= NUM_SHIPS)
        return PLACE_INVALID_INDEX;
    if (p->ships_placed >= NUM_SHIPS)
        return PLACE_ALL_PLACED;

    uint8_t size = g_ship_defs[idx].size;
    uint8_t dr   = horizontal ? 0 : 1;
    uint8_t dc   = horizontal ? 1 : 0;

    // bounds check
    uint8_t end_r = row + dr * (size - 1);
    uint8_t end_c = col + dc * (size - 1);
    if (end_r >= BOARD_SIZE || end_c >= BOARD_SIZE)
        return PLACE_OUT_OF_BOUNDS;

    // overlap + adjacency check (min 1-cell gap between ships)
    for (uint8_t i = 0; i < size; i++) {
        uint8_t r = row + dr * i;
        uint8_t c = col + dc * i;
        if (p->own[r][c] != WATER)
            return PLACE_OVERLAP;
        for (int8_t nr = (int8_t)r - 1; nr <= (int8_t)r + 1; nr++) {
            for (int8_t nc = (int8_t)c - 1; nc <= (int8_t)c + 1; nc++) {
                if (nr < 0 || nr >= BOARD_SIZE || nc < 0 || nc >= BOARD_SIZE)
                    continue;
                if (nr == (int8_t)r && nc == (int8_t)c)
                    continue;
                if (p->own[nr][nc] == SHIP)
                    return PLACE_ADJACENT;
            }
        }
    }

    for (uint8_t i = 0; i < size; i++) {
        uint8_t r = row + dr * i;
        uint8_t c = col + dc * i;
        p->own[r][c] = SHIP;
        p->ship_id[r][c] = (int8_t)idx;
    }

    p->ships[idx].row = row;
    p->ships[idx].col = col;
    p->ships[idx].size = size;
    p->ships[idx].horizontal = horizontal;
    p->ships[idx].hits = 0;
    p->ships[idx].placed = 1;
    p->ships_placed++;
    p->ships_alive++;

    return PLACE_OK;
}

void clear_ships(Player *p) {
    player_init(p);
}

uint8_t all_placed(const Player *p) {
    return p->ships_placed == NUM_SHIPS;
}

ShotResult receive_shot(Player *p, uint8_t row, uint8_t col) {
    if (row >= BOARD_SIZE || col >= BOARD_SIZE)
        return SHOT_INVALID;

    uint8_t cell = p->own[row][col];

    if (cell == HIT || cell == MISS)
        return SHOT_ALREADY;

    if (cell == WATER) {
        p->own[row][col] = MISS;
        return SHOT_MISS;
    }

    p->own[row][col] = HIT;

    int8_t sid = p->ship_id[row][col];
    p->ships[sid].hits++;

    if (p->ships[sid].hits >= p->ships[sid].size) {
        p->ships_alive--;
        if (p->ships_alive == 0)
            return SHOT_WIN;
        return SHOT_SUNK;
    }

    return SHOT_HIT;
}

void record_shot(Player *p, uint8_t row, uint8_t col, ShotResult result) {
    if (row >= BOARD_SIZE || col >= BOARD_SIZE)
        return;

    switch (result) {
        case SHOT_MISS:
            p->track[row][col] = MISS;
            break;
        case SHOT_HIT:
        case SHOT_SUNK:
        case SHOT_WIN:
            p->track[row][col] = HIT;
            break;
        default:
            break;
    }
}

uint8_t all_sunk(const Player *p) {
    return (p->ships_alive == 0) && (p->ships_placed == NUM_SHIPS);
}
