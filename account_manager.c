// Account manager
#include "account_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Lưu dữ liệu vào link list

node create(char username[], char password[], int elo, int current_puzzle, int puzzle_point, int is_signed_in, int match_count,
            int win, int frie_count, int frie_req_count, int wait_add_friend_count, char friend[][USERNAME_SIZE], char friend_req[][USERNAME_SIZE], char wait_add_friend[][USERNAME_SIZE], Match match[])
{
    node temp;
    temp = (node)malloc(sizeof(struct Account));
    temp->next = NULL;
    strcpy(temp->username, username);
    strcpy(temp->password, password);
    temp->elo = elo;
    temp->current_puzzle = current_puzzle;
    temp->puzzle_point = puzzle_point;
    temp->is_signed_in = is_signed_in;
    temp->match_count = match_count;
    temp->win = win;
    temp->frie_req_count = frie_req_count;
    temp->frie_count = frie_count;
    temp->wait_add_friend_count = wait_add_friend_count;
    for (int i = 0; i < frie_count; i++)
    {
        strcpy(temp->friends[i], friend[i]);
    }
    for (int i = 0; i < frie_req_count; i++)
    {
        strcpy(temp->friend_req[i], friend_req[i]);
    }

    for (int i = 0; i < wait_add_friend_count; i++)
    {
        strcpy(temp->wait_add_friend[i], wait_add_friend[i]);
    }
    if(match_count <= 5) {
        for(int i = 0; i < match_count; i++) {
            strcpy(temp->match[i].competitor_name, match[i].competitor_name);
            strcpy(temp->match[i].state, match[i].state);
        }
    } else {
        for(int i = 0; i < 5; i++) {
            strcpy(temp->match[i].competitor_name, match[i].competitor_name);
            strcpy(temp->match[i].state, match[i].state);
        }
    }
    return temp;
}

node addtail(node head, node temp)
{ // Them một node vào cuối danh sách liên kết
    node p;
    if (head == NULL)
    {
        head = temp;
    }
    else
    {
        p = head;
        while (p->next != NULL)
        {
            p = p->next;
        }
        p->next = temp;
    }
    return head;
}

node search(node head, char username[])
{
    // tìm kiếm account theo username
    node p = head;
    if (strcmp(p->username, username) == 0)
    {
        return p;
    }
    while (p->next->next != NULL && strcmp(p->next->username, username) != 0)
        p = p->next;
    if (strcmp(p->next->username, username) == 0)
        return p->next;
    else
        return NULL;
}

void printLists(node head)
{
    node p = head;
    while (p != NULL)
    {
        printf("%s %s %d %d %d %d %d %d %d\n", p->username, p->password, p->elo, p->current_puzzle, p->puzzle_point, p->is_signed_in, p->match_count, p->win, p->frie_count);
        p = p->next;
    }
}

void printHistoryMatch(node head) {
    int i = 0;
    while(i < head->match_count) {
        printf("%s %s\n", head->match[i].competitor_name, head->match[i].state);
        i++;
    }
}

void printFriendList(node head)
{
    node p = head;
    // int i = 0;
    printf("Your friend\n");
    for (int i = 0; i < p->frie_count; i++)
    {
        /* code */
        printf("%s\n", p->friends[i]);
    }

    // while(p != NULL) {
    //     while(p->friends[i]) {
    //         printf("%s\n", p->friends[i++]);
    //     }
    //     i = 0;
    //     p = p -> next;
    // }
}

