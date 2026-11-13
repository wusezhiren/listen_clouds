#include "listen_clouds/transport.h"
#include "listen_clouds/frame.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_crt_bundle.h"
#include "esp_websocket_client.h"
#include "freertos/FreeRTOS.h"

struct lc_ws_client {
    esp_websocket_client_handle_t handle;
    lc_ws_callback_t callback;
    void *context;
    char headers[512];
};

static void on_ws_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)base;
    lc_ws_client_t *client = arg;
    if (!client || !client->callback) return;
    lc_ws_event_t event = {0};
    switch (id) {
    case WEBSOCKET_EVENT_CONNECTED: event.type = LC_WS_CONNECTED; break;
    case WEBSOCKET_EVENT_DISCONNECTED: event.type = LC_WS_DISCONNECTED; break;
    case WEBSOCKET_EVENT_ERROR: event.type = LC_WS_ERROR; break;
    case WEBSOCKET_EVENT_DATA: {
        const esp_websocket_event_data_t *frame = data;
        if (!frame || !lc_ws_frame_complete(frame->data_len, frame->payload_len,
                                             frame->payload_offset, 8192)) return;
        if (frame->op_code == WS_TRANSPORT_OPCODES_BINARY) {
            event.type = LC_WS_BINARY;
        } else if (frame->op_code == WS_TRANSPORT_OPCODES_TEXT) {
            event.type = LC_WS_TEXT;
        } else return;
        event.data = (const uint8_t *)frame->data_ptr;
        event.length = frame->data_len;
        break;
    }
    default: return;
    }
    client->callback(client->context, &event);
}

esp_err_t lc_ws_start(lc_ws_client_t **client, const lc_ws_config_t *config)
{
    if (!client || *client || !config || !config->url || !config->token ||
        !config->device_id || !config->client_id || !config->callback) return ESP_ERR_INVALID_ARG;
    lc_ws_client_t *next = calloc(1, sizeof(*next));
    if (!next) return ESP_ERR_NO_MEM;
    int length = snprintf(next->headers, sizeof(next->headers),
        "Authorization: Bearer %s\r\nProtocol-Version: 1\r\nDevice-Id: %s\r\nClient-Id: %s\r\n",
        config->token, config->device_id, config->client_id);
    if (length < 0 || length >= sizeof(next->headers)) { free(next); return ESP_ERR_INVALID_SIZE; }
    next->callback = config->callback;
    next->context = config->context;
    esp_websocket_client_config_t ws_config = {
        .uri = config->url, .headers = next->headers,
         /* Keep the WebSocket/TLS peak below the free internal-RAM margin
          * while I2S duplex and Opus are active. Frames are already bounded
          * by the protocol layer, so a smaller transport buffer is enough. */
         .crt_bundle_attach = esp_crt_bundle_attach, .buffer_size = 1024,
         .task_stack = 4096, .reconnect_timeout_ms = 10000,
    };
    next->handle = esp_websocket_client_init(&ws_config);
    if (!next->handle) { free(next); return ESP_ERR_NO_MEM; }
    esp_websocket_register_events(next->handle, WEBSOCKET_EVENT_ANY, on_ws_event, next);
    esp_err_t err = esp_websocket_client_start(next->handle);
    if (err != ESP_OK) { esp_websocket_client_destroy(next->handle); free(next); return err; }
    *client = next;
    return ESP_OK;
}

void lc_ws_stop(lc_ws_client_t **client)
{
    if (!client || !*client) return;
    lc_ws_client_t *old = *client;
    *client = NULL;
    esp_websocket_client_stop(old->handle);
    esp_websocket_client_destroy(old->handle);
    free(old);
}

bool lc_ws_connected(const lc_ws_client_t *client)
{
    return client && esp_websocket_client_is_connected(client->handle);
}

int lc_ws_send_text(lc_ws_client_t *client, const char *text, int timeout_ms)
{
    return client && text ? esp_websocket_client_send_text(client->handle, text, strlen(text),
        pdMS_TO_TICKS(timeout_ms)) : -1;
}

int lc_ws_send_binary(lc_ws_client_t *client, const uint8_t *data, size_t length, int timeout_ms)
{
    return client && data && length ? esp_websocket_client_send_bin(client->handle,
        (const char *)data, length, pdMS_TO_TICKS(timeout_ms)) : -1;
}
