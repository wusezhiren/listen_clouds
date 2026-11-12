#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "listen_clouds/protocol.h"

int main(void)
{
    char out[256];
    assert(lc_v1_hello(out, sizeof(out)) == LC_OK);
    assert(strstr(out, "\"version\":1") && strstr(out, "\"mcp\":true"));
    assert(strstr(out, "\"sample_rate\":16000") && strstr(out, "\"frame_duration\":60"));
    assert(lc_v1_hello(out, 8) == LC_ERR_SIZE);
    assert(lc_v1_listen_start("abc-123", true, out, sizeof(out)) == LC_OK);
    assert(strstr(out, "\"session_id\":\"abc-123\""));
    assert(strstr(out, "\"state\":\"start\""));
    assert(strstr(out, "\"mode\":\"auto\""));
    assert(lc_v1_listen_start("abc-123", false, out, sizeof(out)) == LC_OK);
    assert(strstr(out, "\"mode\":\"manual\""));
    assert(lc_v1_listen_stop("abc-123", out, sizeof(out)) == LC_OK);
    assert(strstr(out, "\"state\":\"stop\""));
    assert(lc_v1_listen_start("bad\"id", true, out, sizeof(out)) == LC_ERR_ARGUMENT);
    assert(lc_v1_listen_start("", true, out, sizeof(out)) == LC_ERR_ARGUMENT);
    assert(lc_v1_mcp_envelope("abc-123", "{\"jsonrpc\":\"2.0\"}", out, sizeof(out)) == LC_OK);
    assert(strstr(out, "\"payload\":{\"jsonrpc\":\"2.0\"}"));
    assert(lc_v1_mcp_envelope("abc-123", "not-json", out, sizeof(out)) == LC_ERR_ARGUMENT);
    assert(lc_v1_mcp_envelope("abc-123", "{} trailing", out, sizeof(out)) == LC_ERR_ARGUMENT);
    assert(lc_v1_mcp_envelope("abc-123", "{}", out, 10) == LC_ERR_SIZE);
    lc_v1_event_t event;
    const char hello[] = "{\"type\":\"hello\",\"transport\":\"websocket\",\"session_id\":\"sid-7\"}";
    assert(lc_v1_parse_event(hello, strlen(hello), &event) == LC_OK);
    assert(event.type == LC_V1_HELLO && !strcmp(event.session_id, "sid-7"));
    const char stt[] = "{\"type\":\"stt\",\"text\":\"你好，小智，打开客厅灯。\"}";
    assert(lc_v1_parse_event(stt, strlen(stt), &event) == LC_OK);
    assert(event.type == LC_V1_STT && strstr(event.text, "客厅灯"));
    const char tts[] = "{\"type\":\"tts\",\"state\":\"stop\"}";
    assert(lc_v1_parse_event(tts, strlen(tts), &event) == LC_OK);
    assert(event.type == LC_V1_TTS && !strcmp(event.state, "stop"));
    const char mcp[] = "{\"type\":\"mcp\",\"payload\":{\"jsonrpc\":\"2.0\",\"id\":3,\"method\":\"tools/list\"}}";
    assert(lc_v1_parse_event(mcp, strlen(mcp), &event) == LC_OK);
    assert(event.type == LC_V1_MCP && strstr(event.payload, "tools/list"));
    assert(lc_v1_parse_event("{broken", 7, &event) == LC_ERR_ARGUMENT);
    assert(lc_v1_parse_event(hello, 5, &event) == LC_ERR_ARGUMENT);
    assert(lc_v1_parse_event(hello, strlen(hello), NULL) == LC_ERR_ARGUMENT);
    assert(lc_v1_parse_event("{} trailing", 11, &event) == LC_ERR_ARGUMENT);
    assert(lc_v1_parse_event(hello, 8193, &event) == LC_ERR_ARGUMENT);
    const char bad_mcp[] = "{\"type\":\"mcp\",\"payload\":[]}";
    assert(lc_v1_parse_event(bad_mcp, strlen(bad_mcp), &event) == LC_ERR_ARGUMENT);
    const char bad_hello[] = "{\"type\":\"hello\",\"transport\":\"udp\"}";
    assert(lc_v1_parse_event(bad_hello, strlen(bad_hello), &event) == LC_ERR_ARGUMENT);
    lc_activation_t activation;
    const char activation_json[] = "{\"activation\":{\"code\":\"123456\",\"challenge\":\"abc\"},\"websocket\":{\"url\":\"wss://example.test\",\"token\":\"secret\"}}";
    assert(lc_v1_parse_activation(activation_json, strlen(activation_json), &activation) == LC_OK);
    assert(!strcmp(activation.code, "123456") && !strcmp(activation.token, "secret"));
    assert(lc_v1_parse_activation("{}", 2, &activation) == LC_ERR_ARGUMENT);
    assert(lc_v1_parse_activation("{} trailing", 11, &activation) == LC_ERR_ARGUMENT);
    assert(lc_v1_parse_activation(activation_json, 16384, &activation) == LC_ERR_ARGUMENT);
    const char incomplete_activation[] = "{\"activation\":{\"code\":\"1\",\"challenge\":\"x\"},\"websocket\":{\"url\":\"wss://x\"}}";
    assert(lc_v1_parse_activation(incomplete_activation, strlen(incomplete_activation), &activation) == LC_ERR_ARGUMENT);
    const char bad_url[] = "{\"activation\":{\"code\":\"1\",\"challenge\":\"x\"},\"websocket\":{\"url\":\"http://x\",\"token\":\"secret\"}}";
    assert(lc_v1_parse_activation(bad_url, strlen(bad_url), &activation) == LC_ERR_ARGUMENT);
    const char no_credentials[] = "{\"activation\":{\"code\":\"1\",\"challenge\":\"x\"}}";
    assert(lc_v1_parse_activation(no_credentials, strlen(no_credentials), &activation) == LC_OK);
    puts("voice v1 protocol tests passed");
}
