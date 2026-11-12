#include "listen_clouds/activation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_crt_bundle.h"
#include "esp_http_client.h"

typedef struct { char *body; size_t used; } activation_response_t;

static esp_err_t activation_http_event(esp_http_client_event_t *event)
{
    activation_response_t *response = event->user_data;
    if (event->event_id == HTTP_EVENT_ON_DATA && response && event->data_len > 0) {
        if ((size_t)event->data_len > 16383 - response->used) return ESP_ERR_INVALID_SIZE;
        memcpy(response->body + response->used, event->data, event->data_len);
        response->used += event->data_len;
        response->body[response->used] = 0;
    }
    return ESP_OK;
}

esp_err_t lc_activation_request(const char *endpoint, const char *device_id,
    const char *client_id, const char *user_agent, bool confirm,
    lc_activation_t *out, int *http_status)
{
    if (!endpoint || !device_id || !client_id || !user_agent || !out || !http_status)
        return ESP_ERR_INVALID_ARG;
    *out = (lc_activation_t){0};
    *http_status = 0;
    char url[320];
    int n = snprintf(url, sizeof(url), "%s%s", endpoint, confirm ? "activate" : "");
    if (n < 0 || n >= sizeof(url)) return ESP_ERR_INVALID_SIZE;
    activation_response_t response = {.body = calloc(1, 16384)};
    if (!response.body) return ESP_ERR_NO_MEM;
    esp_http_client_config_t cfg = {.url = url, .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 15000, .buffer_size = 2048,
        .event_handler = activation_http_event, .user_data = &response};
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (!client) { free(response.body); return ESP_ERR_NO_MEM; }
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Activation-Version", "1");
    esp_http_client_set_header(client, "Device-Id", device_id);
    esp_http_client_set_header(client, "Client-Id", client_id);
    esp_http_client_set_header(client, "User-Agent", user_agent);
    esp_http_client_set_header(client, "Accept-Language", "zh-CN");
    esp_http_client_set_post_field(client, "{}", 2);
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) *http_status = esp_http_client_get_status_code(client);
    if (err == ESP_OK && *http_status == 200 && !confirm &&
        lc_v1_parse_activation(response.body, response.used, out) != LC_OK)
        err = ESP_ERR_INVALID_RESPONSE;
    if (err == ESP_OK && *http_status != 200) err = *http_status == 202 ? ESP_ERR_TIMEOUT : ESP_FAIL;
    esp_http_client_cleanup(client);
    free(response.body);
    return err;
}
