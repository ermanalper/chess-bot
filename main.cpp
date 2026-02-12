#include <thread>
#include <array>
#include <iostream>
#include <stack>
#include <SDL.h>
#include <SDL_image.h>
#include <windows.h>
#include "hash.h"
#include "listnode.h"
#define MAX_DEPTH 5
#define TABLE_SIZE 1 << 22
#define TURN_WHITE true
#define TURN_BLACK false
#define WIDTH 1200
#define HEIGHT 900

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
SDL_Surface* pieceSurfaces[13];
SDL_Surface* loadImage(const char* path)
{
    SDL_Surface* loaded = IMG_Load(path);
    if (!loaded) {
        printf("IMG_Load error: %s\n", IMG_GetError());
    }
    return loaded;
}

void loadPieces()
{
    pieceSurfaces[-W_PAWN]   = loadImage("assets/white-pawn.png");
    pieceSurfaces[-B_PAWN]   = loadImage("assets/black-pawn.png");
    pieceSurfaces[-W_ROOK]   = loadImage("assets/white-rook.png");
    pieceSurfaces[-B_ROOK]   = loadImage("assets/black-rook.png");
    pieceSurfaces[-W_KNIGHT] = loadImage("assets/white-knight.png");
    pieceSurfaces[-B_KNIGHT] = loadImage("assets/black-knight.png");
    pieceSurfaces[-W_BISHOP] = loadImage("assets/white-bishop.png");
    pieceSurfaces[-B_BISHOP] = loadImage("assets/black-bishop.png");
    pieceSurfaces[-W_QUEEN]  = loadImage("assets/white-queen.png");
    pieceSurfaces[-B_QUEEN]  = loadImage("assets/black-queen.png");
    pieceSurfaces[-W_KING]   = loadImage("assets/white-king.png");
    pieceSurfaces[-B_KING]   = loadImage("assets/black-king.png");
}


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

