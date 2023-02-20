#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "account_manager.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MSG_SIZE 2048

//* Tín hiệu điều khiển
// chung
// server
#define LOGUP_SUCC 100 /* Đăng ký thành công */
#define LOGIN_SUCC 101 /* Đăng nhập thành công */
#define INCORRECT_ACC 102 /* Tài khoản hoặc mật khẩu không chính xác */
#define SIGNED_IN_ACC 103 /* Tài khoản đã được đăng nhập bởi người khác */
#define RECV_SUCC 104 /* Nhận dữ liệu thành công */
#define EXISTS_ACC 105 /* Tài khoản đã tồn tại */ 
#define CHANGE_PASS_SUCC 106 /* Đổi mật khẩu thành công*/
// client
#define LOGIN_REQ 201 /* Yêu cầu đăng nhập */
#define LOGUP_REQ 203 /* Yêu cầu đăng ký */
#define QUIT_REQ 202 /* Thoát ứng dụng */
#define CHANGE_PASS_REQ 204 /* Đổi mật khẩu */

#define MSG_SENT_SUCC 301
#define END_CHAT 302

#define SHOW_USER 401
#define PRIVATE_CHAT 402
#define CHAT_ALL 403
#define LOG_OUT 404

#define GROUP_CHAT_INIT 405
#define SHOW_GROUP 406
#define NEW_GROUP  407
#define MSG_MAKE_GROUP_SUCC 408
#define MSG_MAKE_GROUP_ERR 409
#define JOIN_GROUP 410
#define JOIN_GROUP_SUCC 411
#define HANDEL_GROUP_MESS 412
#define INVITE_FRIEND 413
#define INVITE_FRIEND_SUCC 414
#define GROUP_CHAT 415
#define GROUP_INFO 416
#define SHOW_GROUP_NAME 417
#define SHOW_GROUP_MEM 418
#define LEAVE_GROUP 419
#define LEAVE_GROUP_SUCC 420

#define VIEW_INFORMATION 501
#define VIEW_MATCH_HISTORY_FRIEND 502
#define VIEW_MATCH_HISTORY 503
#define SHOW_PLAY_PLAYER_MENU 504
#define CHOOSE_DIFFCULT 505
#define SHOW_CHESS_PUZZLE_MENU 506
#define SHOW_FRIEND_MENU 507
#define SHOW_MATCH_HISTORY_MENU 508
#define VIEW_RANKING 509
#define VIEW_CHESS_PUZZLE_RANKING 510
#define PLAY_CHESS_PUZZLE 511
#define CREATE_ROOM 512
#define CREATE_ROOM_SUCC 512
#define JOINT_ROOM 513
#define JOINT_ROOM_SUCC 514
#define VIEW_FRIEND 515
#define ADD_FRIEND 516
#define REMOVE_FRIEND 517
#define ADD_FRIEND_REQUEST 518
#define SENT_FRIEND_REQUEST_SUCC 519
#define ADD_FRIEND_SUCC 520
#define NOTIFY_NEW_FRIEND_REQ 521
#define START_GAME 522
#define END_GAME_WIN 523
#define END_GAME_LOSE 524
#define LEAVE_ROOM 525
#define LEAVE_ROOM_SUCC 526
#define PLAY_COMPUTER 527
#define CREATE_MATCH_WITH_COMPUTER 528
#define CREATE_MATCH_SUCC 529
#define PLAY_MOVE_SUCC 530
#define PLAY_MOVE 531
#define COMPUTER_EASY 1
#define COMPUTER_NORMAL 3
#define COMPUTER_HARD 7
#define LEAVE_COMPUTER_MATCH 532
#define LEAVE_COMPUTER_MATCH_SUCC 533
#define SHOW_FRIEND_REQUEST 534
#define SHOW_FRIEND_REQUEST_2 535
#define ACCEPT 536
#define NO_ACCEPT 537
#define SHOW_FRIEND_REQUEST_ERROR 538
#define FRIEND_REQUEST_SUCC 539
#define REMOVE_FRIEND_SUCC 540
#define CHECK_TURN_PUZZLE 541
#define CHECK_TURN_PUZZLE_SUCC 542
#define END_GAME_DRAW 543
#define WAIT_MOVE 544
#define PLAY_PLAYER_MOVE 545
#define OTHER_PLAYER_MOVE 546

#define FRIEND_CONFIRMATION 600
#define FRIEND_CONFIRMATION_1 601
#define FRIEND_CONFIRMATION_2 602
#define FRIEND_CONFIRMATION_3 603
#define FRIEND_CONFIRMATION_4 604
#define FRIEND_CONFIRMATION_5 605
#define FRIEND_CONFIRMATION_6 606
#define FRIEND_CONFIRMATION_7 607
#define FRIEND_CONFIRMATION_8 608
#define FRIEND_CONFIRMATION_9 609
#define FRIEND_CONFIRMATION_10 610
#define YES_TO_ADD_FRIEND 611
#define DELETE_REQUEST 612

//* Cấu trúc gói tin
typedef struct Package_ {
    char msg[MSG_SIZE]; /* nội dung thông điệp */
    char sender[USERNAME_SIZE]; /* username người gửi */
    char receiver[USERNAME_SIZE]; /* username người nhận */
    int group_id; /*id group muốn gửi*/
    int ctrl_signal; /* mã lệnh */
} Package;

#endif