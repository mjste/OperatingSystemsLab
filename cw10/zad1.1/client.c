#include "common.h"

void process_args_connect_sock(int argc, char **argv, int *sfdp);
void print_board(char *board);
char get_move(char *board, int symbol);
int check_stop_conditions(char *board, int symbol);

int main(int argc, char **argv)
{
    int sfd;
    process_args_connect_sock(argc, argv, &sfd);

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    strcpy(buffer + 1, argv[1]);
    buffer[0] = strlen(buffer + 1);

    write(sfd, buffer, strlen(buffer));
    memset(buffer, 0, BUFFER_SIZE);
    read(sfd, buffer, 1);
    switch (buffer[0])
    {
    case WAIT:
        printf("Waiting for opponent\n");
        break;
    case WRONG_MESSAGE:
        read(sfd, buffer, 1);
        read(sfd, buffer, buffer[0]);
        fatal(buffer);
    }

    read(sfd, buffer, 3);
    int symbol = buffer[0];
    int my_turn = buffer[1];
    char opp_name[NAME_LENGTH];
    int len = buffer[2];
    memset(buffer, 0, BUFFER_SIZE);
    read(sfd, buffer, len);
    strcpy(opp_name, buffer);
    printf("Your opponent is %s\n", opp_name);

    char board[9];
    for (int i = 0; i < 9; i++)
    {
        board[i] = ' ';
    }

    if (my_turn == FIRST)
    {
        print_board(board);
        char move = get_move(board, symbol);
        write(sfd, &move, 1);
        print_board(board);
    }

    while (1)
    {
        char msg;
        printf("Ruch przeciwnika\n");
        if (read(sfd, &msg, 1) != 1)
            fatal("Connection broken");

        switch (msg)
        {
        case QUIT:
            printf("Server ordered quit\n");
            exit(0);
            break;
        case PING:
            msg = PING;
            write(sfd, &msg, 1);
            continue;
        default:
            if (msg < 1 || msg > 9)
                fatal("Incorrect message from server");
            if (symbol == CROSS)
            {
                board[msg - 1] = 'O';
            }
            else
            {
                board[msg - 1] = 'X';
            }
        }

        int result = check_stop_conditions(board, symbol);
        switch (result)
        {
        case DEFEAT:
            printf("Sorry, you lost\n");
            msg = QUIT;
            write(sfd, &msg, 1);
            exit(0);
        case DRAW:
            break;
        }
        get_move(board, symbol);
        result = check_stop_conditions(board, symbol);
    }

    return 0;
}

void process_args_connect_sock(int argc, char **argv, int *sfdp)
{
    int sfd;
    /*
        name
        inet/unix
        ip/path
        [port]
        */
    switch (argc)
    {
    case 4:
        if (strcmp("unix", argv[2]) != 0)
            fatal("Usage: <name> unix <path>");

        sfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sfd == -1)
            fatal("Socket local");

        struct sockaddr_un sun;
        sun.sun_family = AF_UNIX;
        strcpy(sun.sun_path, argv[3]);

        if (connect(sfd, (struct sockaddr *)&sun, sizeof(sun)) == -1)
            fatal("connect local");
        break;
    case 5:
        if (strcmp("inet", argv[2]) != 0)
            fatal("Usage: <name> inet <ip> <port>");

        sfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sfd == -1)
            fatal("Socket inet");

        struct sockaddr_in sin;
        sin.sin_addr.s_addr = inet_addr(argv[3]);
        sin.sin_family = AF_INET;
        sin.sin_port = htons(atoi(argv[4]));

        if (connect(sfd, (struct sockaddr *)&sin, sizeof(sin)) == -1)
            fatal("Connect inet");
        break;
    default:
        fatal("Usage: <name> ( unix <path> ) | ( inet <ip> <port>)");
    }

    // set timeout
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    if (setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1 ||
        setsockopt(sfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == -1)
        fatal("setting timeout");

    *sfdp = sfd;
    return;
}

char get_move(char *board, int symbol)
{
    int ans;
    int running = 1;
    while (running)
    {
        printf("Wybierz pole do zagrania (1-9): ");
        scanf("%d", &ans);
        if (ans >= 1 && ans <= 9 && board[ans] == ' ')
        {
            running = 0;
        }
    }
    if (symbol == CROSS)
    {
        board[ans - 1] = 'X';
    }
    else
    {
        board[ans - 1] = 'O';
    }
    return ans;
}

void print_board(char *board)
{
    printf(" %c | %c | %c \n", board[0], board[1], board[2]);
    printf("---+---+---\n");
    printf(" %c | %c | %c \n", board[3], board[4], board[5]);
    printf("---+---+---\n");
    printf(" %c | %c | %c \n", board[6], board[7], board[8]);
}

int check_stop_conditions(char *board, int symbol)
{
    int other_symbol = symbol == CROSS ? CIRCLE : CROSS;

    for (int i = 0; i < 3; i++)
    {
        int whole_row = 1;
        int whole_column = 1;
        for (int j = 0; j < 3; j++)
        {
            if (board[3 * i + j] != symbol)
                whole_row = 0;
            if (board[3 * j + i] != symbol)
                whole_column = 0;
        }
        if (whole_column || whole_row)
        {
            return WIN;
        }
    }

    // diagonals
    if (board[0] == symbol && board[4] == symbol && board[8] == symbol)
        return WIN;
    if (board[2] == symbol && board[4] == symbol && board[6] == symbol)
        return WIN;

    // check for defeat
    for (int i = 0; i < 3; i++)
    {
        int whole_row = 1;
        int whole_column = 1;
        for (int j = 0; j < 3; j++)
        {
            if (board[3 * i + j] != other_symbol)
                whole_row = 0;
            if (board[3 * j + i] != other_symbol)
                whole_column = 0;
        }
        if (whole_column || whole_row)
        {
            return DEFEAT;
        }
    }

    // diagonals
    if (board[0] == other_symbol && board[4] == other_symbol && board[8] == other_symbol)
        return DEFEAT;
    if (board[2] == other_symbol && board[4] == other_symbol && board[6] == other_symbol)
        return DEFEAT;

    // draw == all_filled
    int all_filled = 1;
    for (int i = 0; i < 9; i++)
    {
        if (board[i] == ' ')
            all_filled = 0;
    }
    if (all_filled)
        return DRAW;
    else
        return PLAY;
}