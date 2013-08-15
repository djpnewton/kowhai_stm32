//
// includes
//

#include "prot.h"
#include "../kowhai/kowhai_protocol_server.h"

#ifdef STM32F1
#include <libopencm3/stm32/f1/gpio.h>
#elif STM32F4
#include <libopencm3/stm32/f4/gpio.h>
#else
ERROR
#endif

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define MAX_PACKET_SIZE 64

//
// treenode symbols
//

#include "symbols.h"

//
// settings tree descriptor
//

struct kowhai_node_t settings_descriptor[] =
{
    { KOW_BRANCH_START,     SYM_SETTINGS,       1,                0 },

    { KOW_UINT8,            SYM_GPIOA,          16,               0 },
    { KOW_UINT8,            SYM_GPIOB,          16,               0 },
    { KOW_UINT8,            SYM_GPIOC,          16,               0 },
    { KOW_UINT8,            SYM_GPIOD,          16,               0 },

    { KOW_BRANCH_END,       SYM_SETTINGS,       0,                0 },
};

//
// air goblin addresses descriptor
//

struct kowhai_node_t ag_addrs_descriptor[] =
{
    { KOW_BRANCH_START,     SYM_AG_ADDRESSES,   1,                0 },
    { KOW_BRANCH_START,     SYM_ADDRESS,        8,                0 },
    { KOW_UINT8,            SYM_DIGIT,          5,                0 },
    { KOW_BRANCH_END,       SYM_ADDRESS,        0,                0 },
    { KOW_BRANCH_END,       SYM_AG_ADDRESSES,   0,                0 },
};

//
// tree structs
//

#pragma pack(1)

struct settings_data_t
{
    uint8_t gpio_a[16];
    uint8_t gpio_b[16];
    uint8_t gpio_c[16];
    uint8_t gpio_d[16];
};

#pragma pack()

//
// global variables
//

static struct settings_data_t settings;

static union function_data_t
{
    uint8_t ag_addresses[8][5];
} function_data;

//
// server structures
//

struct kowhai_protocol_server_tree_item_t tree_list[] = {
    { KOW_TREE_ID(SYM_SETTINGS),                    settings_descriptor,            sizeof(settings_descriptor),            &settings },
    { KOW_TREE_ID_FUNCTION_ONLY(SYM_AG_ADDRESSES),  ag_addrs_descriptor,            sizeof(ag_addrs_descriptor),            &function_data.ag_addresses },
};
struct kowhai_protocol_id_list_item_t tree_id_list[COUNT_OF(tree_list)];
struct kowhai_protocol_server_function_item_t function_list[] = {
    { KOW_FUNCTION_ID(SYM_AG_ADDRESSES), {KOW_UNDEFINED_SYMBOL, SYM_AG_ADDRESSES} },
};
struct kowhai_protocol_id_list_item_t function_id_list[COUNT_OF(function_list)];

char packet_buffer[MAX_PACKET_SIZE];
struct kowhai_protocol_server_t server;
prot_send_buffer_t send_buffer_cb;

//
// functions
//

void node_pre_write(pkowhai_protocol_server_t server, void* param, uint16_t tree_id, struct kowhai_node_t* node, int offset)
{
}

#define GPIO_SET(bank, pin, value) \
{ \
    if (value) \
        gpio_set(bank, pin); \
    else \
        gpio_clear(bank, pin); \
}

#define GPIO_SET_BANK(bank, values) \
{ \
    GPIO_SET(bank, GPIO0, values[0]); \
    GPIO_SET(bank, GPIO1, values[1]); \
    GPIO_SET(bank, GPIO2, values[2]); \
    GPIO_SET(bank, GPIO3, values[3]); \
    GPIO_SET(bank, GPIO4, values[4]); \
    GPIO_SET(bank, GPIO5, values[5]); \
    GPIO_SET(bank, GPIO6, values[6]); \
    GPIO_SET(bank, GPIO7, values[7]); \
    GPIO_SET(bank, GPIO8, values[8]); \
    GPIO_SET(bank, GPIO9, values[9]); \
    GPIO_SET(bank, GPIO10, values[10]); \
    GPIO_SET(bank, GPIO11, values[11]); \
    GPIO_SET(bank, GPIO12, values[12]); \
    GPIO_SET(bank, GPIO13, values[13]); \
    GPIO_SET(bank, GPIO14, values[14]); \
    GPIO_SET(bank, GPIO15, values[15]); \
}

void node_post_write(pkowhai_protocol_server_t server, void* param, uint16_t tree_id, struct kowhai_node_t* node, int offset, int bytes_written)
{
#ifdef STM32F1
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO_ALL);
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO_ALL);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO_ALL);
    gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO_ALL);
#elif STM32F4
    int usb_pins = GPIO9 | GPIO11 | GPIO12;
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_ALL & !usb_pins);
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_ALL);
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_ALL);
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_ALL);
#else
ERROR
#endif
    GPIO_SET_BANK(GPIOA, settings.gpio_a);
    GPIO_SET_BANK(GPIOB, settings.gpio_b);
    GPIO_SET_BANK(GPIOC, settings.gpio_c);
    GPIO_SET_BANK(GPIOD, settings.gpio_d);
}

void server_buffer_send(pkowhai_protocol_server_t server, void* param, void* buffer, size_t buffer_size)
{
    if (send_buffer_cb != NULL)
        send_buffer_cb(buffer, buffer_size);
}

int function_called(pkowhai_protocol_server_t server, void* param, uint16_t function_id)
{
    switch (function_id)
    {
        case SYM_AG_ADDRESSES:
            function_data.ag_addresses[0][0] = 0xff;
            function_data.ag_addresses[0][1] = 0x00;
            function_data.ag_addresses[0][2] = 0xff;
            return 1;
    }
    return 0;
}

void prot_init(prot_send_buffer_t send_buffer)
{
    send_buffer_cb = send_buffer;
    kowhai_server_init(&server,
        MAX_PACKET_SIZE,
        packet_buffer,
        node_pre_write,
        node_post_write,
        NULL,
        server_buffer_send,
        NULL,
        COUNT_OF(tree_list),
        tree_list,
        tree_id_list,
        COUNT_OF(function_list),
        function_list,
        function_id_list,
        function_called,
        NULL,
        COUNT_OF(symbols),
        symbols);
}

void prot_process_packet(void* buffer, size_t buffer_size)
{
    kowhai_server_process_packet(&server, buffer, buffer_size);
}

