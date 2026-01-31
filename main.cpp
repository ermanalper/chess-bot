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
};

int main() {
    int board[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = 0;
        }
    }
    for (int j = 0; j < 8; j++) {
        board[1][j] = W_PAWN;
        board[6][j] = B_PAWN;
    }
    board[0][7] = W_KING;
    board[7][0] = B_KING;

    double val = analyse_leaf_board(board);
    std::cout <<  val << std::endl;

    return 0;
}