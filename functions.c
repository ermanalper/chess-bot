
enum Piece {
    W_PAWN = -1, B_PAWN = -2,
    W_ROOK = -3, B_ROOK = -4,
    W_KNIGHT = -5, B_KNIGHT = -6,
    W_BISHOP = -7, B_BISHOP = -8,
    W_KING = -9, B_KING = -10,
    W_QUEEN = -11, B_QUEEN = -12
};

int is_center_square(int x, int y) {
    return (x == 3 || x == 4) && (y == 3 || y == 4);
}

// x: file (0-7), y: rank (0-7) -> board[y][x]
int bishop_center_domination(int board[8][8], int bx, int by) {
    int dx[4] = { 1, -1, 1, -1 }, dy[4] = { 1, 1, -1, -1 };
    for (int d = 0; d < 4; d++) {
        int x = bx + dx[d], y = by + dy[d];
        while (x >= 0 && x < 8 && y >= 0 && y < 8) {
            if (is_center_square(x, y)) return 1;
            if (board[y][x] < 0) break;
            x += dx[d]; y += dy[d];
        }
    }
    return 0;
}

int rook_center_domination(int board[8][8], int rx, int ry) {
    int dx[4] = { 1, -1, 0, 0 }, dy[4] = { 0, 0, 1, -1 };
    for (int d = 0; d < 4; d++) {
        int x = rx + dx[d], y = ry + dy[d];
        while (x >= 0 && x < 8 && y >= 0 && y < 8) {
            if (is_center_square(x, y)) return 1;
            if (board[y][x] < 0) break;
            x += dx[d]; y += dy[d];
        }
    }
    return 0;
}

int queen_center_domination(int board[8][8], int qx, int qy) {
    return bishop_center_domination(board, qx, qy) || rook_center_domination(board, qx, qy);
}

double analyse_leaf_board(int board[8][8]) {
    int white_pawns[8], black_pawns[8];
    for(int k=0; k<8; k++) { white_pawns[k] = -1; black_pawns[k] = -1; }

    int black_king = 0, white_king = 0;
    double res = 0;

    // i: rank (y), j: file (x)
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int piece = board[i][j];
            if (piece >= 0) continue;

            double curr_res = 0;
            int is_black = ((piece & 1) == 0);

            switch (piece) {
                case W_PAWN:
                    curr_res = 1.0;
                    if (is_center_square(j, i)) curr_res += 0.1;

                    if (i > 4) curr_res += (i - 4) * 0.15;

                    //double pawns
                    if (white_pawns[j] != -1) res -= 0.2;
                    white_pawns[j] = i;
                    break;

                case B_PAWN:
                    curr_res = 1.0;
                    if (is_center_square(j, i)) curr_res += 0.1;

                    if (i < 3) curr_res += (3 - i) * 0.15;

                    //double pawns
                    if (black_pawns[j] != -1) res += 0.2;
                    black_pawns[j] = i;
                    break;

                case W_ROOK:
                case B_ROOK:
                    curr_res = 5.0;
                    if (rook_center_domination(board, j, i)) curr_res += 0.35;
                    break;

                case W_KNIGHT:
                case B_KNIGHT:
                    curr_res = 3.0;
                    if (i >= 2 && i <= 5 && j >= 2 && j <= 5) curr_res += 0.15;
                    break;

                case W_BISHOP:
                case B_BISHOP:
                    curr_res = 3.0;
                    if (bishop_center_domination(board, j, i)) curr_res += 0.1;
                    break;

                case W_QUEEN:
                case B_QUEEN:
                    curr_res = 9.0;
                    if (queen_center_domination(board, j, i)) curr_res += 0.1;
                    break;

                case W_KING:
                    white_king = 1;
                    if (is_center_square(j, i)) curr_res -= 0.4;

                    //pawn shield
                    if (i <= 1) {
                        int pi = i + 1;
                        if (j > 0 && board[pi][j-1] == W_PAWN) curr_res += 0.15;
                        if (board[pi][j]   == W_PAWN)          curr_res += 0.20;
                        if (j < 7 && board[pi][j+1] == W_PAWN) curr_res += 0.15;
                    }
                    break;

                case B_KING:
                    black_king = 1;
                    if (is_center_square(j, i)) curr_res -= 0.4;


                    if (i >= 6) {
                        int pi = i - 1;
                        if (j > 0 && board[pi][j-1] == B_PAWN) curr_res += 0.15;
                        if (board[pi][j]   == B_PAWN)          curr_res += 0.20;
                        if (j < 7 && board[pi][j+1] == B_PAWN) curr_res += 0.15;
                    }
                    break;
            }

            if (is_black) res -= curr_res;
            else          res += curr_res;
        }
    }

    // isolated pawns
    for (int k = 0; k < 8; k++) {
        if (white_pawns[k] != -1) {
            int has_neighbor = (k > 0 && white_pawns[k-1] != -1) || (k < 7 && white_pawns[k+1] != -1);
            if (!has_neighbor) res -= 0.25;
        }
        if (black_pawns[k] != -1) {
            int has_neighbor = (k > 0 && black_pawns[k-1] != -1) || (k < 7 && black_pawns[k+1] != -1);
            if (!has_neighbor) res += 0.25;
        }
    }


    if (white_king && !black_king) return 1000000.0;
    if (!white_king && black_king) return -1000000.0;
    if (!white_king && !black_king) return 0.0;

    return res;
}