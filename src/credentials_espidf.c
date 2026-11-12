#include "listen_clouds/credentials.h"

#include <stdio.h>
#include <string.h>
#include "esp_random.h"
#include "nvs.h"

#define LC_NVS_NAMESPACE "gw_voice"

esp_err_t lc_credentials_load(lc_credentials_t *out)
{
    if (!out) return ESP_ERR_INVALID_ARG;
    *out = (lc_credentials_t){0};
    nvs_handle_t h;
    if (nvs_open(LC_NVS_NAMESPACE, NVS_READONLY, &h) == ESP_OK) {
        size_t n = sizeof(out->url);
        if (nvs_get_str(h, "url", out->url, &n) != ESP_OK) out->url[0] = 0;
        n = sizeof(out->token);
        if (nvs_get_str(h, "token", out->token, &n) != ESP_OK) out->token[0] = 0;
        n = sizeof(out->client_id);
        if (nvs_get_str(h, "client_id", out->client_id, &n) != ESP_OK) out->client_id[0] = 0;
        nvs_close(h);
    }
    if ((out->url[0] && strncmp(out->url, "wss://", 6)) || !out->token[0])
        out->url[0] = out->token[0] = 0;
    if (out->client_id[0]) return ESP_OK;
    esp_err_t err = nvs_open(LC_NVS_NAMESPACE, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    uint8_t uuid[16];
    esp_fill_random(uuid, sizeof(uuid));
    uuid[6] = (uuid[6] & 0x0f) | 0x40;
    uuid[8] = (uuid[8] & 0x3f) | 0x80;
    snprintf(out->client_id, sizeof(out->client_id),
        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
        uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
    err = nvs_set_str(h, "client_id", out->client_id);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

esp_err_t lc_credentials_save(lc_credentials_t *credentials, const char *url, const char *token)
{
    if (!credentials || !url || !token || (url[0] && strncmp(url, "wss://", 6)) ||
        strlen(url) >= sizeof(credentials->url) || strlen(token) >= sizeof(credentials->token))
        return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = nvs_open(LC_NVS_NAMESPACE, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_set_str(h, "url", url);
    if (err == ESP_OK) err = nvs_set_str(h, "token", token);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    if (err == ESP_OK) {
        strcpy(credentials->url, url);
        strcpy(credentials->token, token);
    }
    return err;
}
