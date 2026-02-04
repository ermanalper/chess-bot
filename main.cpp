#include <array>
#include <iostream>

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
    ListNode* get_pseudo_legal_moves(int board[8][8], int is_white_tempo, int *is_king_captured);
    void free_list(ListNode* phead);
    void display_moves(ListNode *phead);
    void init_zobrist(uint64_t zobrist[8][8][12]);
    uint32_t map_table(uint64_t key);
    void free_hashed_moves(Entry table[TABLE_SIZE]);
    uint64_t hash_val(int board[8][8], uint64_t zobrist[8][8][12], int is_white_tempo);
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

ListNode* analyse_state(int board[8][8], int is_white_tempo) {
    uint64_t key = hash_val(board, zobrist, is_white_tempo);
    uint32_t ix = map_table(key);
    if (table[ix].key == key && table[ix].depth >= MAX_DEPTH) {
        /*if (table[ix].pbest_moves) {
            display_moves(table[ix].pbest_moves);
        }*/
        return table[ix].pbest_moves;
    }
    if (table[ix].pbest_moves != nullptr) {
        free_list(table[ix].pbest_moves);
        table[ix].pbest_moves = nullptr;
    }
    ListNode* best_path = nullptr;
    double eval = dfs(board, 0, is_white_tempo, &best_path, zobrist, table);
    /*
    if (best_path) {
        display_moves(best_path);
    }*/
    table[ix].key = key;
    table[ix].depth = MAX_DEPTH;
    table[ix].pos_value = eval;
    table[ix].pbest_moves = best_path;
    return best_path;
}

std::array<std::array<int, 2>, 2> get_player_move() {
    std::array<std::array<int, 2>, 2> move{};
    std::string input;

    while (true) {
        std::cout << "What's your move?";
        std::getline(std::cin, input);
        if (sscanf(input.c_str(), "%d %d : %d %d",
                   &move[0][0], &move[0][1],
                   &move[1][0], &move[1][1]) == 4) {
            return move;
                   } else {
                       std::cout << "Please rewrite your move in this form: 0 1 : 0 2 (the piece's cordinate you want to play, final coordinate)" << std::endl;
                   }
    }
}

int main() {
    int board[8][8];
    initialize_board(board);
    init_zobrist(zobrist);

    ListNode* comp_move = nullptr;
    do {
        std::array<std::array<int, 2>, 2> player_moves = get_player_move();
        int from_row = player_moves[0][0];
        int from_col = player_moves[0][1];

        int to_row   = player_moves[1][0];
        int to_col   = player_moves[1][1];

        int piece = board[from_row][from_col];
        board[from_row][from_col] = 0;
        board[to_row][to_col] = piece;

        std::cout << "Excellent move! Now it's my turn to think..." << std::endl;

        comp_move = analyse_state(board, 0);
        from_row = comp_move->is[0]; from_col = comp_move->is[1];
        to_row = comp_move->ts[0]; to_col = comp_move->ts[1];
        piece = board[from_row][from_col];
        board[from_row][from_col] = 0;
        board[to_row][to_col] = piece;
        std::cout << "I move " << from_row << " " << from_col
                  << " to " << to_row << " " << to_col << "\n";
    } while (comp_move != nullptr && comp_move->next != nullptr);



    free_hashed_moves(table);
    return 0;
}
