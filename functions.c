#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "hash.h"
#include "listnode.h"
#define IS_WHITE(x) (x < 0 && (x % 2 == -1))
#define IS_BLACK(x) (x < 0 && (x % 2 == 0))
#define MAX_DEPTH 5
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

int is_white_piece(int piece) {
    return (piece & 1) == 0;
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
            int is_black = (piece % 2 == 0);

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
void free_list(ListNode* phead) {
    ListNode* pcurr = phead;
    ListNode* pnext;
    while (pcurr != NULL) {
        pnext = pcurr->next;
        free(pcurr);
        pcurr = pnext;
    }
}
ListNode* create_move(int is0, int is1, int ts0, int ts2) {
    ListNode *n = malloc(sizeof(ListNode));
    n->is[0] = is0; n->is[1] = is1;
    n->ts[0] = ts0; n->ts[1] = ts2;
    n->next = NULL;
    return n;
}
ListNode* generate_moves_generic(int board[8][8], int i, int j, int dirs[][2], int num_dirs,
                                 int max_steps, int is_white_tempo, ListNode **ptail, ListNode *phead, int *is_king_captured) {
    //if enemy king can be captured, return that move, else return NULL (other moves are added to the list)
    int enemy_king = is_white_tempo ? B_KING : W_KING;
    for (int d = 0; d < num_dirs; d++) {
        for (int step = 1; step <= max_steps; step++) {
            int ni = i + dirs[d][0] * step;
            int nj = j + dirs[d][1] * step;

            if (ni < 0 || ni >= 8 || nj < 0 || nj >= 8) break; //out of board bounds

            int target = board[ni][nj];

            if (target >= 0) {
                ListNode *pmove = create_move(i, j, ni, nj);
                (*ptail)->next = pmove;
                *ptail = pmove;
            }
            else {
                int is_enemy = is_white_tempo ? IS_BLACK(target) : IS_WHITE(target);
                if (is_enemy) {
                    ListNode *pmove = create_move(i, j, ni, nj);
                    if (target == enemy_king) {
                        free_list(phead->next);
                        *is_king_captured = 1;
                        return pmove;
                    }
                    (*ptail)->next = pmove;
                    *ptail = pmove;
                }
                break;
            }
        }
    }
    return NULL;
}

ListNode* get_pseudo_legal_moves(int board[8][8], int is_white_tempo, int *is_king_captured) {
    //White: piece & 1 == 1, Black: piece & 1 == 0
    ListNode dummy;
    dummy.next = NULL;
    ListNode *ptail = &dummy;
    ListNode *king_capture;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] >= 0 || (IS_WHITE(board[i][j]) != is_white_tempo)) continue;
            int piece = board[i][j];
            //piece found
            //perform all moves. if enemy king can be captured, return that move immediately.
            //than, either an illegal move is played by the opponent, or it's a checkmate. eiter way, the minimax algorithm will try to avoid that branch
            switch (piece) {
                case W_PAWN:
                    if (i + 1 < 8 && board[i + 1][j] >= 0) {
                        ListNode *pmove = create_move(i, j, i + 1, j);
                        ptail->next = pmove; ptail = pmove;
                        if (i == 1 && board[i + 2][j] >= 0) {
                            ListNode *double_move = create_move(i, j, i + 2, j);
                            ptail->next = double_move; ptail = double_move;
                        }
                    }

                    for (int d = -1; d <= 1 && i + 1 < 8; d += 2) {
                        if (0 <= j + d && j + d < 8 && (IS_BLACK(board[i + 1][j + d]))) {
                            ListNode *pmove = create_move(i, j, i + 1, j + d);
                            if (board[i + 1][j + d] == B_KING) {
                                //enemy king is capturable
                                free_list(dummy.next);
                                *is_king_captured = 1;
                                return pmove;
                            } else {
                                ptail->next = pmove; ptail = pmove;
                            }
                        }
                    }
                    break;
                case B_PAWN:
                    if (i - 1 >= 0 && board[i - 1][j] >= 0) {
                        ListNode *pmove = create_move(i, j, i - 1, j);
                        ptail->next = pmove; ptail = pmove;
                        if (i == 6 && board[i - 2][j] >= 0) {
                            ListNode *double_move = create_move(i, j, i - 2, j);
                            ptail->next = double_move; ptail = double_move;
                        }
                    }

                    for (int d = -1; d <= 1 && i - 1 >= 0; d += 2) {
                        if (0 <= j + d && j + d < 8 && (IS_WHITE(board[i - 1][j + d]))) {
                            ListNode *pmove = create_move(i, j, i - 1, j + d);
                            if (board[i - 1][j + d] == W_KING) {
                                //enemy king is capturable
                                free_list(dummy.next);
                                *is_king_captured = 1;
                                return pmove;
                            } else {
                                ptail->next = pmove; ptail = pmove;
                            }
                        }
                    }
                    break;

                case W_KNIGHT: case B_KNIGHT: {
                    int dirs[8][2] = {{2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2}};
                    king_capture = generate_moves_generic(board, i, j, dirs, 8, 1, is_white_tempo, &ptail, &dummy, is_king_captured);
                    if (king_capture) return king_capture;
                    break;
                }

                case W_ROOK: case B_ROOK: {
                    int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
                    king_capture = generate_moves_generic(board, i, j, dirs, 4, 7, is_white_tempo, &ptail, &dummy, is_king_captured);
                    if (king_capture) return king_capture;
                    break;
                }

                case W_BISHOP: case B_BISHOP: {
                    int dirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
                    king_capture = generate_moves_generic(board, i, j, dirs, 4, 7, is_white_tempo, &ptail, &dummy, is_king_captured);
                    if (king_capture) return king_capture;
                    break;
                }

                case W_QUEEN: case B_QUEEN: {
                    int dirs[8][2] = {{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
                    king_capture = generate_moves_generic(board, i, j, dirs, 8, 7, is_white_tempo, &ptail, &dummy, is_king_captured);
                    if (king_capture) return king_capture;
                    break;
                }

                case W_KING: case B_KING: {
                    int dirs[8][2] = {{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
                    king_capture = generate_moves_generic(board, i, j, dirs, 8, 1, is_white_tempo, &ptail, &dummy, is_king_captured);
                    if (king_capture) return king_capture;
                    break;
                }



            }
        }
    }
    return dummy.next;
}

void display_moves(ListNode *phead) {
    ListNode *pcurr = phead;
    while (pcurr != NULL) {
        int is0 = pcurr->is[0]; int is1 = pcurr->is[1];
        int ts0 = pcurr->ts[0]; int ts1 = pcurr->ts[1];
        printf("Move: From   %d %d    To   %d %d \n", is0, is1, ts0, ts1);
        pcurr = pcurr->next;
    }
}

int make_move(int board[8][8], ListNode *pmove, int *pawn_promoted) {
    //Returns: The piece on the target square, if empty: 0 (or positive garbage value if board is not set properly)
    int is0 = pmove->is[0]; int is1 = pmove->is[1];
    int piece = board[is0][is1];
    int ts0 = pmove->ts[0]; int ts1 = pmove->ts[1];
    int ret_val = board[ts0][ts1];
    board[ts0][ts1] = piece;
    board[is0][is1] = 0;
    if ((piece == W_PAWN && ts0 == 7) || (piece == B_PAWN && ts0 == 0)) {
        *pawn_promoted = 1;
        if (piece == W_PAWN) {
            board[ts0][ts1] = W_QUEEN;
        } else if (piece == B_PAWN) {
            board[ts0][ts1] = B_QUEEN;
        }
    }
    return ret_val;
}
void undo_move(int board[8][8], ListNode *pmove, int piece_taken, int pawn_promoted) {
    int is0 = pmove->is[0]; int is1 = pmove->is[1];
    int ts0 = pmove->ts[0]; int ts1 = pmove->ts[1];
    int piece = !pawn_promoted ? board[ts0][ts1] : (IS_WHITE(board[ts0][ts1]) ? W_PAWN : B_PAWN);
    board[is0][is1] = piece;
    board[ts0][ts1] = piece_taken;
}

double dfs(int board[8][8], const int depth, int is_white_tempo, ListNode **ppmoves_path, uint64_t zobrist[8][8][12], Entry table[TABLE_SIZE]) {
    uint64_t key = hash_val(board, zobrist, is_white_tempo);
    uint32_t ix = map_table(key);
    int current_remaining_depth = MAX_DEPTH - depth;

    if (table[ix].key == key && table[ix].depth >= current_remaining_depth) {
        if (ppmoves_path != NULL)
            *ppmoves_path = NULL;
        return table[ix].pos_value;

    }

    if (depth == MAX_DEPTH) {
        if (ppmoves_path) *ppmoves_path = NULL;
        return analyse_leaf_board(board);
    }

    int is_king_captured = 0;
    ListNode *pmoves = get_pseudo_legal_moves(board, is_white_tempo, &is_king_captured);

    if (is_king_captured) {
        if (ppmoves_path) {
            *ppmoves_path = create_move(pmoves->is[0], pmoves->is[1], pmoves->ts[0], pmoves->ts[1]);
            (*ppmoves_path)->next = NULL;
        }

        double score = is_white_tempo ? 1000000.0 : -1000000.0;

        free_list(pmoves);
        return score;
    }

    ListNode *pcurr = pmoves;
    double thresh = is_white_tempo ? -2000000.0 : 2000000.0;
    ListNode *best_full_path = NULL;
    while (pcurr != NULL) {
        int pawn_promoted = 0;
        int piece_taken = make_move(board, pcurr, &pawn_promoted);

        ListNode *child_path = NULL;
        double curr_path_val = dfs(board, depth + 1, !is_white_tempo, &child_path, zobrist, table);

        if ((is_white_tempo && curr_path_val > thresh) || (!is_white_tempo && curr_path_val < thresh)) {
            thresh = curr_path_val;

            if (best_full_path) free_list(best_full_path);

            best_full_path = create_move(pcurr->is[0], pcurr->is[1], pcurr->ts[0], pcurr->ts[1]);
            best_full_path->next = child_path;
        } else {
            if (child_path) free_list(child_path);
        }

        undo_move(board, pcurr, piece_taken, pawn_promoted);
        pcurr = pcurr->next;
    }

    if (ppmoves_path) {
        *ppmoves_path = best_full_path;
    } else {
        free_list(best_full_path);
    }

    if (table[ix].depth <= current_remaining_depth) {
        table[ix].key = key;
        table[ix].depth = current_remaining_depth;
        table[ix].pos_value = thresh;
        table[ix].pbest_moves = NULL;
    }

    free_list(pmoves);
    return thresh;
}

