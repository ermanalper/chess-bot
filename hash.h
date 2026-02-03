// hash.h
#pragma once

#ifndef CHESS_BOT_HASH_H
#define CHESS_BOT_HASH_H
#define TABLE_SIZE 5

#include <stdint.h>
#include "listnode.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct Entry {
        uint64_t key;
        double pos_value;          // white or black winning?
        ListNode *pbest_moves;     // best move list
        int depth;
    } Entry;

    uint32_t map_table(uint64_t key);
    void init_zobrist(uint64_t zobrist[8][8][12]);
    uint64_t hash_val(int board[8][8], uint64_t zobrist[8][8][12], int is_white_tempo);
    uint64_t splitmix64(uint64_t *seed);
    void free_hashed_moves(Entry table[TABLE_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // CHESS_BOT_HASH_H
