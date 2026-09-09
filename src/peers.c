//таблица узлов

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "peers.h"
#include "net.h"

struct peer peers[MAX_PEERS];

static const char *state_name[] = { "ALIVE", "SUSPECT", "DEAD" };

int peers_find(uint32_t ip, uint16_t port)
{
    for (int i = 0; i < MAX_PEERS; i++)
        if (peers[i].used && peers[i].ip == ip && peers[i].port == port)
            return i;
    return -1;
}

int peers_find_by_port(uint16_t port)
{
    for (int i = 0; i < MAX_PEERS; i++)
        if (peers[i].used && peers[i].port == port)
            return i;
    return -1;
}

int peers_add(uint32_t ip, uint16_t port)
{
    int i = peers_find(ip, port);
    if (i >= 0)
        return i;                       // уже есть

    // ищем свободный слот
    for (i = 0; i < MAX_PEERS; i++) {
        if (!peers[i].used) {
            peers[i].used      = 1;
            peers[i].ip        = ip;
            peers[i].port      = port;
            peers[i].state     = PEER_ALIVE;
            peers[i].last_seen = time(NULL);
            printf("+ узел %s:%d\n", net_ip_str(ip), port);
            return i;
        }
    }

    fprintf(stderr, "таблица узлов полная, %s:%d не влез\n", net_ip_str(ip), port);
    return -1;
}

void peers_remove(uint32_t ip, uint16_t port)
{
    int i = peers_find(ip, port);
    if (i < 0)
        return;
    printf("- узел %s:%d\n", net_ip_str(ip), port);
    peers[i].used = 0;
}

void peers_seen(uint32_t ip, uint16_t port)
{
    int i = peers_add(ip, port);
    if (i < 0)
        return;
    peers[i].last_seen = time(NULL);
    peers[i].state     = PEER_ALIVE;
}

int peers_count(void)
{
    int n = 0;
    for (int i = 0; i < MAX_PEERS; i++)
        if (peers[i].used)
            n++;
    return n;
}

void peers_print(void)
{
    time_t now = time(NULL);
    char   addr[32];

    printf("  порт   адрес                  статус   молчит\n");
    for (int i = 0; i < MAX_PEERS; i++) {
        if (!peers[i].used)
            continue;
        snprintf(addr, sizeof addr, "%s:%d", net_ip_str(peers[i].ip), peers[i].port);
        printf("  %-6d %-22s %-8s %ld с\n",
               peers[i].port, addr, state_name[peers[i].state],
               (long)(now - peers[i].last_seen));
    }
    printf("  всего: %d\n", peers_count());
}
