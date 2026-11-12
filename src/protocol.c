#include "listen_clouds/protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

static bool safe_id(const char *id)
{
    if (!id || !*id || strlen(id) > 95) return false;
    for (const unsigned char *p = (const unsigned char *)id; *p; ++p)
        if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9') || *p == '-' || *p == '_')) return false;
    return true;
}

static lc_status_t format_result(char *out, size_t capacity, int count)
{
    if (count < 0 || (size_t)count >= capacity) {
        if (out && capacity) out[0] = '\0';
        return LC_ERR_SIZE;
    }
    return LC_OK;
}

lc_status_t lc_v1_hello(char *out, size_t capacity)
{
    if (!out || !capacity) return LC_ERR_ARGUMENT;
    return format_result(out, capacity, snprintf(out, capacity,
        "{\"type\":\"hello\",\"version\":1,\"features\":{\"mcp\":true},"
        "\"transport\":\"websocket\",\"audio_params\":{\"format\":\"opus\","
        "\"sample_rate\":16000,\"channels\":1,\"frame_duration\":60}}"));
}

lc_status_t lc_v1_listen_start(const char *session_id, bool automatic,
                                    char *out, size_t capacity)
{
    if (!safe_id(session_id) || !out || !capacity) return LC_ERR_ARGUMENT;
    return format_result(out, capacity, snprintf(out, capacity,
        "{\"session_id\":\"%s\",\"type\":\"listen\",\"state\":\"start\",\"mode\":\"%s\"}",
        session_id, automatic ? "auto" : "manual"));
}

lc_status_t lc_v1_listen_stop(const char *session_id, char *out, size_t capacity)
{
    if (!safe_id(session_id) || !out || !capacity) return LC_ERR_ARGUMENT;
    return format_result(out, capacity, snprintf(out, capacity,
        "{\"session_id\":\"%s\",\"type\":\"listen\",\"state\":\"stop\"}", session_id));
}

lc_status_t lc_v1_mcp_envelope(const char *session_id, const char *payload_json,
                                     char *out, size_t capacity)
{
    if (!safe_id(session_id) || !payload_json || !out || !capacity) return LC_ERR_ARGUMENT;
    cJSON *payload = cJSON_ParseWithOpts(payload_json, NULL, 1);
    if (!cJSON_IsObject(payload)) { cJSON_Delete(payload); return LC_ERR_ARGUMENT; }
    cJSON_Delete(payload);
    return format_result(out, capacity, snprintf(out, capacity,
        "{\"type\":\"mcp\",\"session_id\":\"%s\",\"payload\":%s}", session_id, payload_json));
}

static bool copy_field(char *dst, size_t capacity, const char *value)
{
    if (!value) return true;
    size_t length = strlen(value);
    if (length >= capacity) return false;
    memcpy(dst, value, length + 1);
    return true;
}

lc_status_t lc_v1_parse_event(const char *data, size_t length, lc_v1_event_t *out)
{
    if (!data || !length || length > 8192 || !out) return LC_ERR_ARGUMENT;
    *out = (lc_v1_event_t){0};
    char *input = malloc(length + 1);
    if (!input) return LC_ERR_SIZE;
    memcpy(input, data, length);
    input[length] = '\0';
    cJSON *root = cJSON_ParseWithOpts(input, NULL, 1);
    if (!cJSON_IsObject(root)) { cJSON_Delete(root); free(input); return LC_ERR_ARGUMENT; }
    const char *type = cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(root, "type"));
    if (!type) { cJSON_Delete(root); free(input); return LC_ERR_ARGUMENT; }
    lc_status_t result = LC_OK;
    if (!strcmp(type, "hello")) {
        const char *transport = cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(root, "transport"));
        const char *id = cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(root, "session_id"));
        if (!transport || strcmp(transport, "websocket") || !id || !safe_id(id)) result = LC_ERR_ARGUMENT;
        else { out->type = LC_V1_HELLO; copy_field(out->session_id, sizeof(out->session_id), id); }
    } else if (!strcmp(type, "stt")) {
        out->type = LC_V1_STT;
        if (!copy_field(out->text, sizeof(out->text),
                cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(root, "text")))) result = LC_ERR_SIZE;
    } else if (!strcmp(type, "tts")) {
        out->type = LC_V1_TTS;
        if (!copy_field(out->state, sizeof(out->state),
                cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(root, "state")))) result = LC_ERR_SIZE;
    } else if (!strcmp(type, "mcp")) {
        out->type = LC_V1_MCP;
        cJSON *payload = cJSON_GetObjectItemCaseSensitive(root, "payload");
        if (!cJSON_IsObject(payload)) result = LC_ERR_ARGUMENT;
        else {
            char *json = cJSON_PrintUnformatted(payload);
            if (!json || !copy_field(out->payload, sizeof(out->payload), json)) result = LC_ERR_SIZE;
            cJSON_free(json);
            if (!copy_field(out->method, sizeof(out->method),
                    cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(payload, "method")))) result = LC_ERR_SIZE;
        }
    } else out->type = LC_V1_OTHER;
    cJSON_Delete(root);
    free(input);
    return result;
}
