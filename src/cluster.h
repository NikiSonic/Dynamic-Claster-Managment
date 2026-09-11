//логика кластера: вход, обработка сообщений

#ifndef CLUSTER_H
#define CLUSTER_H

#include <stdint.h>
#include "proto.h"

void cluster_init(int sock, int my_port);
void cluster_join(uint32_t ip, uint16_t port);                          // войти через известный узел
void cluster_send(int type, uint32_t ip, uint16_t port, const char *text);
void cluster_handle(struct msg *m, uint32_t ip, uint16_t port);         // пришло сообщение

#endif
