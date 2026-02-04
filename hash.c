#include <stdint.h>
#include <stdlib.h>

#include "listnode.h"

#define TABLE_SIZE 1 << 22



uint64_t splitmix64(uint64_t *seed) {
    uint64_t z = (*seed += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}
uint64_t side_key;
void init_zobrist(uint64_t zobrist[8][8][12]) {
    uint64_t seed = 0xDEADBEEFCAFEBABEULL;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            for (int p = 0; p < 12; p++) {
                zobrist[r][c][p] = splitmix64(&seed);
            }
        }
    }
    side_key = splitmix64(&seed);
}

uint64_t hash_val(int board[8][8], uint64_t zobrist[8][8][12], int is_white_tempo) {
    //[1, 12] -> pieces (pieces are negative so piece * -1)
    uint64_t h = 0x0000000000000000000;
    int piece_code;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j += 1) {
            if (board[i][j] >= 0) continue;
            piece_code = ((-1) * board[i][j]) - 1;
            h ^= zobrist[i][j][piece_code];
        }
    }
    if (!is_white_tempo) {
        h ^= side_key;
    }

    return h;
}
typedef struct Entry {
    uint64_t key;
    double pos_value; //white or black winning?
    ListNode *pbest_moves; //when a longer sequence of moves found at this state of board, moves and pos_value will be replaced
    int depth;
}Entry;

uint32_t map_table(uint64_t key) {
    return key & ((TABLE_SIZE) - 1); //table size is a power of 2 (not prime, but having such a table size lets us map without using % operator, faster)
}

void free_hashed_moves(Entry table[TABLE_SIZE]) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (table[i].pbest_moves != NULL) {
            ListNode* pcurr = table[i].pbest_moves;
            ListNode* pnext;
            while (pcurr != NULL) {
                pnext = pcurr->next;
                free(pcurr);
                pcurr = pnext;
            }
        }
    }
}
