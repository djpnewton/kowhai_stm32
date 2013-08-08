#ifndef _USB_H_
#define _USB_H_

typedef void (*usb_serial_read_cb_t)(char c);

void usb_init(usb_serial_read_cb_t serial_read);
void usb_poll_forever(void);
void usb_poll(void);
void usb_write_serial(char c);

#endif