void readFileAccount(node *head)
{
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    int st;
    int elo;
    int match_count;
    int win;
    int frie_count;
    int frie_req_count;
    int wait_add_friend_count;
    char friends[MAX_FRIEND][USERNAME_SIZE];
    char friend_req[30][USERNAME_SIZE];
    char wait_add_friend[30][USERNAME_SIZE];
    Match match[MAX_MATCH];

    // char friends[FRIEND_COUNT][USERNAME_SIZE];
    int current_puzzle;
    int puzzle_point;
    node temp;
    int i = 0;
    int j = 0, k = 0;
    int m = 0;
    FILE *file = fopen("account.txt", "r");
    if (file == NULL)
    {
        printf("no such file.");
        return;
    }
    while (!feof(file))
    {
        fscanf(file, "%s %s\n", username, password);
        char fileName[FILENAME_SIZE] = "./account/";
        char line[MAX_LENGTH];
        strcat(fileName, username);
        strcat(fileName, ".txt");
        FILE *f = fopen(fileName, "r");
        if (f == NULL)
        {
            printf("No such file.");
            return;
        }
        while (fgets(line, sizeof(line), f) != NULL)
        {
            i = 0;
            j = 0;
            k = 0;
            char *p;
            p = (char *)malloc(100 * sizeof(char));
            if (strstr(line, "ELO"))
            {
                p = strtok(line, " ");
                while (p != NULL)
                {
                    p = strtok(NULL, " ");
                    if (p != NULL)
                        elo = atoi(p);
                }
            }
            if (strstr(line, "PUZZLE"))
            {
                p = strtok(line, " ");
                if (p != NULL)
                {
                    p = strtok(NULL, " ");
                    if (p != NULL)
                        current_puzzle = atoi(p);
                }
                if (p != NULL)
                {
                    p = strtok(NULL, " ");
                    if (p != NULL)
                        puzzle_point = atoi(p);
                }
            }
            if (strstr(line, "MATCH_COUNT"))
            {
                p = strtok(line, " ");
                while (p != NULL)
                {
                    p = strtok(NULL, " ");
                    if (p != NULL)
                        match_count = atoi(p);
                }
            }
            if (strstr(line, "WINS"))
            {
                p = strtok(line, " ");
                while (p != NULL)
                {
                    p = strtok(NULL, " ");
                    if (p != NULL)
                        win = atoi(p);
                    // printf("%d\n", win);
                }
            }

            if (strstr(line, "FRIES_COUNT"))
            {
                p = strtok(line, " ");
                while (p != NULL)
                {
                    p = strtok(NULL, " ");
                    if (p != NULL)
                        frie_count = atoi(p);
                    // printf("%d", frie_count);
                }
            }
            if (strstr(line, "FRIE_REQ_COUNT"))
            {
                p = strtok(line, " ");
                while (p != NULL)
                {
                    p = strtok(NULL, " ");
                    if (p != NULL)
                        frie_req_count = atoi(p);
                }
            }
            if (strstr(line, "WAIT_ADD_FRIE_COUNT"))
            {
                p = strtok(line, " ");
                while (p != NULL)
                {
                    p = strtok(NULL, " ");
                    if (p != NULL)
                        wait_add_friend_count = atoi(p);
                }
            }
            if (strstr(line, "FRIENDS"))
            {
                p = strtok(line, " ");
                while (p != NULL)
                {
                    p = strtok(NULL, " ");
                    if (p != NULL)
                    {
                        // printf("%s\n", p);
                        p[strlen(p)] = '\0';
                        if (p[strlen(p) - 1] == '\n')
                        {
                            p[strlen(p) - 1] = '\0';
                        }

                        // printf("i: %d\n", i);
                        strcpy(friends[i++], p);
                    }
                }
                // frie_count = i;
            }
            if (strstr(line, "FRIEND_REQUEST"))
            {
                p = strtok(line, " ");
                while (p != NULL)
                {
                    p = strtok(NULL, " ");
                    if (p != NULL)
                    {
                        // printf("%s\n", p);
                        p[strlen(p)] = '\0';
                        if (p[strlen(p) - 1] == '\n')
                        {
                            p[strlen(p) - 1] = '\0';
                        }
                        strcpy(friend_req[j++], p);
                    }
                }
                // frie_req_count = j;
            }
            if (strstr(line, "WAIT_ADD_FRIEND"))
            {
                p = strtok(line, " ");
                while (p != NULL)
                {
                    p = strtok(NULL, " ");
                    if (p != NULL)
                    {
                        // printf("%s\n", p);
                        p[strlen(p)] = '\0';
                        if (p[strlen(p) - 1] == '\n')
                        {
                            p[strlen(p) - 1] = '\0';
                        }
                        strcpy(wait_add_friend[k++], p);
                    }
                }
                wait_add_friend_count = k;
            }
            // printf("%d\n", frie_count);
            // free(p);
            if(strstr(line, "MATCH_HISTORY")) {
                while (fgets(line, sizeof(line), f) != NULL)
                {
                    p = strtok(line, " ");
                    if(p != NULL) {
                        p = strtok(NULL, " ");
                        if(p != NULL) strcpy(match[m].competitor_name, p);
                    }
                    if(p != NULL) {
                        p = strtok(NULL, " ");
                        if(p != NULL) strcpy(match[m].state, p);
                    }
                    m++;
                }
                
            }
        }
        temp = create(username, password, elo, current_puzzle, puzzle_point, 0, match_count,
                      win, frie_count, frie_req_count, wait_add_friend_count, friends, friend_req, wait_add_friend, match);
        *head = addtail(*head, temp);
    }
}

