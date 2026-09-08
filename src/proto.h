//формат сообщений

#ifndef PROTO_H
#define PROTO_H

#include <stdint.h>
#include "config.h"

enum {
    MSG_HELLO   = 1,   // хочу в кластер
    MSG_WELCOME = 2,   // заходи, вот кого я знаю
    MSG_PING    = 3,   // я живой
    MSG_BYE     = 4,   // я ухожу
    MSG_DATA    = 5    // просто текст
};

#define FLAG_XOR   1   // текст зашифрован 

// адрес узла для списка в WELCOME
struct addr_info {
    uint32_t ip;
    uint16_t port;
};

struct msg {
    uint8_t  type;                      // MSG_*
    uint8_t  flags;                     // FLAG_*
    uint16_t port;                      // порт отправителя, он же его имя
    uint8_t  count;                     // сколько адресов в list
    struct addr_info list[MAX_PEERS];   // только для WELCOME
    char     text[TEXT_SIZE];           // только для DATA
};

#endif
