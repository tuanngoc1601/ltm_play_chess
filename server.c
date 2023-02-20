#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

Active_user user[MAX_USER];
Group group[MAX_GROUP];
Room room[MAX_ROOM];
Puzzle puzzle[15];
node acc_list;
Ranking rank[MAX_NODE_LIST];
Puzzle puzzle_list[20];

S_BOARD pos[30];
S_SEARCHINFO info[30];
int movetime = 3000;
int play_with_computer[30];
int engineSide[30];
int depth[30];

int create_listen_socket()
{

    int listen_socket;
    struct sockaddr_in server_addr;

    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        report_err(ERR_SOCKET_INIT);
        exit(0);
    }

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        report_err(ERR_SOCKET_INIT);
        exit(0);
    }

    if (listen(listen_socket, MAX_USER) < 0)
    {
        report_err(ERR_SOCKET_INIT);
        exit(0);
    }

    return listen_socket;
}

int accept_conn(int listen_socket)
{

    int conn_socket;
    struct sockaddr_in client_addr;
    int client_addr_size = sizeof(struct sockaddr);

    if ((conn_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &client_addr_size)) < 0)
    {
        report_err(ERR_CONN_ACCEPT);
        exit(0);
    }

    return conn_socket;
}

void make_server()
{
    int listen_socket;

    readFileAccount(&acc_list);
    readFileChessPuzzle();
    printHistoryMatch(acc_list);
    // printPuzzle();
    // printFriendList(acc_list);
    // printLists(acc_list);
    listen_socket = create_listen_socket();
    for (int i = 0; i < MAX_USER; i++)
    {
        user[i].socket = -1;
        // 17/01/2023
        for (int j = 0; j < MAX_GROUP; j++)
            user[i].room_id = -1;
    }
    for (int i = 0; i < MAX_ROOM; i++)
    {
        for (int j = 0; j < MAX_USER; j++)
        {
            room[i].member[j].socket = -1;
        }
        room[i].curr_num = 0;
    }

    for (int i = 0; i < 30; i++)
    {
        play_with_computer[i] = -1;
    }

    printf("Server created\n");
    AllInit();

    while (1)
    {

        int conn_socket = accept_conn(listen_socket);

        pthread_t client_thr;
        if (pthread_create(&client_thr, NULL, pre_login_srv, (void *)&conn_socket) < 0)
        {
            report_err(ERR_CREATE_THREAD);
            exit(0);
        }
        pthread_detach(client_thr);
    }

    close(listen_socket);
}

void *pre_login_srv(void *param)
{

    int conn_socket = *((int *)param);
    Package pkg;

    while (1)
    {

        recv(conn_socket, &pkg, sizeof(pkg), 0);

        switch (pkg.ctrl_signal)
        {
        case LOGIN_REQ:
            handle_login(conn_socket, acc_list);
            break;
        case LOGUP_REQ:
            handle_signup(conn_socket, acc_list);
            break;
        case QUIT_REQ:
            close(conn_socket);
            printf("user quit\n");
            pthread_exit(NULL);
        }
    }
}

void handle_signup(int conn_socket, node acc_list)
{
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    Package pkg;
    node target_account;
    int result;

    recv(conn_socket, &pkg, sizeof(pkg), 0);
    strcpy(username, pkg.msg);

    pkg.ctrl_signal = RECV_SUCC;
    send(conn_socket, &pkg, sizeof(pkg), 0);

    recv(conn_socket, &pkg, sizeof(pkg), 0);
    strcpy(password, pkg.msg);

    printf("%s\n", username);
    printf("%s\n", password);

    target_account = search(acc_list, username);
    if (target_account)
        result = EXISTS_ACC;
    else
        result = LOGUP_SUCC;

    if (result == LOGUP_SUCC)
    {
        // printf("Your account is registed successfully\n");
        for (int i = 0; i < MAX_USER; i++)
        {
            if (user[i].socket < 0)
            {
                strcpy(user[i].username, username);
                user[i].socket = conn_socket;
                break;
            }
        }
    }
    else
    {
        printf("This account has been existed!\n");
    }

    pkg.ctrl_signal = result;
    send(conn_socket, &pkg, sizeof(pkg), 0);
    if (result == LOGUP_SUCC)
    {
        char friends[MAX_FRIEND][USERNAME_SIZE];
        char friend_req[30][USERNAME_SIZE];
        char wait_add_friend[30][USERNAME_SIZE];
        Match match[MAX_MATCH];
        node temp = create(username, password, 1000, 1, 0, 0, 0, 0, 0, 0, 0, friends, friend_req, wait_add_friend, match);
        acc_list = addtail(acc_list, temp);
        updateAccountFile(acc_list);
        addFileAccount(acc_list, username);
    }
}

void handle_login(int conn_socket, node acc_list)
{

    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    Package pkg;
    node target_acc;
    int result;

    recv(conn_socket, &pkg, sizeof(pkg), 0);
    strcpy(username, pkg.msg);

    pkg.ctrl_signal = RECV_SUCC;
    send(conn_socket, &pkg, sizeof(pkg), 0);

    recv(conn_socket, &pkg, sizeof(pkg), 0);
    strcpy(password, pkg.msg);

    printf("%s\n", username);
    printf("%s\n", password);

    target_acc = search(acc_list, username);
    if (target_acc)
    {
        if (target_acc->is_signed_in)
        {
            result = SIGNED_IN_ACC;
        }
        else
        {
            if (strcmp(target_acc->password, password) == 0)
            {
                result = LOGIN_SUCC;
            }
            else
            {
                result = INCORRECT_ACC;
            }
        }
    }
    else
    {
        result = INCORRECT_ACC;
    }

    if (result == LOGIN_SUCC)
    {
        printf("login success\n");
        target_acc->is_signed_in = 1;
        // updateAccountFile(acc_list);

        for (int i = 0; i < MAX_USER; i++)
        {
            if (user[i].socket < 0)
            {
                strcpy(user[i].username, username);
                user[i].socket = conn_socket;
                sv_update_port_group(&user[i], group);
                break;
            }
        }
    }
    else if (result == SIGNED_IN_ACC)
    {
        printf("already signed in acc\n");
    }
    else
    {
        printf("incorrect acc\n");
    }

    pkg.ctrl_signal = result;
    send(conn_socket, &pkg, sizeof(pkg), 0);
    if (result == LOGIN_SUCC)
    {
        node temp = getAccountBySocket(conn_socket);
        CheckFriendRequest(temp, conn_socket);
        sv_user_use(conn_socket);
    }
}

