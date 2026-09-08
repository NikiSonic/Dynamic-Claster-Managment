
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "proto.h"
#include "net.h"

static void usage(void)
{
    fprintf(stderr,
        "запуск:  node --port <порт> [--join <ip:порт>]\n"
        "пример:  node --port 5000\n"
        "         node --port 5001 --join 127.0.0.1:5000\n");
}

int main(int argc, char **argv)
{
    int      my_port   = 0;
    uint32_t join_ip   = 0;
    uint16_t join_port = 0;
    int      have_join = 0;

    // печатать сразу
    setvbuf(stdout, NULL, _IOLBF, 0);

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

    int sock = net_open(my_port);
    if (sock < 0)
        return 1;
    printf("узел %d запущен\n", my_port);

    // здороваемся
    if (have_join) {
        struct msg m;
        memset(&m, 0, sizeof m);
        m.type = MSG_HELLO;
        m.port = my_port;
        snprintf(m.text, sizeof m.text, "привет, я узел %d", my_port);

        net_send(sock, join_ip, join_port, &m);
        printf("-> HELLO для %s:%d\n", net_ip_str(join_ip), join_port);
    }

    // принимаем и печатаем
    while (1) {
        struct msg m;
        uint32_t   from_ip;
        uint16_t   from_port;

        if (net_recv(sock, &m, &from_ip, &from_port) < 0)
            continue;

        printf("<- от %s:%d  type=%d  text=\"%s\"\n",
               net_ip_str(from_ip), from_port, m.type, m.text);

        // на HELLO отвечаем WELCOME 
        if (m.type == MSG_HELLO) {
            struct msg reply;
            memset(&reply, 0, sizeof reply);
            reply.type = MSG_WELCOME;
            reply.port = my_port;
            snprintf(reply.text, sizeof reply.text, "заходи, я узел %d", my_port);

            net_send(sock, from_ip, from_port, &reply);
            printf("-> WELCOME для %s:%d\n", net_ip_str(from_ip), from_port);
        }
    }

    return 0;
}
