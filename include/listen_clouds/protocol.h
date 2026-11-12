#pragma once

#include <stdbool.h>
#include <stddef.h>
typedef enum {
    LC_OK = 0,
    LC_ERR_ARGUMENT,
    LC_ERR_SIZE,
} lc_status_t;

lc_status_t lc_v1_hello(char *out, size_t capacity);
lc_status_t lc_v1_listen_start(const char *session_id, bool automatic, char *out, size_t capacity);
lc_status_t lc_v1_listen_stop(const char *session_id, char *out, size_t capacity);
lc_status_t lc_v1_mcp_envelope(const char *session_id, const char *payload_json,
                               char *out, size_t capacity);

typedef enum { LC_V1_HELLO, LC_V1_STT, LC_V1_TTS, LC_V1_MCP, LC_V1_OTHER } lc_v1_type_t;
typedef struct {
    lc_v1_type_t type;
    char session_id[96];
    char text[512];
    char state[32];
    char method[64];
    char payload[4096];
} lc_v1_event_t;

/* Only complete text frames; caller must reassemble fragmented WebSocket frames. */
lc_status_t lc_v1_parse_event(const char *data, size_t length, lc_v1_event_t *out);
/* Activation response, optional websocket credentials are copied, never logged. */
typedef struct {
    char code[64];
    char challenge[192];
    char url[256];
    char token[192];
} lc_activation_t;
lc_status_t lc_v1_parse_activation(const char *data, size_t length, lc_activation_t *out);