void sv_user_use(int conn_socket)
{

    Package pkg;
    int login = 1;
    while (login)
    {
        if (recv(conn_socket, &pkg, sizeof(pkg), 0) > 0) // printf("Receive from %d\n", conn_socket);
            printf("%d chooses %d \n", conn_socket, pkg.ctrl_signal);
        switch (pkg.ctrl_signal)
        {        

        case CHOOSE_DIFFCULT:
            ChooseDiffcultServer(conn_socket, &pkg);
            break;

        case SHOW_PLAY_PLAYER_MENU:
            ShowPlayWithPlayer(conn_socket, &pkg);
            break;

        case SHOW_CHESS_PUZZLE_MENU:
            ShowChessPuzzleServer(conn_socket, &pkg);
            break;

        case SHOW_FRIEND_MENU:
            ShowFriendServer(conn_socket, &pkg);
            break;

        case SHOW_MATCH_HISTORY_MENU:
            ShowMatchHistoryServer(conn_socket, &pkg);
            break;
        case VIEW_MATCH_HISTORY:
            printf("History match:\n");
            ViewMatchHistoryServer(conn_socket, &pkg);
            break;
        case VIEW_INFORMATION:
            printf("View information\n");
            ViewInformationServer(conn_socket, &pkg);
            break;
        case VIEW_CHESS_PUZZLE_RANKING:
            ViewChessPuzzleRankServer(conn_socket, &pkg);
            break;
        case PLAY_CHESS_PUZZLE:
            ChessPuzzleServer(conn_socket, &pkg);
            break;
        case CHECK_TURN_PUZZLE:
            ChessPuzzleTurnServer(conn_socket, &pkg);
            break;
        case VIEW_RANKING:
            ViewChessRankServer(conn_socket, &pkg);
            break;
        case CHANGE_PASS_REQ:
            ChangePassServer(conn_socket, &pkg);
            break;
        case CREATE_ROOM:
            CreateRoomServer(conn_socket, &pkg);
            break;        
        case LEAVE_ROOM:
            LeaveRoomServer(conn_socket, &pkg);
            break;
        case LEAVE_COMPUTER_MATCH:
            LeavePlayComputerServer(conn_socket, &pkg);
            break;
        case CREATE_MATCH_WITH_COMPUTER:
            CreateMatchWithPlayer(conn_socket, &pkg);
            break;
        case PLAY_MOVE:
            PlayWithPlayer(conn_socket, &pkg);
            break;
        case OTHER_PLAYER_MOVE:
            PlayWithOtherPlayer(conn_socket, &pkg);
            break;
        case LOG_OUT:
            login = 0;
            sv_logout(conn_socket, &pkg);
            break;       
        case JOINT_ROOM:
            JointRoomServer(conn_socket, &pkg);
            break;
        case VIEW_FRIEND:
            ViewFriendServer(conn_socket, &pkg);
            break;
        case ADD_FRIEND:
            AddFriendServer(conn_socket, &pkg);
            break;
        case REMOVE_FRIEND:
            RemoveFriendServer(conn_socket, &pkg);
            break;
        case SHOW_FRIEND_REQUEST:
            ReplyFriendServer(conn_socket, &pkg);
            break;
        case ACCEPT:
            AcceptFriend(conn_socket, &pkg);
            break;
        case NO_ACCEPT:
            NotAcceptFriend(conn_socket, &pkg);
            break;        
        default:
            break;
        }
        printf("Done %d of %d\n", pkg.ctrl_signal, conn_socket);
    }

    for (int i = 0; i < MAX_USER; i++)
    {
        if (user[i].socket == conn_socket)
        {
            node target_acc = search(acc_list, user[i].username);
            target_acc->is_signed_in = 0;
            user[i].socket = -1;
            // updateAccountFile(acc_list);
            for (int j = 0; j < MAX_ROOM; j++)
            {
                if (user[i].group_id[j] >= 0)
                {
                    int group_id = user[i].group_id[j];
                    int user_id_group = sv_search_id_user_group(group[group_id], user[i].username);
                    if (user_id_group >= 0)
                    {
                        // printf("1\n");
                        // printf("%d %d\n", group_id, user_id_group);
                        group[group_id].group_member[user_id_group].socket = 0; // can cap nhat khi dang nhap lai
                    }
                    user[i].group_id[j] = -1;
                }
            }
            break;
        }
    }
}


int search_user(int conn_socket)
{
    int i = 0;
    for (i = 0; i < MAX_USER; i++)
    {
        if (user[i].socket == conn_socket)
            return i;
    }
    return -1;
}

int sv_search_id_user_group(Group group, char *user_name)
{
    int i = 0;
    for (i = 0; i < MAX_USER; i++)
    {
        if (strcmp(group.group_member[i].username, user_name) == 0)
        {            
            return i;
        }
    }
    return -1;
}

int sv_add_group_user(Active_user *user, int group_id)
{
    for (int i = 0; i < MAX_GROUP; i++)
    {
        if (user->group_id[i] < 0)
        {
            user->group_id[i] = group_id;
            return 1;
        }
    }
    return 0;
}


void sv_update_port_group(Active_user *user, Group *group)
{
    int i = 0;
    int user_id_port;
    for (i = 0; i < MAX_GROUP; i++)
    {
        user_id_port = sv_search_id_user_group(group[i], user->username);
        if (user_id_port >= 0)
        {
            sv_add_group_user(user, i);
            group[i].group_member[user_id_port].socket = user->socket;
        }
    }
}

