#ifndef _WIRELESS_H_
#define _WIRELESS_H_

#include <stdint.h>

#define AG_MAX_ADDRS 8
#define AG_ADDR_LEN 5

#define AG_MODE_MASTER 0
#define AG_MODE_SLAVE 1

void wireless_init(int mode, uint8_t addr[AG_ADDR_LEN]);
void wireless_set_address(uint8_t addr[AG_ADDR_LEN]);
int wireless_master_send_serial_char(char c);
int wireless_master_find_slaves(uint8_t* count, uint8_t addrs[AG_MAX_ADDRS][AG_ADDR_LEN]);
void wireless_poll(void);

#endif