void updateAccountFile(node head)
{
    FILE *file = fopen("account.txt", "w");
    for (node p = head; p != NULL; p = p->next)
    {
        fprintf(file, "%s %s\n", p->username, p->password);
    }
    fclose(file);
}

void addFileAccount(node head, char username[])
{
    char friends[MAX_LENGTH];
    char friendRequest[MAX_LENGTH];
    char waitAddFriend[MAX_LENGTH];
    char num[2];
    char fileName[FILENAME_SIZE] = "./account/";
    strcat(fileName, username);
    strcat(fileName, ".txt");
    FILE *file = fopen(fileName, "w+");
    node temp = search(head, username);
    fprintf(file, "%s %d\n", "ELO", temp->elo);
    fprintf(file, "%s %d %d\n", "PUZZLE", temp->current_puzzle, temp->puzzle_point);
    fprintf(file, "%s %d\n", "MATCH_COUNT", temp->match_count);
    fprintf(file, "%s %d\n", "WINS", temp->win);
    fprintf(file, "%s %d\n", "FRIES_COUNT", temp->frie_count);
    for (int i = 0; i < temp->frie_count; i++)
    {
        strcat(friends, temp->friends[i]);
        if(i != temp->frie_count-1)
        strcat(friends, " ");
    }    
    fprintf(file, "%s %s\n", "FRIENDS", friends);
    strcpy(friends, "");
    fprintf(file, "%s %d\n", "FRIE_REQ_COUNT", temp->frie_req_count);
    for (int i = 0; i < temp->frie_req_count; i++)
    {
        strcat(friendRequest, temp->friend_req[i]);
        if(i != temp->frie_req_count-1)
        strcat(friendRequest, " ");
    }
    fprintf(file, "%s %s\n", "FRIEND_REQUEST", friendRequest);
    strcpy(friendRequest, "");
    fprintf(file, "%s %d\n", "WAIT_ADD_FRIE_COUNT", temp->wait_add_friend_count);
    for (int i = 0; i < temp->wait_add_friend_count; i++)
    {
        strcat(waitAddFriend, temp->wait_add_friend[i]);
        if(i != temp->wait_add_friend_count-1)
        strcat(waitAddFriend, " ");
    }
    fprintf(file, "%s %s\n", "WAIT_ADD_FRIEND", waitAddFriend);
    strcpy(waitAddFriend, "");
    fprintf(file, "%s", "MATCH_HISTORY");
    if(temp->match_count <= 5 && temp->match_count > 0) {
        for(int i = 0; i < temp->match_count; i++) {
            char number_match[10];
            strcpy(number_match, "MATCH_");
            sprintf(num, "%d", i+1);
            strcat(number_match, num);
            fprintf(file, "%s %s %s\n", number_match, temp->match[i].competitor_name, temp->match[i].state);
        }
    } else {
        for(int i = 0; i < 5; i++) {
            char number_match[10];
            strcpy(number_match, "MATCH_");
            sprintf(num, "%d", i+1);
            strcat(number_match, num);
            fprintf(file, "%s %s %s\n", number_match, temp->match[i].competitor_name, temp->match[i].state);
        }
    }
    fclose(file);
}

int getUserCount(node head)
{
    node p = head;
    int i = 0;
    while (p != NULL)
    {
        i++;
        p = p->next;
    }
    return i;
}