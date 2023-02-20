#ifndef __ACCOUNT_MANAGER_H__
#define __ACCOUNT_MANAGER_H__

#include <stdio.h>

#define USERNAME_SIZE 50
#define PASSWORD_SIZE 10
#define FILENAME_SIZE 30
#define MAX_LENGTH 1024
#define MAX_NODE_LIST 100
#define MAX_MOVE 100
#define MAX_MATCH 5
#define FRIEND_COUNT 30
#define MAX_FRIEND 30
#define MAX_CONSECUTIVE_FAIL 3


typedef struct Match {
    char competitor_name[30];
    char state[10]; 
} Match;

//Cau truc mot account
typedef struct Account {    
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    char friends[MAX_FRIEND][USERNAME_SIZE];
    char friend_req[30][USERNAME_SIZE];
    char wait_add_friend[30][USERNAME_SIZE];
    // historyMatch history[5];
    int elo;
    int current_puzzle;
    int puzzle_point;
    int is_signed_in;
    int match_count;
    int win;
    int frie_count;
    int frie_req_count;
    int wait_add_friend_count;
    Match match[MAX_MATCH];
    struct Account *next;
} Account;

// Cấu trúc bảng xếp hạng người chơi theo elo hoặc puzzle point
typedef struct Ranking {
    char username[USERNAME_SIZE];
    int elo;
    int puzzle_point;
} Ranking;


typedef struct Account *node;  // Định nghĩa node

/**
* Hàm khởi tạo một node trong danh sách
*/
node create(char username[], char password[], int elo, int current_puzzle, int puzzle_point, int is_signed_in, int match_count, 
            int win, int frie_count, int frie_req_count, int wait_add_friend_count, char friend[][USERNAME_SIZE], char friend_req[][USERNAME_SIZE], char wait_add_friend[30][USERNAME_SIZE], Match match[MAX_MATCH]);
/**
* Thêm một node vào danh sách liên kết chứa thông tin tất cả các người chơi
*/
node addtail(node head, node temp);

/**
* Tìm thông tin người chơi trong danh sách liên kết theo username
*/
node search(node head, char username[]);

/**
* In thông tin tất cả bạn bè
*/
void printFriendList(node head);

/**
* Duyệt danh sách và in ra thông tin tất cả người chơi
*/
void printLists(node head);

void printHistoryMatch(node head);

/**
* Đọc file account.txt lấy thông tin username và password của tất cả người chơi lưu vào danh sách
* Đọc các file thông tin của từng người chơi lưu vào danh sách: player1.txt, player2.txt...
*/
void readFileAccount(node *head);

/**
* Cập nhật file account.txt khi change password
*/
void updateAccountFile(node head);

/**
* Cập nhật thông tin từng file người chơi khi dữ liệu thay đổi
* Cập nhật vào file lấy thông tin từ danh sách
*/
void addFileAccount(node head, char username[]);

/**
* Lấy số lượng account
*/
int getUserCount(node head);

#endif