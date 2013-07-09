//
// includes
//

#include "prot.h"
#include "../kowhai/kowhai_protocol_server.h"

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

    { KOW_BRANCH_START,     SYM_GPIOD,          1,                0 },
    { KOW_UINT8,            SYM_P12,            1,                0 },
    { KOW_UINT8,            SYM_P13,            1,                0 },
    { KOW_UINT8,            SYM_P14,            1,                0 },
    { KOW_UINT8,            SYM_P15,            1,                0 },
    { KOW_BRANCH_END,       SYM_GPIOD,          0,                0 },

    { KOW_BRANCH_END,       SYM_SETTINGS,       0,                0 },
};

//
// settings tree structs
//

#pragma pack(1)

struct gpio_t
{
    uint8_t P12;
    uint8_t P13;
    uint8_t P14;
    uint8_t P15;
};

struct settings_data_t
{
    struct gpio_t gpio_d;
};

#pragma pack()

//
// global variables
//

static struct settings_data_t settings;

//
// server structures
//

uint16_t tree_list[] = {SYM_SETTINGS};
struct kowhai_node_t* tree_descriptors[] = {settings_descriptor};
void* tree_data_buffers[] = {&settings};
uint16_t function_list[] = {};
struct kowhai_protocol_function_details_t function_list_details[] = {};

char packet_buffer[MAX_PACKET_SIZE];
size_t tree_descriptor_sizes[COUNT_OF(tree_descriptors)];
struct kowhai_protocol_server_t server;
prot_send_buffer_t send_buffer_cb;

//
// functions
//

void node_pre_write(pkowhai_protocol_server_t server, void* param, uint16_t tree_id, struct kowhai_node_t* node, int offset)
{
}

void node_post_write(pkowhai_protocol_server_t server, void* param, uint16_t tree_id, struct kowhai_node_t* node, int offset, int bytes_written)
{
}

void server_buffer_send(pkowhai_protocol_server_t server, void* param, void* buffer, size_t buffer_size)
{
    if (send_buffer_cb != NULL)
        send_buffer_cb(buffer, buffer_size);
}

int function_called(pkowhai_protocol_server_t server, void* param, uint16_t function_id)
{
    return 1;
}

void prot_init(prot_send_buffer_t send_buffer)
{
    send_buffer_cb = send_buffer;
    kowhai_server_init_tree_descriptor_sizes(tree_descriptors, tree_descriptor_sizes, COUNT_OF(tree_descriptors));
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
        tree_descriptors,
        tree_descriptor_sizes,
        tree_data_buffers,
        COUNT_OF(function_list),
        function_list,
        function_list_details,
        function_called,
        NULL,
        COUNT_OF(symbols),
        symbols);
}

void prot_process_packet(void* buffer, size_t buffer_size)
{
    kowhai_server_process_packet(&server, buffer, buffer_size);
}

