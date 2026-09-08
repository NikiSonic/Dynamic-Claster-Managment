//сокеты

#ifndef NET_H
#define NET_H

#include <stdint.h>
#include "proto.h"

// открыть udp-сокет
int net_open(int port);

// отправить m на (ip, port). 0 или -1
int net_send(int sock, uint32_t ip, uint16_t port, const struct msg *m);

// ждать и принять одно сообщение, адрес отправителя кладёт в from_*. 0 или -1
int net_recv(int sock, struct msg *m, uint32_t *from_ip, uint16_t *from_port);

// "127.0.0.1:5000" -> ip и port. 0 или -1
int net_parse_addr(const char *str, uint32_t *ip, uint16_t *port);

// ip -> "127.0.0.1". общий буфер, два раза в одном printf нельзя
const char *net_ip_str(uint32_t ip);

#endif