void sv_logout(int conn_socket, Package *pkg)
{
    printf("%d logout\n", conn_socket);
    pkg->ctrl_signal = LOG_OUT;
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void CheckFriendRequest(node user, int conn_socket)
{
    Package pkg;
    pkg.ctrl_signal = NOTIFY_NEW_FRIEND_REQ;
    printf("f: %d\n", user->frie_req_count);
    if (user->frie_req_count > 0)
    {
        for (int i = 0; i < user->frie_req_count; i++)
        {
            strcpy(pkg.msg, "You have add friend request from ");
            strcat(pkg.msg, user->friend_req[i]);
            send(conn_socket, &pkg, sizeof(pkg), 0);
        }
    }
}

void ViewInformationServer(int conn_socket, Package *pkg)
{
    // Ngoc
    int user_index = search_user(conn_socket);
    // printf("%s\n", user[user_index].username);
    char user_name_active[USERNAME_SIZE];
    strcpy(user_name_active, user[user_index].username);
    node temp = search(acc_list, user_name_active);
    char elo_string[5];
    char current_puzzle_string[5];
    char puzzle_point_string[5];
    char match_count_string[5];
    char win_string[5];
    sprintf(elo_string, "%d", temp->elo);
    sprintf(current_puzzle_string, "%d", temp->current_puzzle);
    sprintf(puzzle_point_string, "%d", temp->puzzle_point);
    sprintf(match_count_string, "%d", temp->match_count);
    sprintf(win_string, "%d", temp->win);
    char information[MAX_LENGTH];
    // user_name_active + "\n" + "elo: " + elo + "\n" + "current_puzzle: " + current_puzzle + "\n" + "puzzle_point: " + puzzle_point + "\n" + "match_count: " + match_count + "\n" + "win: " + win + "\n";
    strcpy(information, "username: ");
    strcat(information, user_name_active);
    strcat(information, "\n");
    strcat(information, "elo: ");
    strcat(information, elo_string);
    strcat(information, "\n");
    strcat(information, "current_puzzle: ");
    strcat(information, current_puzzle_string);
    strcat(information, "\n");
    strcat(information, "puzzle_point: ");
    strcat(information, puzzle_point_string);
    strcat(information, "\n");
    strcat(information, "match_count: ");
    strcat(information, match_count_string);
    strcat(information, "\n");
    strcat(information, "win: ");
    strcat(information, win_string);
    strcat(information, "\n");
    strcpy(pkg->msg, information);
    strcpy(information, "");
    pkg->ctrl_signal = VIEW_INFORMATION;
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void readFileChessPuzzle()
{
    char levelPuzzle[3];
    char filePuzzle[FILENAME_SIZE];
    char line[MAX_LENGTH];
    for (int i = 0; i < 10; i++)
    {
        sprintf(levelPuzzle, "%d", i + 1);
        strcpy(filePuzzle, "./puzzle/");
        strcat(filePuzzle, levelPuzzle);
        strcat(filePuzzle, ".txt");
        FILE *file = fopen(filePuzzle, "r");
        if (file == NULL)
        {
            printf("no such file.");
            return;
        }
        int j = 0;
        while (fgets(line, sizeof(line), file) != NULL)
        {
            if (strstr(line, "8") || strstr(line, "7") || strstr(line, "6") || strstr(line, "5") || strstr(line, "4") || strstr(line, "3") ||
                strstr(line, "2") || strstr(line, "1"))
            {
                strcpy(puzzle_list[i].board[j++], line);
            }
            if (strstr(line, "0"))
            {
                line[0] = ' ';
                strcpy(puzzle_list[i].board[j++], line);
            }
            if (strstr(line, "MOVE"))
            {
                char *p = (char *)malloc(100 * sizeof(char));
                p = strtok(line, " ");
                while (p != NULL)
                {
                    p = strtok(NULL, " ");
                    if (p != NULL)
                    {
                        p[strlen(p)] = '\0';
                        if (p[strlen(p) - 1] == '\n')
                        {
                            p[strlen(p) - 1] = '\0';
                        }
                        strcpy(puzzle_list[i].move, p);
                    }
                }
            }
        }
        fclose(file);
    }
}

void printPuzzle()
{
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            printf("%s", puzzle_list[i].board[j]);
        }
        printf("MOVE: %s\n", puzzle_list[i].move);
    }
}

