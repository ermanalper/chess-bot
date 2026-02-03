#include "hash.h"
#include "listnode.h"
#define MAX_DEPTH 5
#define TABLE_SIZE 1 << 22
enum Piece {
    W_PAWN = -1,
    B_PAWN = -2,
    W_ROOK = -3,
    B_ROOK = -4,
    W_KNIGHT = -5,
    B_KNIGHT = -6,
    W_BISHOP = -7,
    B_BISHOP = -8,
    W_KING = -9,
    B_KING = -10,
    W_QUEEN = -11,
    B_QUEEN = -12
};

extern "C" {
    double analyse_leaf_board(int board[8][8]);
    ListNode* get_pseudo_legal_moves(int board[8][8], int is_white_tempo);
    void free_list(ListNode* phead);
    void display_moves(ListNode *phead);
    double dfs(int board[8][8], const int depth, int is_white_tempo, ListNode **ppmoves_path, uint64_t zobrist[8][8][12], Entry table[TABLE_SIZE]);
};
uint64_t zobrist[8][8][12];
Entry table[TABLE_SIZE];


void initialize_board(int board[8][8]) {
    for (int i = 2; i < 6; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = 0;
        }
    }
    for (int j = 0; j < 8; j++) {
        board[1][j] = W_PAWN;
        board[6][j] = B_PAWN;
    }
    board[0][0] = W_ROOK;   board[0][7] = W_ROOK;
    board[0][1] = W_KNIGHT; board[0][6] = W_KNIGHT;
    board[0][2] = W_BISHOP; board[0][5] = W_BISHOP;
    board[0][3] = W_QUEEN;  board[0][4] = W_KING;

    board[7][0] = B_ROOK;   board[7][7] = B_ROOK;
    board[7][1] = B_KNIGHT; board[7][6] = B_KNIGHT;
    board[7][2] = B_BISHOP; board[7][5] = B_BISHOP;
    board[7][3] = B_QUEEN;  board[7][4] = B_KING;
}

void analyse_state(int board[8][8], int is_white_tempo) {
    uint64_t key = hash_val(board, zobrist, is_white_tempo);
    uint32_t ix = map_table(key);
    if (table[ix].key == key && table[ix].depth >= MAX_DEPTH) {
        if (table[ix].pbest_moves) {
            display_moves(table[ix].pbest_moves);
        }
        return;
    }
    if (table[ix].pbest_moves != nullptr) {
        free_list(table[ix].pbest_moves);
        table[ix].pbest_moves = nullptr;
    }
    ListNode* best_path = nullptr;
    double eval = dfs(board, 0, is_white_tempo, &best_path, zobrist, table);
    if (best_path) {
        display_moves(best_path);
    }
    table[ix].key = key;
    table[ix].depth = MAX_DEPTH;
    table[ix].pos_value = eval;
    table[ix].pbest_moves = best_path;
}

int main() {
    int board[8][8];
    initialize_board(board);
    init_zobrist(zobrist);



    free_hashed_moves(table);
    return 0;
}
