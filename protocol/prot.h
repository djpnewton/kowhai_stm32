#ifndef _PROT_H_
#define _PROT_H_

#include "stddef.h"

typedef void (*prot_send_buffer_t)(void* buffer, size_t buffer_size);

void prot_init(prot_send_buffer_t send_buffer);
void prot_process_packet(void* buffer, size_t buffer_size);

#endif

