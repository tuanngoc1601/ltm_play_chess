#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "server.h"
#include "network.h"
#include "error.h"
#include "util.h"

/**
 * Menu đăng nhập
*/
void LoginMenu();

/**
  Giao diện menu chính
*/
void MainMenu();


/**
 Chọn độ khó của máy
*/
void ChooseDifficultyComputer();

/**
 Menu chọn màu quân
*/
void ChooseColorMenu();

/**
 Giao diện chơi với người
*/
void PlayWithOtherPlayerMenu();

/**
 Giao diện giải thế cờ
*/
void ChessPuzzleMenu();

/**
 Menu bạn bè
*/
void FriendMenu();

/**
 Menu lịch sử đấu
*/
void MatchHistoryMenu();

/**
 Giải thế cờ
*/
void ChessPuzzle(int client_socket);

void ChessPuzzleTurn(int client_socket);

/**
 Xem bảng xếp hạng theo elo
*/
void ViewChessRank(int client_socket);

/**
 Xem bảng xếp hạng theo điểm chess puzzle
*/
void ViewChessPuzzleRank(int client_socket);

/**
 * Đổi mật khẩu
 */
void ChangePassword(int client_socket);

/**
 * Đăng nhập với tên đăng nhập và mật khẩu
 * @param client_socket socket đã kết nối đến server
 * @return 0: đăng nhập thất bại
 * @return 1: đăng nhập thành công
*/
int login(int client_socket);

//* Chức năng sau đăng nhập
/**
 * Cho người dùng nhập lựa chọn, thực hiện chức năng tương ứng
 * Phân luồng đọc-ghi
 * @param client_socket socket đã kết nối đến server
*/
void user_use(int client_socket);

/**
 * Đọc nội dung tin nhắn
 * @param param socket kết nối đến server
*/

/**
 * Xem thông tin cá nhân 
*/
void ViewInformation(int client_socket);

//* Chức năng sau đăng nhập
/**
 * Cho người dùng nhập lựa chọn, thực hiện chức năng tương ứng 
 * @param client_socket socket đã kết nối đến server
*/

/**
* Hiển thị các chức năng khi chơi với máy
*/
void ShowPlayComputer(int client_socket);

/**
* Hiển thị chức năng khi chơi với người chơi khác
*/
void ShowPlayPlayer(int client_socket);

/**
* Hiển thị các chức năng giải thế cờ
*/
void ShowChessPuzzle(int client_socket);

/**
* Hiển thị chức năng friend
*/
void ShowFriendMenu(int client_socket);

/**
* Hiển thị các chức năng xem lịch sử đấu
*/
void ShowMatchHistoryMenu(int client_socket);

/**
* Chọn màu quân cờ
*/
void ShowChooseColor(int client_socket, int diffcult);

/**
* Xử lý chơi với máy tính
*/
void PlayWithComputer(int client_socket, int diffcult, int color);

/**
* Đang chơi với máy
*/
void InRoomWithComputer(int client_socket);

/**
* Rời phòng chơi với máy
*/
void LeavePlayComputer(int client_socket);

/**
* Tạo phòng chơi
*/
void CreateRoom(int client_socket);

/**
* Tham gia vào phòng chơi
*/
void JointRoom(int client_socket);

/**
* Rời khỏi phòng chơi
*/
void LeaveRoom(int client_socket);

/**
* Xem danh sách người chơi đã kết bạn
*/
void ViewFriend(int client_socket);

/**
* Chức năng gửi lời mời kết bạn
*/
void AddFriend(int client_socket);

/**
* Xóa kết bạn
*/
void RemoveFriend(int client_socket);

/**
* Phản hồi yêu cầu kết bạn từ người chơi khác
*/
void ReplyRequestFriend(int client_socket);    

/**
* Gửi danh sách người chơi yêu cầu kết bạn
*/
void ReplyRequestFriend2(int client_socket);    

/**
* Đang chơi với người
*/
void InRoom(int client_socket);

/**
* Hướng dẫn người chơi sử dụng các lệnh để chơi
*/
void RoomTutorial();

void ViewMatchHistory(int client_socket);

/**
* Nhận các message gửi từ server, in ra kết quả
*/
void *read_msg(void *param);

#endif

