//работа с udp-сокетом.

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "net.h"

// собрать адрес в виде, который понимают сокеты
static void make_sockaddr(struct sockaddr_in *a, uint32_t ip, uint16_t port)
{
    memset(a, 0, sizeof *a);
    a->sin_family = AF_INET;        // ipv4
    a->sin_addr.s_addr = ip;
    a->sin_port = htons(port);      // порт в сетевой порядок байт
}

int net_open(int port)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);   // SOCK_DGRAM = udp
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in a;
    make_sockaddr(&a, htonl(INADDR_ANY), port);  // слушать на всех адресах машины

    // занять порт
    if (bind(sock, (struct sockaddr *)&a, sizeof a) < 0) {
        perror("bind");
        close(sock);
        return -1;
    }

    return sock;
}

int net_send(int sock, uint32_t ip, uint16_t port, const struct msg *m)
{
    struct sockaddr_in a;
    make_sockaddr(&a, ip, port);

    // структура уходит целиком, как есть
    ssize_t n = sendto(sock, m, sizeof *m, 0, (struct sockaddr *)&a, sizeof a);
    if (n < 0) {
        perror("sendto");
        return -1;
    }
    return 0;
}

int net_recv(int sock, struct msg *m, uint32_t *from_ip, uint16_t *from_port)
{
    struct sockaddr_in a;
    socklen_t alen = sizeof a;

    // ждёт пакет и говорит, от кого он
    ssize_t n = recvfrom(sock, m, sizeof *m, 0, (struct sockaddr *)&a, &alen);
    if (n < 0) {
        perror("recvfrom");
        return -1;
    }

    // чужой размер = не наше
    if (n != (ssize_t)sizeof *m) {
        fprintf(stderr, "net: пришёл пакет странного размера (%zd байт), пропускаю\n", n);
        return -1;
    }

    *from_ip   = a.sin_addr.s_addr;
    *from_port = ntohs(a.sin_port);     // порт обратно в обычное число
    return 0;
}

int net_parse_addr(const char *str, uint32_t *ip, uint16_t *port)
{
    char ip_str[64];
    int  p;

    if (sscanf(str, "%63[^:]:%d", ip_str, &p) != 2)
        return -1;

    *ip = inet_addr(ip_str);
    if (*ip == INADDR_NONE)
        return -1;

    if (p <= 0 || p > 65535)
        return -1;

    *port = (uint16_t)p;
    return 0;
}

const char *net_ip_str(uint32_t ip)
{
    struct in_addr a;
    a.s_addr = ip;
    return inet_ntoa(a);
}
