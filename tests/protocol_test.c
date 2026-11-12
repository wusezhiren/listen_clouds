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
    puts("voice v1 protocol tests passed");
}
