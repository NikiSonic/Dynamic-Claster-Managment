//точка входа: аргументы, главный цикл, консоль

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>

#include "config.h"
#include "proto.h"
#include "net.h"
#include "peers.h"

static int sock;
static int my_port;
static int running = 1;

static void usage(void)
{
    fprintf(stderr,
        "запуск:  node --port <порт> [--join <ip:порт>]\n"
        "пример:  node --port 5000\n"
        "         node --port 5001 --join 127.0.0.1:5000\n");
}

static void help(void)
{
    printf("команды:\n"
           "  peers                 кто в кластере\n"
           "  send <порт> <текст>   отправить текст узлу\n"
           "  quit                  выйти\n");
}

// собрать сообщение и отправить
static void send_msg(int type, uint32_t ip, uint16_t port, const char *text)
{
    struct msg m;
    memset(&m, 0, sizeof m);
    m.type = type;
    m.port = my_port;
    snprintf(m.text, sizeof m.text, "%s", text);
    net_send(sock, ip, port, &m);
}

// пришло сообщение
static void on_message(struct msg *m, uint32_t ip, uint16_t port)
{
    peers_seen(ip, port);

    switch (m->type) {
    case MSG_HELLO:
        printf("узел %d вошёл\n", port);
        send_msg(MSG_WELCOME, ip, port, "");
        break;
    case MSG_WELCOME:
        printf("узел %d нас принял\n", port);
        break;
    case MSG_DATA:
        printf("[%d] %s\n", port, m->text);
        break;
    default:
        printf("от %d непонятное сообщение, type=%d\n", port, m->type);
    }
}

// набрали строку в консоли
static void on_command(char *line)
{
    line[strcspn(line, "\n")] = 0;      // убрать \n в конце

    if (strcmp(line, "peers") == 0) {
        peers_print();

    } else if (strncmp(line, "send ", 5) == 0) {
        char *rest = line + 5;              // всё после "send "
        int   port = atoi(rest);
        char *text = strchr(rest, ' ');     // пробел после порта
        if (port <= 0 || text == NULL) {
            printf("надо так: send <порт> <текст>\n");
            return;
        }
        text++;                             // пропустить сам пробел

        int i = peers_find_by_port(port);
        if (i < 0) {
            printf("не знаю узел %d, смотри peers\n", port);
            return;
        }
        send_msg(MSG_DATA, peers[i].ip, peers[i].port, text);

    } else if (strcmp(line, "quit") == 0) {
        running = 0;

    } else if (strcmp(line, "help") == 0) {
        help();

    } else if (line[0] != 0) {
        printf("не понял, напиши help\n");
    }
}

int main(int argc, char **argv)
{
    uint32_t join_ip   = 0;
    uint16_t join_port = 0;
    int      have_join = 0;

    setvbuf(stdout, NULL, _IOLBF, 0);   // печатать сразу
    setvbuf(stdin,  NULL, _IONBF, 0);   // читать клавиатуру без буфера

    // аргументы
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            my_port = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--join") == 0 && i + 1 < argc) {
            if (net_parse_addr(argv[i + 1], &join_ip, &join_port) < 0) {
                fprintf(stderr, "плохой адрес: %s\n", argv[i + 1]);
                return 1;
            }
            have_join = 1;
            i++;
        } else {
            usage();
            return 1;
        }
    }
    if (my_port <= 0 || my_port > 65535) {
        usage();
        return 1;
    }

    sock = net_open(my_port);
    if (sock < 0)
        return 1;
    printf("узел %d запущен\n", my_port);
    help();

    // здороваемся
    if (have_join) {
        peers_add(join_ip, join_port);
        send_msg(MSG_HELLO, join_ip, join_port, "");
    }

    // главный цикл: ждём сразу и сеть, и клавиатуру
    struct pollfd fds[2] = {
        { .fd = sock, .events = POLLIN },
        { .fd = 0,    .events = POLLIN },   // 0 = клавиатура
    };

    while (running) {
        if (poll(fds, 2, 500) < 0) {        // ждём не дольше 0.5 с
            perror("poll");
            break;
        }

        if (fds[0].revents & POLLIN) {      // пришёл пакет
            struct msg m;
            uint32_t   ip;
            uint16_t   port;
            if (net_recv(sock, &m, &ip, &port) == 0)
                on_message(&m, ip, port);
        }

        if (fds[1].revents & POLLIN) {      // набрали строку
            char line[512];
            if (fgets(line, sizeof line, stdin) == NULL)    // ctrl+d
                break;
            on_command(line);
        }
    }

    printf("пока\n");
    return 0;
}
