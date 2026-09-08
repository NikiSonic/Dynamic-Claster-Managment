//все настройки узла.

#ifndef CONFIG_H
#define CONFIG_H

#define MAX_PEERS        32    // макс узлоы
#define TEXT_SIZE        256   // макс длина текста в сообщении

#define PING_INTERVAL    2     // раз в столько секунд шлём PING всем
#define SUSPECT_TIMEOUT  5     // столько секунд тишины до подозрения
#define DEAD_TIMEOUT     10    // столько секунд тишины до смэрт

#endif
