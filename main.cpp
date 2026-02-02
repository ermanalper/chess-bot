#include <iostream>
#include "listnode.h"
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
    double dfs(int board[8][8], const int depth, int is_white_tempo, ListNode **ppmoves_path);
};

int main() {
    int board[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = 0;
        }
    }
    /*
    for (int j = 0; j < 8; j += 2) {
        board[1][j] = W_PAWN;
        board[6][j] = B_PAWN;
    }*/
    board[7][0] = B_KING;
    board[5][0] = W_KING;
    board[7][1] = W_KNIGHT;
    board[5][2] = W_PAWN;
    board[6][3] = W_ROOK;
    board[7][3] = W_ROOK;
    board[6][4] = B_ROOK;

   // board[3][4] = W_KNIGHT;
   // board[3][0] = B_ROOK;
    //int w_king_pos[2] = {0, 7};
    ListNode* best_path = nullptr;
    double eval = dfs(board, 0, 1, &best_path);
    std::cout << eval << std::endl;
    if (best_path) {
        display_moves(best_path);
        free_list(best_path);
    }


    return 0;
}