#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
#include "listen_clouds/protocol.h"

typedef struct lc_ws_client lc_ws_client_t;
typedef enum { LC_WS_CONNECTED, LC_WS_DISCONNECTED, LC_WS_ERROR, LC_WS_TEXT, LC_WS_BINARY } lc_ws_event_type_t;
typedef struct {
    lc_ws_event_type_t type;
    const uint8_t *data;
    size_t length;
} lc_ws_event_t;
/* data is borrowed from WebSocket and valid only during the callback. */
typedef void (*lc_ws_callback_t)(void *ctx, const lc_ws_event_t *event);

typedef struct {
    const char *url;
    const char *token;
    const char *device_id;
    const char *client_id;
    lc_ws_callback_t callback;
    void *context;
} lc_ws_config_t;

esp_err_t lc_ws_start(lc_ws_client_t **client, const lc_ws_config_t *config);
void lc_ws_stop(lc_ws_client_t **client);
bool lc_ws_connected(const lc_ws_client_t *client);
int lc_ws_send_text(lc_ws_client_t *client, const char *text, int timeout_ms);
int lc_ws_send_binary(lc_ws_client_t *client, const uint8_t *data, size_t length, int timeout_ms);
