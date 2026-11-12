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
