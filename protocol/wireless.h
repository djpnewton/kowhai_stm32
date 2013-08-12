#ifndef _WIRELESS_H_
#define _WIRELESS_H_

#define AG_MODE_MASTER 0
#define AG_MODE_SLAVE 1

void wireless_init(int mode);
void wireless_send_serial_char(char c);
void wireless_poll(void);

#endif
