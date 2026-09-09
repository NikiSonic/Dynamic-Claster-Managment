//таблица узлов

#ifndef PEERS_H
#define PEERS_H

#include <stdint.h>
#include <time.h>
#include "config.h"

enum { PEER_ALIVE, PEER_SUSPECT, PEER_DEAD };

struct peer {
    int      used;          // 1 = слот занят
    uint32_t ip;
    uint16_t port;
    int      state;         // PEER_*
    time_t   last_seen;     // когда последний раз что-то от него получали
};

extern struct peer peers[MAX_PEERS];

int  peers_add(uint32_t ip, uint16_t port);       // индекс или -1 если таблица полная
void peers_remove(uint32_t ip, uint16_t port);
int  peers_find(uint32_t ip, uint16_t port);      // индекс или -1
int  peers_find_by_port(uint16_t port);           // индекс или -1
void peers_seen(uint32_t ip, uint16_t port);      // от него что-то пришло
int  peers_count(void);
void peers_print(void);

#endif
