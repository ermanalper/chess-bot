#include <array>
#include <iostream>
#include <stack>

#include "hash.h"
#include "listnode.h"
#define MAX_DEPTH 5
#define TABLE_SIZE 1 << 22
#define TURN_WHITE true
#define TURN_BLACK false
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
std::string getPieceSymbol(int piece) {
    switch (piece) {
        case W_PAWN:   return " P "; case B_PAWN:   return " p ";
        case W_ROOK:   return " R "; case B_ROOK:   return " r ";
        case W_KNIGHT: return " N "; case B_KNIGHT: return " n ";
        case W_BISHOP: return " B "; case B_BISHOP: return " b ";
        case W_QUEEN:  return " Q "; case B_QUEEN:  return " q ";
        case W_KING:   return " K "; case B_KING:   return " k ";
        default:       return "   ";
    }
}
void draw_board(int board[8][8]) {
    for (int i = 7; i >= 0; --i) {
        for (int j = 0; j < 8; ++j) {
            std::cout << getPieceSymbol(board[i][j]);
        }
        std::cout << "" << std::endl;

    }
}

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

std::array<std::array<int, 2>, 2> get_player_move(int board[8][8]) {
    auto board_disp = false;
    std::array<std::array<int, 2>, 2> move{};
    std::string input;

    while (true) {
        board_disp = false;
        std::cout << "What's your move?: ";
        std::getline(std::cin, input);
        if (input == "--undo") {
            return {{{-1, -1}, {-1, -1}}}; //sentinel, indicator to undo move
        } else if (input == "--board") {
            draw_board(board);
            board_disp = true;
        }
        if (sscanf(input.c_str(), "%d %d : %d %d",
                   &move[0][0], &move[0][1],
                   &move[1][0], &move[1][1]) == 4) {
            return move;
                   } else if (!board_disp) {
                       std::cout << "Please rewrite your move in this form: 0 1 : 0 2 (the piece's cordinate you want to play, final coordinate)" << std::endl;
                   }
    }
}

int main() {
    int board[8][8];
    //initialize_board(board);
    for (auto i = 0; i < 8; i++) {
        for (auto j = 0; j < 8; j++) {
            board[i][j] = 0;
        }
    }
    board[0][0] = W_KING;
    board[7][0] = B_KING;
    board[5][7] = W_PAWN;
    init_zobrist(zobrist);
    std::stack<std::array<int, 6>> move_stack;
    ListNode* comp_move = nullptr;
    auto move_ctr = 0;
    auto turn = TURN_WHITE;
    auto piece_taken = 0;
    auto move_played = false;

    draw_board(board);
    int from_row, from_col, to_row, to_col, piece;
    do {
        auto pawn_promoted = false;
        auto disp_move = (int) (move_ctr / 2) + 1;

        piece_taken = 0;
        move_played = false;
        if (turn == TURN_WHITE) {
            std::array<std::array<int, 2>, 2> player_moves = get_player_move(board);
            from_row = player_moves[0][0];
            from_col = player_moves[0][1];
            to_row   = player_moves[1][0];
            to_col   = player_moves[1][1];
            if (from_col == -1 && move_stack.size() >= 2) {
                //undo from stack
                for (auto i = 0; i < 2; i++) {
                    std::array<int, 6> past_move = move_stack.top(); move_stack.pop();
                    auto piece_taken = past_move[4];
                    auto past_from_row = past_move[0]; auto past_from_col = past_move[1];
                    auto past_to_row = past_move[2]; auto past_to_col = past_move[3];
                    auto played_piece = board[past_to_row][past_to_col];
                    auto promotion_info = past_move[5];
                    if (promotion_info == 1) {
                        played_piece = W_PAWN;
                    } else if (promotion_info == -1) {
                        played_piece = B_PAWN;
                    }
                    board[past_to_row][past_to_col] = piece_taken;
                    board[past_from_row][past_from_col] = played_piece;
                }
            } else {
                //play move
                std::cout << "Excellent move! Now it's my turn to think! " << "\n";
                piece = board[from_row][from_col];
                piece_taken = board[to_row][to_col];
                board[from_row][from_col] = 0;
                if (piece == W_PAWN && to_row == 7) {
                    piece = W_QUEEN;
                    pawn_promoted = true;
                    std::cout << "You've promoted a pawn to queen! " << "\n";

                }
                board[to_row][to_col] = piece;
                move_played = true;
            }


        } else if (turn == TURN_BLACK) {
            pawn_promoted = false;
            comp_move = analyse_state(board, 0);
            from_row = comp_move->is[0]; from_col = comp_move->is[1];
            to_row = comp_move->ts[0]; to_col = comp_move->ts[1];
            piece_taken = board[to_row][to_col];
            piece = board[from_row][from_col];

            board[from_row][from_col] = 0;
            board[to_row][to_col] = piece;
            if (piece == B_PAWN && to_row == 0) {
                board[to_row][to_col] = B_QUEEN;
                pawn_promoted = true;
            }
            std::cout << disp_move << ". I move " << from_row << " " << from_col
                      << " to " << to_row << " " << to_col;
            if (pawn_promoted) {
                std::cout << " = Q ";
            }
            std::cout<<"\n";
            move_played = true;
        }
        if (move_played) {
            auto promotion_info = 0;
            if (pawn_promoted) {
                promotion_info = -1; //black promoted
                if (turn == TURN_WHITE) {
                    promotion_info = 1; //white promoted
                }
            }
            move_stack.push({from_row, from_col, to_row, to_col, piece_taken, promotion_info});
            turn = !turn;
            move_ctr += 1;
        }




    } while (move_ctr == 1  || (comp_move != nullptr && comp_move->next != nullptr));



    free_hashed_moves(table);
    return 0;
}