void analyse_state(ListNode*& output, int board[8][8], int is_white_tempo, std::atomic<bool>& move_returned) {
    uint64_t key = hash_val(board, zobrist, is_white_tempo);
    uint32_t ix = map_table(key);
    if (table[ix].key == key && table[ix].depth >= MAX_DEPTH) {
        /*if (table[ix].pbest_moves) {
            display_moves(table[ix].pbest_moves);
        }*/
        output = table[ix].pbest_moves;
        return;
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
    output = best_path;
    move_returned = true; //finish dfs
    return;
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

void undo_move(int board[8][8], std::stack<std::array<int, 6>> &move_stack) {
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
}

std::array<int, 2> mapPxToSq(int px, int py)
{
    int boardSize = std::min(WIDTH, HEIGHT);
    int squareSize = boardSize / 8;

    int offsetX = (WIDTH  - boardSize) / 2;
    int offsetY = (HEIGHT - boardSize) / 2;

    if (px < offsetX || px >= offsetX + boardSize ||
        py < offsetY || py >= offsetY + boardSize)
    {
        return {-1, -1};
    }

    int x = (px - offsetX) / squareSize;
    int y = (py - offsetY) / squareSize;
    y = 7 - y;

    return {x, y};
}
std::array<int, 2> mapSqToPx(int sqX, int sqY) {
    //returns up-left px
    int boardSize = std::min(WIDTH, HEIGHT);
    int boardOffsetX = (WIDTH  - boardSize) / 2;
    int boardOffsetY = (HEIGHT - boardSize) / 2;
    int squareSize = boardSize / 8;
    int x = boardOffsetX + sqX * squareSize;
    int y = boardOffsetY + (7 - sqY) * squareSize;
    return {x, y};
}

void drawBoardOnSDL(int board[8][8], SDL_Surface* psurface)
{
    int boardSize = std::min(WIDTH, HEIGHT);
    int squareSize = boardSize / 8;

    int offsetX = (WIDTH  - boardSize) / 2;
    int offsetY = (HEIGHT - boardSize) / 2;

    Uint32 lightColor = SDL_MapRGB(psurface->format, 240, 217, 181);
    Uint32 darkColor  = SDL_MapRGB(psurface->format, 181, 136, 99);

    for (int row = 0; row < 8; ++row)
    {
        for (int col = 0; col < 8; ++col)
        {
            SDL_Rect rect;

            rect.x = offsetX + col * squareSize;
            rect.y = offsetY + row * squareSize;
            rect.w = squareSize;
            rect.h = squareSize;

            bool isLight = (row + col) % 2 == 0;

            SDL_FillRect(psurface, &rect, isLight ? lightColor : darkColor);
            auto piece = board[7 - row][col];
            if (piece != 0) {
                SDL_Surface* pieceImg = pieceSurfaces[-piece];

                SDL_Rect dst;
                dst.x = rect.x;
                dst.y = rect.y;
                dst.w = squareSize;
                dst.h = squareSize;

                SDL_BlitScaled(pieceImg, NULL, psurface, &dst);
            }
        }
    }
}

void highlightSq(int i, int j, SDL_Surface* psurface) {
    std::array<int, 2> px = mapSqToPx(i, j); //topleft sq
    int thickness = 4;
    int x = px[0]; int y = px[1];
    int boardSize = std::min(WIDTH, HEIGHT);
    int squareSize = boardSize / 8;


    Uint32 red = SDL_MapRGB(psurface->format, 255, 0, 0);


    SDL_Rect r1 = {x, y, squareSize, thickness};
    SDL_Rect r2 = {x, y + squareSize - thickness, squareSize, thickness};
    SDL_Rect r3 = {x, y, thickness, squareSize};
    SDL_Rect r4 = {x + squareSize - thickness, y, thickness, squareSize};

    SDL_FillRect(psurface, &r1, red);
    SDL_FillRect(psurface, &r2, red);
    SDL_FillRect(psurface, &r3, red);
    SDL_FillRect(psurface, &r4, red);
}

int main() {
    //engine thread
    std::thread engine_thread;

    //set SDL2
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: ", SDL_GetError();
        return 1;
    }
    SDL_Window* pwindow = SDL_CreateWindow(
        "Chess",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        0
    );
    if (!pwindow) {
        std::cerr << "SDL_Init Error: ", SDL_GetError();
        SDL_Quit();
        return 1;
    }
    SDL_Surface* psurface = SDL_GetWindowSurface(pwindow);
    SDL_UpdateWindowSurface(pwindow);
    //set sdl image
    IMG_Init(IMG_INIT_PNG);
    loadPieces();

    int board[8][8];
    initialize_board(board);
    init_zobrist(zobrist);
    std::stack<std::array<int, 6>> move_stack;
    ListNode* comp_move = nullptr;
    auto move_ctr = 0;
    auto turn = TURN_WHITE;
    auto piece_taken = 0;
    auto move_played = false;
    std::atomic<bool> move_returned(false);
    //draw_board(board);
    int from_row, from_col, to_row, to_col;
    int clicked_sq[2] = {-1, -1};
    bool dfsrunning = false;
    auto running = true;
    SDL_Event e;
    auto board_changed = true;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
                break;
            } else if (turn == TURN_WHITE && e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                //get mouse position
                int x, y; SDL_GetMouseState(&x, &y);
                std::array<int, 2> sq = mapPxToSq(x, y);
                drawBoardOnSDL(board, psurface); //reset prev click
                int piece = board[sq[1]][sq[0]];
                if (piece == 0 || ((piece & 1) == 0) || (sq[0] == clicked_sq[0] && sq[1] == clicked_sq[1])) {
                    //re-clicked same square or empty square or piece, don't highlight, so it will be unhighlighted
                    if (clicked_sq[0] != -1 && (clicked_sq[0] != sq[0] || clicked_sq[1] != sq[1])) {
                        //there was a piece selected before, move that piece to new clicked square
                        int moving_piece = board[clicked_sq[0]][clicked_sq[1]];
                        board[sq[1]][sq[0]] = moving_piece;
                        board[clicked_sq[0]][clicked_sq[1]] = 0;
                        board_changed = true;
                        turn = TURN_BLACK;
                    }
                    clicked_sq[0] = -1; clicked_sq[1] = -1;
                } else {
                    //highlight new clicked square
                    highlightSq(sq[0], sq[1], psurface);
                    clicked_sq[0] = sq[1]; clicked_sq[1] = sq[0];
                }
                SDL_UpdateWindowSurface(pwindow);

            }
        }

        if (turn == TURN_BLACK) {
            if (!dfsrunning) {
                dfsrunning = true;
                move_returned = false;
                int board_copy[8][8];
                memcpy(board_copy, board, sizeof(board));

                engine_thread = std::thread(analyse_state,
                                            std::ref(comp_move),
                                            board_copy,
                                            0,
                                            std::ref(move_returned));
                engine_thread.detach();

            }
            if (move_returned) {
                //play move
                from_row = comp_move->is[0]; from_col = comp_move->is[1];
                to_row = comp_move->ts[0]; to_col = comp_move->ts[1];
                auto piece = board[from_row][from_col];
                board[from_row][from_col] = 0;
                board[to_row][to_col] = piece;
                board_changed = true;
                turn = TURN_WHITE;
                dfsrunning = false;
            }
        }
        if (board_changed) {
            drawBoardOnSDL(board, psurface);
            board_changed = false; //changes applied
            SDL_UpdateWindowSurface(pwindow);

        }

    }

    SDL_DestroyWindow(pwindow);
    SDL_Quit();


    free_hashed_moves(table);
    return 0;
}
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    //set sdl image

    return main();
}