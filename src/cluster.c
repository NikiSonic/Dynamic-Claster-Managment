//логика кластера

#include <stdio.h>
#include <string.h>

#include "cluster.h"
#include "peers.h"
#include "net.h"

static int sock;
static int my_port;

void cluster_init(int s, int port)
{
    sock    = s;
    my_port = port;
}

void cluster_send(int type, uint32_t ip, uint16_t port, const char *text)
{
    struct msg m;
    memset(&m, 0, sizeof m);
    m.type = type;
    m.port = my_port;
    snprintf(m.text, sizeof m.text, "%s", text);
    net_send(sock, ip, port, &m);
}

void cluster_join(uint32_t ip, uint16_t port)
{
    peers_add(ip, port);
    cluster_send(MSG_HELLO, ip, port, "");
}

// WELCOME со списком всех, кого знаем (кроме самого новичка)
static void send_welcome(uint32_t ip, uint16_t port)
{
    struct msg m;
    memset(&m, 0, sizeof m);
    m.type = MSG_WELCOME;
    m.port = my_port;

    for (int i = 0; i < MAX_PEERS; i++) {
        if (!peers[i].used)
            continue;
        if (peers[i].ip == ip && peers[i].port == port)
            continue;
        m.list[m.count].ip   = peers[i].ip;
        m.list[m.count].port = peers[i].port;
        m.count++;
    }

    net_send(sock, ip, port, &m);
}

// пришёл список: с каждым, кого ещё не знаем, здороваемся
static void take_list(struct msg *m)
{
    for (int i = 0; i < m->count; i++) {
        uint32_t ip   = m->list[i].ip;
        uint16_t port = m->list[i].port;
        if (peers_find(ip, port) >= 0)
            continue;
        peers_add(ip, port);
        cluster_send(MSG_HELLO, ip, port, "");
    }
}

void cluster_handle(struct msg *m, uint32_t ip, uint16_t port)
{
    peers_seen(ip, port);

    switch (m->type) {
    case MSG_HELLO:
        printf("узел %d вошёл\n", port);
        send_welcome(ip, port);
        break;
    case MSG_WELCOME:
        printf("узел %d нас принял, в списке ещё %d\n", port, m->count);
        take_list(m);
        break;
    case MSG_DATA:
        printf("[%d] %s\n", port, m->text);
        break;
    default:
        printf("от %d непонятное сообщение, type=%d\n", port, m->type);
    }
}
