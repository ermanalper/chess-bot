#ifndef CHESS_BOT_LISTNODE_H
typedef struct ListNode {
    struct ListNode* next;
    int is[2], ts[2]; //initial square [i, j]; terminal square[i, j]
} ListNode;

#define CHESS_BOT_LISTNODE_H

#endif //CHESS_BOT_LISTNODE_H