void ChessPuzzleServer(int conn_socket, Package *pkg)
{
    // Thai
    node user_name = search(acc_list, pkg->sender);
    strcpy(pkg->msg, "Your level : ");
    pkg->msg[strlen(pkg->msg)] = user_name->current_puzzle + 49;
    // strcat(pkg->msg,user_name->current_puzzle);
    strcat(pkg->msg, "\n");
    for (int j = 0; j < 9; j++)
    {
        strcat(pkg->msg, puzzle_list[user_name->current_puzzle].board[j]);
        // strcat(pkg->msg,"\n");
    }
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void ChessPuzzleTurnServer(int conn_socket, Package *pkg)
{
    node user_name = search(acc_list, pkg->sender);
    if (strcmp(pkg->msg, puzzle_list[user_name->current_puzzle].move) == 0)
    {
        strcpy(pkg->msg, "Congratulations, You win...!");
        user_name->puzzle_point = user_name->puzzle_point + user_name->current_puzzle;
        user_name->current_puzzle++;
        addFileAccount(acc_list, user_name->username);
    }
    else
    {
        strcpy(pkg->msg, "Wrong !! You lost..!");
        send(conn_socket, pkg, sizeof(*pkg), 0);
    }
    pkg->ctrl_signal = CHECK_TURN_PUZZLE_SUCC;
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void ViewMatchHistoryServer(int conn_socket, Package *pkg) {
    node temp = getAccountBySocket(conn_socket);
    char history_match[MAX_LENGTH];
    char num[2];
    if(temp->match_count <= 5 && temp->match_count > 0) {
        for(int i = 0; i < temp->match_count; i++) {
            char num_match[10];
            strcpy(num_match, "MATCH_");
            sprintf(num, "%d", i+1);
            strcat(num_match, num);
            strcat(history_match, num_match);
            strcat(history_match, " ");
            strcat(history_match, temp->match[i].competitor_name);
            strcat(history_match, " ");
            strcat(history_match, temp->match[i].state);
            strcat(history_match, "\n");
        }
    } else {
        for(int i = 0; i < 5; i++) {
            char num_match[10];
            strcpy(num_match, "MATCH_");
            sprintf(num, "%d", i+1);
            strcat(num_match, num);
            strcat(history_match, num_match);
            strcat(history_match, " ");
            strcat(history_match, temp->match[i].competitor_name);
            strcat(history_match, " ");
            strcat(history_match, temp->match[i].state);
            strcat(history_match, "\n");
        }
    }
    printf("%s\n", history_match);
    strcpy(pkg->msg, history_match);
    strcpy(history_match, "");
    pkg->ctrl_signal = VIEW_MATCH_HISTORY;
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void ViewChessRankServer(int conn_socket, Package *pkg){
    //Ngoc
    char elo_string[6];
    char chessRank_string[MAX_LENGTH];
    getListUserRanking(acc_list);
    sortUserRanking(0);
    for (int i = 0; i < getUserCount(acc_list); i++)
    {
        sprintf(elo_string, "%d", rank[i].elo);
        strcat(chessRank_string, rank[i].username);
        strcat(chessRank_string, " ");
        strcat(chessRank_string, elo_string);
        strcat(chessRank_string, "\n");
    }
    strcpy(pkg->msg, chessRank_string);
    strcpy(chessRank_string, "");
    pkg->ctrl_signal = VIEW_RANKING;
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void ViewChessPuzzleRankServer(int conn_socket, Package *pkg)
{
    // Ngoc
    char puzzle_point_string[5];
    char chessPuzzleRanking[MAX_LENGTH];
    getListUserRanking(acc_list);
    sortUserRanking(1);
    for (int i = 0; i < getUserCount(acc_list); i++)
    {
        sprintf(puzzle_point_string, "%d", rank[i].puzzle_point);
        strcat(chessPuzzleRanking, rank[i].username);
        strcat(chessPuzzleRanking, " ");
        strcat(chessPuzzleRanking, puzzle_point_string);
        strcat(chessPuzzleRanking, "\n");
    }
    strcpy(pkg->msg, chessPuzzleRanking);
    strcpy(chessPuzzleRanking, "");
    pkg->ctrl_signal = VIEW_CHESS_PUZZLE_RANKING;
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

node getAccountBySocket(int conn_socket)
{
    node temp;
    int user_index = search_user(conn_socket);
    char user_name_active[USERNAME_SIZE];
    strcpy(user_name_active, user[user_index].username);
    temp = search(acc_list, user_name_active);
    return temp;
}

node searchRecommendFriendByElo(int conn_socket, Package *pkg)
{
    node temp = getAccountBySocket(conn_socket);
    Ranking compareElo[100];
    Ranking tmp;
    int count = 0;
    getListUserRanking(acc_list);
    sortUserRanking(0);
    for (int i = 0; i < getUserCount(acc_list); i++)
    {
        strcpy(compareElo[i].username, rank[i].username);
        compareElo[i].elo = abs(rank[i].elo - temp->elo);
    }
    for (int i = 0; i < getUserCount(acc_list) - 1; i++)
    {
        for (int j = i + 1; j < getUserCount(acc_list); j++)
        {
            if (compareElo[i].elo > compareElo[j].elo)
            {
                tmp = compareElo[i];
                compareElo[i] = compareElo[j];
                compareElo[j] = tmp;
            }
        }
    }
    while (count < 5)
    {
        if (strcmp(temp->username, compareElo[count].username) == 0)
        {
            continue;
        }
        strcat(pkg->msg, compareElo[count].username);
        strcat(pkg->msg, "\n");
        count++;
    }
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void ChangePassServer(int conn_socket, Package *pkg)
{
    // Ngoc
    node temp = getAccountBySocket(conn_socket);
    strcpy(temp->password, pkg->msg);
    updateAccountFile(acc_list);
    pkg->ctrl_signal = CHANGE_PASS_SUCC;
    strcpy(pkg->msg, "Update password successfully\n");
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void getListUserRanking(node head)
{
    node p = head;
    int i = 0;
    while (p != NULL)
    {
        strcpy(rank[i].username, p->username);
        rank[i].elo = p->elo;
        rank[i].puzzle_point = p->puzzle_point;
        i++;
        p = p->next;
    }
}

void sortUserRanking(int type)
{
    Ranking temp;
    switch (type)
    {
    case 0:
        for (int i = 0; i < getUserCount(acc_list) - 1; i++)
        {
            for (int j = i + 1; j < getUserCount(acc_list); j++)
            {
                if (rank[i].elo < rank[j].elo)
                {
                    temp = rank[i];
                    rank[i] = rank[j];
                    rank[j] = temp;
                }
            }
        }
        break;
    case 1:
        for (int i = 0; i < getUserCount(acc_list) - 1; i++)
        {
            for (int j = i + 1; j < getUserCount(acc_list); j++)
            {
                if (rank[i].puzzle_point < rank[j].puzzle_point)
                {
                    temp = rank[i];
                    rank[i] = rank[j];
                    rank[j] = temp;
                }
            }
        }
        break;
    default:
        break;
    }
}

void ChooseDiffcultServer(int conn_socket, Package *pkg)
{
    strcpy(pkg->msg, "Diffcult\n");
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void ShowPlayWithPlayer(int conn_socket, Package *pkg)
{
    strcpy(pkg->msg, "Player\n");
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void ShowChessPuzzleServer(int conn_socket, Package *pkg)
{
    strcpy(pkg->msg, "Chess Puzzle\n");
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void ShowFriendServer(int conn_socket, Package *pkg)
{
    strcpy(pkg->msg, "Friend\n");
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void ShowMatchHistoryServer(int conn_socket, Package *pkg)
{
    strcpy(pkg->msg, "Match\n");
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

int SearchRoomPlayComputer()
{
    for (int i = 0; i < 30; i++)
    {
        if (play_with_computer[i] < 0)
        {
            play_with_computer[i] = 1;
            return i;
        }
    }
    return -1;
}

void CreateMatchWithPlayer(int conn_socket, Package *pkg)
{

    int difficult;
    int color;
    sscanf(pkg->msg, "%d %d", &difficult, &color);

    int user_id = search_user(conn_socket);
    user[user_id].computer_id = SearchRoomPlayComputer();
    int current_id = user[user_id].computer_id;

    CreateNewBoard(current_id);
    info[current_id].GAME_MODE = CONSOLEMODE;
    info[current_id].POST_THINKING = TRUE;

    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    printf("Create Match Complete\n");

    engineSide[current_id] = !color;
    depth[current_id] = difficult;
    ParseFen(START_FEN, &pos[current_id]);
    pkg->ctrl_signal = CREATE_MATCH_SUCC;
    strcpy(pkg->msg, PrintBoard(&pos[current_id]));
    send(conn_socket, pkg, sizeof(*pkg), 0);

    if (pos[current_id].side == engineSide[current_id])
    {
        info[current_id].starttime = GetTimeMs();
        info[current_id].depth = depth[current_id];

        if (movetime != 0)
        {
            info[current_id].timeset = TRUE;
            info[current_id].stoptime = info[current_id].starttime + movetime;
        }
        strcpy(pkg->msg, SearchPosition(&pos[current_id], &info[current_id]));
        pkg->ctrl_signal = PLAY_MOVE_SUCC;
        send(conn_socket, pkg, sizeof(*pkg), 0);
    }
}

void PlayWithPlayer(int conn_socket, Package *pkg)
{
    char inBuf[80], command[80];
    int move = NOMOVE;
    int result;
    strcpy(inBuf, pkg->msg);
    node user_account = getAccountBySocket(conn_socket);

    int user_id = search_user(conn_socket);
    int current_id = user[user_id].computer_id;

    move = ParseMove(inBuf, &pos[current_id]);
    if (move == NOMOVE)
    {
        pkg->ctrl_signal = ERR_MOVE;
        send(conn_socket, pkg, sizeof(*pkg), 0);
        // printf("Command unknown:%s\n",inBuf);
        return;
    }
    MakeMove(&pos[current_id], move);
    pos[current_id].ply = 0;

    if ((checkresult(&pos[current_id]) == FALSE))
    {
        info[current_id].starttime = GetTimeMs();
        info[current_id].depth = depth[current_id];

        if (movetime != 0)
        {
            info[current_id].timeset = TRUE;
            info[current_id].stoptime = info[current_id].starttime + movetime;
        }
        strcpy(pkg->msg, SearchPosition(&pos[current_id], &info[current_id]));
        pkg->ctrl_signal = PLAY_MOVE_SUCC;
        send(conn_socket, pkg, sizeof(*pkg), 0);
        result = checkresult(&pos[current_id]);

        if (result >= 3 && result <= 6)
        {
            pkg->ctrl_signal = END_GAME_DRAW;
            strcpy(pkg->msg, "Draw\n");
            strcpy(user_account->match[user_account->match_count].state, "DRAW");
            send(conn_socket, pkg, sizeof(*pkg), 0);
        }
        else if (result == 1)
        {
            strcpy(user_account->match[user_account->match_count].competitor_name, "Computer");
            if (pos[current_id].side)
            {
                pkg->ctrl_signal = END_GAME_LOSE;   
                strcpy(user_account->match[user_account->match_count].state, "LOSE");             
                strcpy(pkg->msg, "You Lose\n");
            }
            else
            {
                pkg->ctrl_signal = END_GAME_WIN;
                user_account->win++;
                strcpy(user_account->match[user_account->match_count].state, "WIN");
                strcpy(pkg->msg, "You Win\n");
            }           
            user_account->match_count++;
            send(conn_socket, pkg, sizeof(*pkg), 0);
        }
        else
        {
            strcpy(user_account->match[user_account->match_count].competitor_name, "Computer");
            if (result == 2)
            {
                if (pos[current_id].side)
                {
                    pkg->ctrl_signal = END_GAME_WIN;
                    strcpy(user_account->match[user_account->match_count].state, "WIN");
                    strcpy(pkg->msg, "You Win\n");
                }
                else
                {
                    pkg->ctrl_signal = END_GAME_LOSE;
                    strcpy(user_account->match[user_account->match_count].state, "LOSE");          
                    strcpy(pkg->msg, "You Lose\n");
                }
                user_account->match_count++;
                send(conn_socket, pkg, sizeof(*pkg), 0);
            }
        }
    }
    else
    {
        result = checkresult(&pos[current_id]);
        if (result >= 3 && result <= 6)
        {
            pkg->ctrl_signal = END_GAME_DRAW;
            strcpy(pkg->msg, "Draw 1\n");
            send(conn_socket, pkg, sizeof(*pkg), 0);
        }
        else if (result == 1)
        {
            if (pos[current_id].side)
            {
                pkg->ctrl_signal = END_GAME_LOSE;
                strcpy(user_account->match[user_account->match_count].state, "LOSE"); 
                strcpy(pkg->msg, "You Lose\n");
            }
            else
            {
                pkg->ctrl_signal = END_GAME_WIN;
                strcpy(user_account->match[user_account->match_count].state, "WIN"); 
                strcpy(pkg->msg, "You Win\n");
            }
            user_account->match_count++;
            send(conn_socket, pkg, sizeof(*pkg), 0);
        }
        else
        {
            if (pos[current_id].side)
            {
                pkg->ctrl_signal = END_GAME_WIN;
                strcpy(user_account->match[user_account->match_count].state, "WIN"); 
                strcpy(pkg->msg, "You Win\n");
            }
            else
            {
                pkg->ctrl_signal = END_GAME_LOSE;
                strcpy(user_account->match[user_account->match_count].state, "LOSE"); 
                strcpy(pkg->msg, "You Lose\n");
            }
            user_account->match_count++;            
            send(conn_socket, pkg, sizeof(*pkg), 0);
        }
    }
}

void LeavePlayComputerServer(int conn_socket, Package *pkg)
{
    int user_id = search_user(conn_socket);
    play_with_computer[user[user_id].computer_id] = -1;
    pkg->ctrl_signal = LEAVE_COMPUTER_MATCH_SUCC;
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void CreateRoomServer(int conn_socket, Package *pkg)
{
    int user_id = search_user(conn_socket);
    int room_id = -1;
    for (int i = 0; i < MAX_ROOM; i++)
    {
        if (room[i].curr_num == 0)
        {
            room_id = i;
            AddRoom(&user[user_id], room_id);
            AddPlayerInRoom(user[user_id], &room[i]);
            sprintf(room[i].name, "Room_%d", room_id);
            break;
        }
    }
    strcpy(pkg->msg, room[room_id].name);
    pkg->group_id = room_id;
    pkg->ctrl_signal = CREATE_ROOM_SUCC;
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

int AddRoom(Active_user *user, int room_id)
{
    for (int i = 0; i < MAX_ROOM; i++)
    {
        if (user->room_id < 0)
        {
            user->room_id = room_id;
            return 1;
        }
    }
    return 0;
}

int AddPlayerInRoom(Active_user user, Room *room)
{
    int i = 0;
    for (i = 0; i < MAX_USER; i++)
    {
        if (room->member[i].socket < 0)
        {
            room->member[i].socket = user.socket;
            strcpy(room->member[i].username, user.username);
            room->curr_num++;
            return i;
        }
    }
    return 0;
}

void JointRoomServer(int conn_socket, Package *pkg)
{
    char room_name[ROOM_NAME_SIZE];
    int room_id = -1;
    int user_id = -1;

    user_id = search_user(conn_socket);
    strcpy(room_name, pkg->msg);
    room_id = SearchRoom(room, user[user_id], room_name);
    if (room_id >= 0)
    {
        printf("%s JOIN Room %s\n", pkg->sender, room[room_id].name);
        AddRoom(&user[user_id], room_id);
        AddPlayerInRoom(user[user_id], &room[room_id]);
        strcpy(pkg->msg, room_name);
        pkg->ctrl_signal = JOINT_ROOM_SUCC;
        pkg->group_id = room_id;
        send(conn_socket, pkg, sizeof(*pkg), 0);
        printf("Num: %d\n", room[room_id].curr_num);
        if (room[room_id].curr_num == 2)
        {
            sleep(1);
            int user2_id = search_user(room[room_id].member[0].socket);
            pkg->ctrl_signal = START_GAME;
            // Tuan
            strcpy(pkg->msg, "Start Game");
            send(room[room_id].member[0].socket, pkg, sizeof(*pkg), 0);
            send(room[room_id].member[1].socket, pkg, sizeof(*pkg), 0);

            user[user_id].computer_id = SearchRoomPlayComputer();
            user[user2_id].computer_id = user[user_id].computer_id;
            int current_id = user[user_id].computer_id;

            CreateNewBoard(current_id);
            info[current_id].GAME_MODE = CONSOLEMODE;
            info[current_id].POST_THINKING = TRUE;

            setbuf(stdin, NULL);
            setbuf(stdout, NULL);
            printf("Create Match Complete\n");
            
            ParseFen(START_FEN, &pos[current_id]);
            pkg->ctrl_signal = CREATE_MATCH_SUCC;
            strcpy(pkg->msg, PrintBoard(&pos[current_id]));
            send(room[room_id].member[0].socket, pkg, sizeof(*pkg), 0);
            send(room[room_id].member[1].socket, pkg, sizeof(*pkg), 0);

            pkg->ctrl_signal = WAIT_MOVE;
            strcpy(pkg->msg, "Wait Player Move");
            send(room[room_id].member[1].socket, pkg, sizeof(*pkg), 0);            
        }
    }
    else
    {
        pkg->ctrl_signal = ERR_ROOM_NOT_FOUND;
        send(conn_socket, pkg, sizeof(*pkg), 0);
    }
}

void PlayWithOtherPlayer(int conn_socket, Package *pkg)
{
    char inBuf[80], command[80];
    int move = NOMOVE;
    int result;
    strcpy(inBuf, pkg->msg);
    int room_id = pkg->group_id;
    int user1_id;

    if (room[room_id].member[0].socket != conn_socket)
    {
        user1_id = search_user(room[room_id].member[0].socket);
    }
    else
    {
        user1_id = search_user(room[room_id].member[1].socket);
    }
    int user_id = search_user(conn_socket);

    int current_id = user[user_id].computer_id;

    move = ParseMove(inBuf, &pos[current_id]);
    if (move == NOMOVE)
    {
        pkg->ctrl_signal = ERR_MOVE;
        send(conn_socket, pkg, sizeof(*pkg), 0);
        return;
    }
    MakeMove(&pos[current_id], move);
    pos[current_id].ply = 0;

    if ((checkresult(&pos[current_id]) == FALSE))
    {

        pkg->ctrl_signal = PLAY_MOVE_SUCC;
        strcpy(pkg->msg, PrintBoard(&pos[current_id]));
        send(user[user_id].socket, pkg, sizeof(*pkg), 0);
        send(user[user1_id].socket, pkg, sizeof(*pkg), 0);

        pkg->ctrl_signal = WAIT_MOVE;
        strcpy(pkg->msg, "Wait Player Move");
        send(user[user_id].socket, pkg, sizeof(*pkg), 0);
    }
    else
    {
        result = checkresult(&pos[current_id]);
        if (result >= 3 && result <= 6)
        {
            pkg->ctrl_signal = END_GAME_DRAW;
            strcpy(pkg->msg, "Draw 1\n");
            send(user[user_id].socket, pkg, sizeof(*pkg), 0);
            send(user[user1_id].socket, pkg, sizeof(*pkg), 0);
        }
        else if (result == 1)
        {
            if (pos[current_id].side)
            {
                pkg->ctrl_signal = END_GAME_LOSE;
                strcpy(pkg->msg, "You Lose\n");
                send(user[user_id].socket, pkg, sizeof(*pkg), 0);
                pkg->ctrl_signal = END_GAME_WIN;
                strcpy(pkg->msg, "You Win\n");
                send(user[user1_id].socket, pkg, sizeof(*pkg), 0);
            }
            else
            {
                pkg->ctrl_signal = END_GAME_WIN;
                strcpy(pkg->msg, "You Win\n");
                send(user[user_id].socket, pkg, sizeof(*pkg), 0);
                pkg->ctrl_signal = END_GAME_LOSE;
                strcpy(pkg->msg, "You Lose\n");
                send(user[user1_id].socket, pkg, sizeof(*pkg), 0);
            }            
        }
        else
        {
            if (pos[current_id].side)
            {
                pkg->ctrl_signal = END_GAME_LOSE;
                strcpy(pkg->msg, "You Lose\n");
                send(user[user_id].socket, pkg, sizeof(*pkg), 0);
                pkg->ctrl_signal = END_GAME_WIN;
                strcpy(pkg->msg, "You Win\n");
                send(user[user1_id].socket, pkg, sizeof(*pkg), 0);
            }
            else
            {
                pkg->ctrl_signal = END_GAME_WIN;
                strcpy(pkg->msg, "You Win\n");
                send(user[user_id].socket, pkg, sizeof(*pkg), 0);
                pkg->ctrl_signal = END_GAME_LOSE;
                strcpy(pkg->msg, "You Lose\n");
                send(user[user1_id].socket, pkg, sizeof(*pkg), 0);
            }            
        }
    }
}

int SearchRoom(Room room[], Active_user user, char *name)
{
    int i;
    int room_id = -1;
    for (i = 0; i < MAX_ROOM; i++)
    {
        printf("Room %s %s\n", room[i].name, name);
        if (strcmp(room[i].name, name) == 0)
        {
            room_id = i;
            // printf("%s\n",group[i].group_name);
            return room_id;
        }
    }
    return -1;
}

void LeaveRoomServer(int conn_socket, Package *pkg)
{
    int room_id = pkg->group_id;
    printf("%d\n", room_id);
    int user_id = search_user(conn_socket);
    int i = 0;
    for (i = 0; i < MAX_USER; i++)
    {
        Member mem = room[room_id].member[i];
        printf("%s %s\n", user[user_id].username, mem.username);
        if (strcmp(mem.username, user[user_id].username) == 0)
        {
            room[room_id].member[i].socket = -1;
            room[room_id].curr_num--;
            user[user_id].room_id = -1;
            strcpy(room[room_id].member[i].username, "");
            // gui thong bao den cho moi nguoi
            // strcpy(pkg->msg, "LEAVE GROUP ");
            // pkg->ctrl_signal = GROUP_CHAT;
            // sv_group_chat(conn_socket, pkg);

            // gui lai cho user
            strcpy(pkg->msg, "LEAVE ROOM SUCCESS: ");
            strcat(pkg->msg, room[room_id].name);
            pkg->ctrl_signal = LEAVE_ROOM_SUCC;
            send(conn_socket, pkg, sizeof(*pkg), 0);
        }
    }
}

void ViewFriendServer(int conn_socket, Package *pkg)
{
    // Thai
    node use_friend = getAccountBySocket(conn_socket);
    if (use_friend->frie_count == 0)
    {
        strcpy(pkg->msg, "No friends yet\n");
    }
    else
    {
        strcpy(pkg->msg, "Your friend\n");
        for (int i = 0; i < use_friend->frie_count; i++)
        {
            strcat(pkg->msg, use_friend->friends[i]);
            strcat(pkg->msg, "\n");
        }
    }

    // pkg->ctrl_signal,VIEW_FRIEND);
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void AddFriendServer(int conn_socket, Package *pkg)
{
    // Thai
    node use_friend = search(acc_list, pkg->receiver);
    node sender = search(acc_list, pkg->sender);
    int i = 0;

    if (use_friend == NULL)
    {
        pkg->ctrl_signal = ERR_INVALID_RECEIVER;
        send(conn_socket, pkg, sizeof(*pkg), 0);
        return;
    }
    else if (use_friend->frie_count == MAX_FRIEND)
    {
        pkg->ctrl_signal = ERR_FULL_FRIEND;
        send(conn_socket, pkg, sizeof(*pkg), 0);
        return;
    }
    if (use_friend->is_signed_in == 1)
    {
        for (i = 0; i < MAX_USER; i++)
        {
            if (strcmp(pkg->receiver, user[i].username) == 0)
            {
                if (CheckWaitRequestFriend(sender, pkg->receiver) == 0)
                {
                    pkg->ctrl_signal = ADD_FRIEND_REQUEST;
                    strcpy(pkg->msg, "You have add friend request from ");
                    strcat(pkg->msg, pkg->sender);
                    send(user[i].socket, pkg, sizeof(*pkg), 0);
                }
                break;
            }
        }
    }
    if (CheckWaitRequestFriend(sender, pkg->receiver) == 0)
    {
        strcpy(use_friend->friend_req[use_friend->frie_req_count++], pkg->sender);
        pkg->ctrl_signal = SENT_FRIEND_REQUEST_SUCC;
        strcpy(pkg->msg, "Friend request has been sent successfully\n");
        send(conn_socket, pkg, sizeof(*pkg), 0);
    }
    else
    {
        pkg->ctrl_signal = ADD_FRIEND_SUCC;
        DeleteFriendRequest(use_friend, pkg->sender);
        sender->frie_count++;
        use_friend->frie_count++;
        strcpy(use_friend->friends[use_friend->frie_count++], sender->username);
        strcpy(sender->friends[sender->frie_count++], use_friend->username);
        send(conn_socket, pkg, sizeof(*pkg), 0);
    }
}

void DeleteFriendRequest(node account_friend, char sender_name[])
{
    for (int i = 0; i < account_friend->frie_req_count; i++)
    {
        if (strcmp(account_friend->friend_req[i], sender_name) == 0)
        {
            for (int j = i; j < account_friend->frie_req_count - 1; j++)
            {
                strcpy(account_friend->friend_req[i], account_friend->friend_req[i + 1]);
            }
            account_friend->frie_req_count--;
            return;
        }
    }
}

int DeleteFriend(node account_friend, char sender_name[])
{
    for (int i = 0; i < account_friend->frie_count; i++)
    {
        if (strcmp(account_friend->friends[i], sender_name) == 0)
        {
            for (int j = i; j < account_friend->frie_count - 1; j++)
            {
                strcpy(account_friend->friends[i], account_friend->friends[i + 1]);
            }
            account_friend->frie_count--;
            return 1;
        }
    }
    return -1;
}

int CheckWaitRequestFriend(node account_friend, char sender_name[])
{
    for (int i = 0; i < account_friend->frie_req_count; i++)
    {
        if (strcmp(account_friend->friend_req[i], sender_name) == 0)
            return 1;
    }
    return 0;
}

void RemoveFriendServer(int conn_socket, Package *pkg)
{
    // Thai
    node use_friend = search(acc_list, pkg->receiver);
    node sender = search(acc_list, pkg->sender);
    if (use_friend == NULL)
    {
        strcpy(pkg->msg, "Don't see this person in your friend list.");
        send(conn_socket, pkg, sizeof(*pkg), 0);
        return;
    }

    int check = DeleteFriend(use_friend, pkg->sender);
    // pkg->ctrl_signal = REMOVE_FRIEND;
    if (check == -1)
    {
        strcpy(pkg->msg, "Don't see this person in your friend list.");
        send(conn_socket, pkg, sizeof(*pkg), 0);
    }
    else
    {
        check = DeleteFriend(sender, pkg->receiver);
        strcpy(pkg->msg, "Deleted.");
        send(conn_socket, pkg, sizeof(*pkg), 0);
    }
}

void ReplyFriendServer(int conn_socket, Package *pkg)
{
    // Thai
    node use_friend = getAccountBySocket(conn_socket);
    if (use_friend->frie_req_count == 0)
    {
        strcpy(pkg->msg, "No friend request yet\n");
    }
    else
    {
        strcpy(pkg->msg, "Your friend request\n");
        for (int i = 0; i < use_friend->frie_req_count; i++)
        {
            strcat(pkg->msg, use_friend->friend_req[i]);
            strcat(pkg->msg, "\n");
        }
    }

    // pkg->ctrl_signal,VIEW_FRIEND);

    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void AcceptFriend(int conn_socket, Package *pkg)
{
    int check = -1;
    node user_name = search(acc_list, pkg->sender);
    node use_friend = search(acc_list, pkg->receiver);
    for (int i = 0; i < user_name->frie_req_count; i++)
    {
        if (strcmp(user_name->friend_req[i], pkg->receiver) == 0)
        {
            check = 1;
            break;
        }
    }
    if (check == -1)
    {
        pkg->ctrl_signal = SHOW_FRIEND_REQUEST_ERROR;
        strcpy(pkg->msg, "Not found this person in your list request.\n");
        send(conn_socket, pkg, sizeof(*pkg), 0);
    }
    else
    {
        if (user_name->frie_count == 10)
        {
            pkg->ctrl_signal = SHOW_FRIEND_REQUEST_ERROR;
            strcpy(pkg->msg, "Full friend.\n");
            send(conn_socket, pkg, sizeof(*pkg), 0);
        }
        else
        {
            strcpy(user_name->friends[user_name->frie_count], pkg->receiver);
            user_name->frie_count++;
            strcpy(use_friend->friends[use_friend->frie_count], pkg->sender);
            use_friend->frie_count++;
            DeleteFriendRequest(user_name, pkg->receiver);
            strcpy(pkg->msg, "Make friends successfully.");
            pkg->ctrl_signal = FRIEND_REQUEST_SUCC;
            send(conn_socket, pkg, sizeof(*pkg), 0);
        }
    }
}

void NotAcceptFriend(int conn_socket, Package *pkg)
{
    int check = -1;
    node user_name = search(acc_list, pkg->sender);
    node use_friend = search(acc_list, pkg->receiver);
    for (int i = 0; i < user_name->frie_req_count; i++)
    {
        if (strcmp(user_name->friend_req[i], pkg->receiver) == 0)
        {
            check = 1;
            break;
        }
    }
    if (check == -1)
    {
        pkg->ctrl_signal = SHOW_FRIEND_REQUEST_ERROR;
        strcpy(pkg->msg, "Not found this person in your list request.\n");
        send(conn_socket, pkg, sizeof(*pkg), 0);
    }
    else
    {
        DeleteFriendRequest(user_name, pkg->receiver);
        strcpy(pkg->msg, "Deleted.");
        pkg->ctrl_signal = FRIEND_REQUEST_SUCC;
        send(conn_socket, pkg, sizeof(*pkg), 0);
    }
}

void CreateNewBoard(int current_id)
{
    // AllInit();
    info[current_id].quit = FALSE;
    pos[current_id].HashTable->pTable = NULL;
    InitHashTable(pos[current_id].HashTable, 64);
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
}
// main
int main()
{
    make_server();
    return 0;
}