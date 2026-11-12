#include "listen_clouds/protocol.h"

#include <stdio.h>
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
